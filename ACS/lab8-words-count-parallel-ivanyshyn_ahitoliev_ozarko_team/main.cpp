#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "utility/counter.hpp"
#include "cfg_reader.hpp"
#include "time_measurements.hpp"
#include "thread_queue.hpp"
#include <sstream>
#include <thread>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <archive.h>
#include <archive_entry.h>

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

void file_filler(std::string &input_dir, ThreadQueue<std::filesystem::__cxx11::directory_entry> &txt_queue, ThreadQueue<std::filesystem::__cxx11::directory_entry> &zip_queue, 
    size_t &max_file_size, std::vector<std::string> &indexing_extensions, std::vector<std::string> &archives_extensions, 
    std::chrono::_V2::system_clock::time_point &t1, std::chrono::_V2::system_clock::time_point &t2, 
    std::chrono::duration<int64_t, std::nano>  &t3) {
    t1 = get_current_time_fenced();
    for (const auto& entry : fs::recursive_directory_iterator(input_dir)) {
        if (fs::is_regular_file(entry)) {
            std::string extension = entry.path().extension().string();
            if (std::find(indexing_extensions.begin(), indexing_extensions.end(), extension) != indexing_extensions.end() && fs::file_size(entry.path()) <= max_file_size) {
                txt_queue.push(entry);
            } else if (std::find(archives_extensions.begin(), archives_extensions.end(), extension) != archives_extensions.end() && fs::file_size(entry.path()) <= max_file_size){
                zip_queue.push(entry);
            }
        }
    }
    std::filesystem::directory_entry empty;
    zip_queue.push(empty);
    t2 = get_current_time_fenced();
    t3 = t2 - t1;
}

void unpacker(ThreadQueue<std::filesystem::__cxx11::directory_entry> &txt_queue, ThreadQueue<std::string> &binary_queue, 
    ThreadQueue<std::filesystem::__cxx11::directory_entry> &zip_queue, 
    std::chrono::_V2::system_clock::time_point &t1, std::chrono::_V2::system_clock::time_point &t2, 
    std::chrono::duration<int64_t, std::nano>  &t3) {
    t1 = get_current_time_fenced();
    while (true) {
        auto entry_z = zip_queue.pop();
        if (entry_z.path() == fs::path("")) {
            zip_queue.push(entry_z);
            break;
        }
        const char* zip_filename = entry_z.path().c_str();
        struct archive* a;
        struct archive_entry* entry;
        int r;
        a = archive_read_new();
        archive_read_support_format_all(a);
        r = archive_read_open_filename(a, zip_filename, 10240); // Adjust the buffer size as needed
        if (r != ARCHIVE_OK) {
            std::cerr << "Failed to open zip file: " << archive_error_string(a) << std::endl;
        }
        while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
            std::vector<char> buffer(4096);
            std::string content;
            ssize_t size;
            while ((size = archive_read_data(a, buffer.data(), buffer.size())) > 0) {
                content.append(buffer.data(), size);
            }
            binary_queue.push(content);
        }
        archive_read_close(a);
        archive_read_free(a);
    }
    std::filesystem::directory_entry empty;
    txt_queue.push(empty);
    t2 = get_current_time_fenced();
    t3 = t2 - t1;
}

void binary_filler(ThreadQueue<std::filesystem::__cxx11::directory_entry> &txt_queue, 
    ThreadQueue<std::string> &binary_queue, std::chrono::_V2::system_clock::time_point &t1, 
    std::chrono::_V2::system_clock::time_point &t2, std::chrono::duration<int64_t, std::nano>  &t3) {
    t1 = get_current_time_fenced();
    while (true) {
        auto entry = txt_queue.pop();
        if (entry.path() == fs::path("")) {
            txt_queue.push(entry);
            break;
        }
        std::ifstream file(entry.path());
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        binary_queue.push(content);
    }
    binary_queue.push("End");
    t2 = get_current_time_fenced();
    t3 = t2 - t1;
}

void dictionary_filler(ThreadQueue<std::string> &binary_queue, 
    ThreadQueue<std::unordered_map<std::string, size_t>> &dict_queue) {
    while (true) {
        auto content = binary_queue.pop();
        if (content == "End") {
            binary_queue.push(content);
            break;
        }
        std::unordered_map<std::string, size_t> word_counts;
        auto tmp_counts = count_words(content);
        for (const auto& [word, count] : tmp_counts) {
            word_counts[word] += count;
        }
        dict_queue.push(word_counts);
    }
}

void merger(ThreadQueue<std::unordered_map<std::string, size_t>> &dict_queue) {
    std::string poison = "A";
    while (true) {
        std::unordered_map<std::string, size_t> one = dict_queue.pop();
        bool empty = dict_queue.empty();
        if (one.size() == 1 && one.find(poison) != one.end()) {
            dict_queue.push(one);
            if (empty) {
                break;
            } else {
                continue;
            }
        } 
        std::unordered_map<std::string, size_t> two = dict_queue.pop();
        empty = dict_queue.empty();
        if (two.size() == 1 && two.find(poison) != two.end()) {
            dict_queue.push(one);
            dict_queue.push(two);
            if (empty) {
                break;
            } else {
                continue;
            }
        }
        for (auto pair : two) {
            one[pair.first] += pair.second;
        }
        dict_queue.push(one);
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
    std::chrono::_V2::system_clock::time_point file_time_1;
    std::chrono::_V2::system_clock::time_point file_time_2;
    std::chrono::duration<int64_t, std::nano>  file_time_res;
    ThreadQueue<std::filesystem::__cxx11::directory_entry> txt_queue(filenames_queue_size);
    ThreadQueue<std::filesystem::__cxx11::directory_entry> zip_queue(filenames_queue_size);
    std::thread iterator_thread([&]() {
        file_filler(input_dir, txt_queue, zip_queue, max_file_size, indexing_extensions, archives_extensions,
        file_time_1, file_time_2, file_time_res);
    });
    std::chrono::_V2::system_clock::time_point zip_time_1;
    std::chrono::_V2::system_clock::time_point zip_time_2;
    std::chrono::duration<int64_t, std::nano>  zip_time_res;
    ThreadQueue<std::string> binary_queue(raw_files_queue_size);
    std::thread unpack_thread([&]() {
        unpacker(txt_queue, binary_queue, zip_queue, zip_time_1, zip_time_2, zip_time_res);
    });
    std::chrono::_V2::system_clock::time_point read_time_1;
    std::chrono::_V2::system_clock::time_point read_time_2;
    std::chrono::duration<int64_t, std::nano>  read_time_res;
    std::thread binary_thread([&]() {
        binary_filler(txt_queue, binary_queue, read_time_1, read_time_2, read_time_res);
    });
    std::vector<std::thread> worker_vectors;
    std::vector<std::thread> merge_vectors;
    ThreadQueue<std::unordered_map<std::string, size_t>> dictionary_queue(dictionaries_queue_size);
    for (size_t thread = 0; thread < thread_amount; thread++) {
        std::thread worker([&]() {
            dictionary_filler(binary_queue, dictionary_queue);
        });
        worker_vectors.push_back(std::move(worker));
    }
    for (size_t thread = 0; thread < merge_amount; thread++) {
        std::thread merg([&]() {
            merger(dictionary_queue);
        });
        merge_vectors.push_back(std::move(merg));
    }
    auto t1 = get_current_time_fenced();
    iterator_thread.join();
    unpack_thread.join();
    binary_thread.join();
    for (auto & thread: worker_vectors) {
        thread.join();
    }
    std::string poison = "A";
    std::unordered_map<std::string, size_t> poison_pill{{poison, 0}};
    dictionary_queue.push(poison_pill);
    for (auto & thread: merge_vectors) {
        thread.join();
    }
    auto t2 = get_current_time_fenced();
    auto time_read = t2 - t1;
    std::unordered_map<std::string, size_t> word_counts = dictionary_queue.pop();
    std::vector<std::pair<std::string, size_t>> sorted_by_alphabet(word_counts.begin(), word_counts.end());
    std::vector<std::pair<std::string, size_t>> sorted_by_count(word_counts.begin(), word_counts.end());
    std::sort(sorted_by_alphabet.begin(), sorted_by_alphabet.end());
    std::sort(sorted_by_count.begin(), sorted_by_count.end(), [](const auto& a, const auto& b) {
        if (a.second == b.second) {
            return a.first < b.first;
        }
        return a.second > b.second;
    });
    t1 = get_current_time_fenced();
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
    std::cout << "Total=" << to_ms(time_read) << std::endl;
    std::cout << "Finding=" << to_ms(file_time_res) + to_ms(zip_time_res)<< std::endl;
    std::cout << "Reading=" << to_ms(read_time_res) << std::endl;
    std::cout << "Writing=" << to_ms(time_write) << std::endl;
    return 0;
}