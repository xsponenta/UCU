#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "utility/counter.hpp"
#include "cfg_reader.hpp"
#include "time_measurements.hpp"
#include <sstream>
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

std::unordered_map<std::string, size_t> txt_reader(std::string input_dir, 
    size_t max_file_size, std::vector<std::string> indexing_extensions, std::vector<std::string> archives_extensions) {
    std::unordered_map<std::string, size_t> word_counts;
    for (const auto& entry : fs::recursive_directory_iterator(input_dir)) {
        if (fs::is_regular_file(entry)) {
            std::string extension = entry.path().extension().string();
            if (std::find(indexing_extensions.begin(), indexing_extensions.end(), extension) != indexing_extensions.end() && fs::file_size(entry.path()) <= max_file_size) {
                try {
                    std::ifstream file(entry.path());
                    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                    file.close();
                    auto tmp_counts = count_words(content);
                    for (const auto& [word, count] : tmp_counts) {
                        word_counts[word] += count;
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error processing file '" << entry.path() << "': " << e.what() << std::endl;
                }
            } else if (std::find(archives_extensions.begin(), archives_extensions.end(), extension) != archives_extensions.end() && fs::file_size(entry.path()) <= max_file_size) {
                const char* zip_filename = entry.path().c_str();
                struct archive* a;
                struct archive_entry* entry;
                int r;
                a = archive_read_new();
                archive_read_support_format_zip(a);
                r = archive_read_open_filename(a, zip_filename, 10240);
                //if (r != ARCHIVE_OK) {
                //    std::cerr << "Failed to open zip file: " << archive_error_string(a) << std::endl;
                //}
                while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
                    try {
                        std::ifstream file(archive_entry_pathname(entry));
                        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
                        file.close();
                        auto tmp_counts = count_words(content);
                        for (const auto& [word, count] : tmp_counts) {
                            word_counts[word] += count;
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Error processing file '" << archive_entry_pathname(entry) << "': " << e.what() << std::endl;
                    }
                }
                archive_read_close(a);
                archive_read_free(a);
            }
        }
    }
    return word_counts;
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

    auto t1 = get_current_time_fenced();
    auto word_counts = txt_reader(input_dir, max_file_size, indexing_extensions, archives_extensions);
    auto t2 = get_current_time_fenced();
    auto time_read = t2 - t1;
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
    std::cout << "Writing=" << to_ms(time_write) << std::endl;
    return 0;
}