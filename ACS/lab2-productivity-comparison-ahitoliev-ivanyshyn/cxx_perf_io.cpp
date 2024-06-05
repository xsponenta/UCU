#include <iostream>
#include <fstream>
#include "program1.hpp"
#include <chrono>
#include <atomic>

inline std::chrono::high_resolution_clock::time_point
get_current_time_fenced() {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

uint64_t count_letters(const std::vector<std::string>& data) {
    uint64_t count = 0;
    for (const auto& word : data) {
        count += word.length();
    }
    return count;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Wrong number of arguments" << std::endl;
        return static_cast<int>(Error::INVALID_ARGS);
    }

    std::string file = argv[1];
    int method_number = std::stoi(argv[2]);

    std::vector<std::string> data;

    try {
        std::ifstream input_file(file);
        if (!input_file.is_open()) {
            throw Error(Error::CANNOT_OPEN_INPUT, "Unable to open file with input data");
        }

        auto start = get_current_time_fenced();

        switch (method_number) {
            case 1:
                data = readFileStreamIdiomatic(file);
                break;
            case 2:
                data = readFileInMemory(file);
                break;
            case 3:
                data = readFileAsString(file);
                break;
            case 4:
                data = readFileLargeFile(file);
                break;
            case 5:
                data = readFileStreamIterators(file);
                break;
            default:
                throw Error(Error::INVALID_METHOD, "Wrong method number");
        }

        auto finish = get_current_time_fenced();
        auto total_time = finish - start;
        auto total_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_time).count();
        std::cout << total_time_ms << std::endl;

        uint64_t letter_count = count_letters(data);
        std::cout << letter_count << std::endl;

    } catch (const Error& e) {
        std::cerr << e.message() << std::endl;
        return static_cast<int>(e.code());
    }

    return static_cast<int>(Error::SUCCESS);
}
