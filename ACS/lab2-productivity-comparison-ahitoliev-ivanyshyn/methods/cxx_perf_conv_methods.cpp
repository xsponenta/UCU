#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <chrono>
#include <cstdlib>
#include <charconv>
#include <cstdio>
#include "cxx_perf_conv.hpp"
#include <atomic>

std::vector<unsigned> stringstream_method(std::vector<std::string> data) {

    std::vector<unsigned> result;
    result.reserve(data.size());

    for (const auto& str : data) {
        std::stringstream ss(str);
        unsigned num;
        ss >> num;
        result.push_back(num);
    }

    return result;
}

std::vector<unsigned> atoi_method(std::vector<std::string> data) {
    std::vector<unsigned> result;
    result.reserve(data.size());

    for (const auto& str : data) {
        unsigned num = std::atoi(str.c_str());
        result.push_back(num);
    }

    return result;
}

std::vector<unsigned> stod_method(std::vector<std::string> data) {
    std::vector<unsigned> result;
    result.reserve(data.size());

    for (const auto& str : data) {
        double num = std::stod(str);
        result.push_back(static_cast<unsigned>(num));
    }

    return result;
}

std::vector<unsigned> from_chars_method(std::vector<std::string> data) {
    std::vector<unsigned> result;
    result.reserve(data.size());

    for (const auto& str : data) {
        auto [p, ec] = std::from_chars(str.data(), str.data() + str.size(), result.emplace_back());
        if (ec != std::errc()) {
            std::cerr << "Error converting string to unsigned int using std::from_chars." << std::endl;
            return {};
        }
    }

    return result;
}

std::vector<unsigned> sscanf_method(std::vector<std::string> data) {
    std::vector<unsigned> result;
    result.reserve(data.size());

    for (const auto& str : data) {
        unsigned num;
        if (std::sscanf(str.c_str(), "%u", &num) == 1) {
            result.push_back(num);
        } else {
            std::cerr << "Error converting string to unsigned int using sscanf." << std::endl;
            return {};
        }
    }

    return result;
}