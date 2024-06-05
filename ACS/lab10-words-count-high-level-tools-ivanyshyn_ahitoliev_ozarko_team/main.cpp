#include <iostream>
#include <fstream>
#include <filesystem>
#include "utility/counter.hpp"
#include "cfg_reader.hpp"
#include "time_measurements.hpp"
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <archive.h>
#include <archive_entry.h>

#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/parallel_for_each.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_pipeline.h>

namespace fs = std::filesystem;

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

std::string trimQuotes(const std::string& str) {
    if (str.size() >= 2 && str.front() == '"' && str.back() == '"') {
        return str.substr(1, str.size() - 2);
    }
    return str;
}

using FileQueue = tbb::concurrent_queue<std::filesystem::directory_entry>;
using BinaryQueue = tbb::concurrent_queue<std::string>;
using DictionaryQueue = tbb::concurrent_queue<tbb::concurrent_unordered_map<std::string, size_t>>;

std::pair<fs::directory_entry, bool> find_files(tbb::flow_control& fc, std::filesystem::recursive_directory_iterator &iter,
        std::filesystem::recursive_directory_iterator &iter_end, std::vector<std::string> &indexing_extensions, 
        std::vector<std::string> &archives_extensions, size_t &max_file_size) {
    if (iter == iter_end) {
        fc.stop();
        return {};
    }
    auto entry = *(iter++);
    if (fs::is_regular_file(entry)) {
        std::string extension = entry.path().extension().string();
        if (std::find(indexing_extensions.begin(), indexing_extensions.end(), extension) != indexing_extensions.end() &&
            fs::file_size(entry.path()) <= max_file_size) {
            return {entry, true};
        } else if (std::find(archives_extensions.begin(), archives_extensions.end(), extension) != archives_extensions.end() &&
                    fs::file_size(entry.path()) <= max_file_size) {
            return {entry, false};
        }
    }
    return {};
}

std::string read_files(std::pair<fs::directory_entry, bool> &entry_pair) {
    auto [entry, is_txt] = entry_pair;
    if (entry.path() == fs::path("")) {
        return "";
    }
    if (is_txt) {
        std::ifstream file(entry.path());
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        return content;
    } else {
        const char* zip_filename = entry.path().c_str();
        struct archive* a;
        struct archive_entry* entry;
        int r;
        a = archive_read_new();
        archive_read_support_format_zip(a);
        r = archive_read_open_filename(a, zip_filename, 10240);
        // if (r != ARCHIVE_OK) {
        //     std::cerr << "Failed to open zip file: " << archive_error_string(a) << std::endl;
        //     return "";
        // }
        std::string contents;
        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            std::filesystem::path filePath(archive_entry_pathname(entry));
            std::filesystem::__cxx11::directory_entry directoryEntry(filePath);
            std::ifstream file(directoryEntry.path());
            std::vector<char> buffer(4096);
            ssize_t size;
            while ((size = archive_read_data(a, buffer.data(), buffer.size())) > 0) {
                contents.append(buffer.data(), size);
            }
            file.close();
        }
        archive_read_close(a);
        archive_read_free(a);
        return contents;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    std::string config_file = argv[1];

    if (!fs::exists(config_file)) {
        std::cerr << "Error: Configuration file '" << config_file << "' does not exist." << std::endl;
        return 2;
    }

    std::string input_dir, out_by_a, out_by_n;
    std::vector<std::string> indexing_extensions, archives_extensions;
    size_t max_file_size;
    size_t thread_amount;
    size_t merge_amount;
    size_t filenames_queue_size;
    size_t raw_files_queue_size;
    size_t dictionaries_queue_size;
    integral_config config_map{};
    try {
        read_cfg(config_map, config_file);
    } catch (const CustomError& e) {
        std::cerr << e.what() << std::endl;
        return 3;
    }
    input_dir = trimQuotes(config_map.indir);
    out_by_a = trimQuotes(config_map.out_by_a);
    out_by_n = trimQuotes(config_map.out_by_n);
    thread_amount = config_map.indexing_threads;
    merge_amount = config_map.merging_threads;
    filenames_queue_size = config_map.filenames_queue_size;
    raw_files_queue_size = config_map.raw_files_queue_size;
    dictionaries_queue_size = config_map.dictionaries_queue_size;
    std::string indexing_extensions_str = config_map.indexing_extensions;
    indexing_extensions = split(indexing_extensions_str, ',');
    std::string indexing_archives_str = config_map.archives_extensions;
    archives_extensions = split(indexing_archives_str, ',');
    max_file_size = config_map.max_file_size;
    tbb::concurrent_unordered_map<std::string, size_t> word_counts;
    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        std::cerr << "Error: Input directory '" << input_dir << "' does not exist or is not a directory." << std::endl;
        return 4;
    }
    size_t lastSlashIndexA = out_by_a.find_last_of('/');
    size_t lastSlashIndexN = out_by_n.find_last_of('/');
    if (lastSlashIndexA != std::string::npos || lastSlashIndexN != std::string::npos) {
        std::string directoryPathA = out_by_a.substr(0, lastSlashIndexA);
        std::string directoryPathN = out_by_n.substr(0, lastSlashIndexN);
        if (!fs::is_directory(directoryPathA) || !fs::is_directory(directoryPathA)) {
            std::cerr << "Error: Output directories do not exist." << std::endl;
            return 5;
        }
    }

    FileQueue txt_queue;
    FileQueue zip_queue;

    BinaryQueue binary_queue((tbb::cache_aligned_allocator<std::string>()));
    DictionaryQueue dictionary_queue((tbb::cache_aligned_allocator<tbb::concurrent_unordered_map<std::string, size_t>>()));
    std::mutex word_counts_mutex;

    auto iter = fs::recursive_directory_iterator(input_dir);
    auto iter_end = std::filesystem::end(iter);
    long long time_read = 0;
    long long time_find = 0;
    auto t1 = get_current_time_fenced();
    tbb::parallel_pipeline(
            thread_amount,
            tbb::make_filter<void, std::pair<fs::directory_entry, bool>>(
                    tbb::filter_mode::serial_out_of_order, [&](tbb::flow_control& fc) -> std::pair<fs::directory_entry, bool> {
                        auto t1 = get_current_time_fenced();
                        auto result = find_files(fc, iter, iter_end, indexing_extensions, archives_extensions, max_file_size);
                        auto t2 = get_current_time_fenced();
                        time_find += to_ms(t2-t1);
                        return result;
                    }
            ) & tbb::make_filter<std::pair<fs::directory_entry, bool>, std::string>(
                    tbb::filter_mode::serial_out_of_order, [&](std::pair<fs::directory_entry, bool> entry_pair) -> std::string {
                        auto t1 = get_current_time_fenced();
                        auto result = read_files(entry_pair);
                        auto t2 = get_current_time_fenced();
                        time_read += to_ms(t2-t1);
                        return result;
                    }
            ) & tbb::make_filter<std::string, tbb::concurrent_unordered_map<std::string, size_t>>(
                    tbb::filter_mode::parallel, [&](std::string content) -> tbb::concurrent_unordered_map<std::string, size_t> {
                        tbb::concurrent_unordered_map<std::string, size_t> word_counts;
                        auto tmp_counts = count_words(content);
                        for (const auto& [word, count] : tmp_counts) {
                            word_counts[word] += count;
                        }
                        return word_counts;
                    }
            ) & tbb::make_filter<tbb::concurrent_unordered_map<std::string, size_t>, void>(
                    tbb::filter_mode::serial_in_order, [&](tbb::concurrent_unordered_map<std::string, size_t> local_word_counts) {
                        for (const auto& [word, count] : local_word_counts) {
                            std::lock_guard<std::mutex> lock(word_counts_mutex);
                            word_counts[word] += count;
                        }
                    }
            )
    );
    auto t2 = get_current_time_fenced();
    auto time_total = t2 - t1;

    t1 = get_current_time_fenced();
    std::vector<std::pair<std::string, size_t>> sorted_by_alphabet(word_counts.begin(), word_counts.end());
    std::vector<std::pair<std::string, size_t>> sorted_by_count(word_counts.begin(), word_counts.end());
    std::sort(sorted_by_alphabet.begin(), sorted_by_alphabet.end());
    std::sort(sorted_by_count.begin(), sorted_by_count.end(), [](const auto& a, const auto& b) {
        if (a.second == b.second) {
            return a.first < b.first;
        }
        return a.second > b.second;
    });
    try {
        std::ofstream file_a(out_by_a);
        for (const auto& [word, count] : sorted_by_alphabet) {
            file_a << word << ": " << count << std::endl;
        }
        file_a.close();

        std::ofstream file_n(out_by_n);
        for (const auto& [word, count] : sorted_by_count) {
            file_n << word << ": " << count << std::endl;
        }
        file_n.close();
    } catch (const std::exception& e) {
        std::cerr << "Error writing output files: " << e.what() << std::endl;
        return 6;
    }
    t2 = get_current_time_fenced();
    auto time_write = t2 - t1;
    std::cout << "Total=" << to_ms(time_total) << std::endl;
    std::cout << "Finding=" << time_find << std::endl;
    std::cout << "Reading=" << time_read << std::endl;
    std::cout << "Writing=" << to_ms(time_write) << std::endl;
    return 0;
}
