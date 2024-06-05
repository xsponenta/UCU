#include <iostream>
#include <fstream>
#include "cxx_perf_conv.hpp"
#include <chrono>
#include <atomic>

std::vector<std::string> readFileLargeFile(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> data;

    auto start_pos = file.tellg();
    file.ignore(std::numeric_limits<std::streamsize>::max());
    auto char_count = file.gcount();

    file.seekg(start_pos);

    std::string fileContents(char_count, '\0');

    file.read(&fileContents[0], fileContents.size());

    size_t start = 0;
    while (start < fileContents.length()) {
        size_t end = fileContents.find_first_of(" \t\n\r", start);
        if (end == std::string::npos) {
            data.push_back(fileContents.substr(start));
            break;
        } else {
            data.push_back(fileContents.substr(start, end - start));
            start = end + 1;
        }
    }

    return data;
}

inline std::chrono::high_resolution_clock::time_point
get_current_time_fenced() {
    std::atomic_thread_fence(std::memory_order_seq_cst);
    auto res_time = std::chrono::high_resolution_clock::now();
    std::atomic_thread_fence(std::memory_order_seq_cst);
    return res_time;
}

uint64_t sum_nums(const std::vector<unsigned>& numbers) {
    uint64_t sum = 0;
    for (unsigned num : numbers) {
        sum += num;
    }
    return sum % 64;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Wrong number of arguments" << std::endl;
        return static_cast<int>(Error::INVALID_ARGS);
    }

    std::string file = argv[1];
    int method_number = std::stoi(argv[2]);

    std::vector<unsigned> result;

    try {
    std::ifstream input_file(file);

    if (!input_file.is_open()) {
            throw Error(Error::CANNOT_OPEN_INPUT, "Unable to open file with input data");
        }

    std::vector<std::string> data = readFileLargeFile(file);

    auto start = get_current_time_fenced();

    switch (method_number) {
        case 1:
            result = stringstream_method(data);
            break;
        case 2:
            result = atoi_method(data);
            break;
        case 3:
            result = stod_method(data);
            break;
        case 4:
            result = from_chars_method(data);
            break;
        case 5:
            result = sscanf_method(data);
            break;
        default:
            throw Error(Error::INVALID_METHOD, "Wrong method number");
    }

    auto finish = get_current_time_fenced();
    auto total_time = finish - start;
    auto total_time_ms = std::chrono::duration_cast<std::chrono::milliseconds>(total_time).count();
    std::cout << total_time_ms << std::endl;

    uint64_t letter_count = sum_nums(result);
    std::cout << letter_count << std::endl;

    } catch (const Error& e) {
        std::cerr << e.message() << std::endl;
        return static_cast<int>(e.code());
    }

    return static_cast<int>(Error::SUCCESS);
}
