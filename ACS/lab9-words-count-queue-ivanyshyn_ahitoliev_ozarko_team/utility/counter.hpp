#ifndef COUNTERS_HPP
#define COUNTERS_HPP

#include <iostream>
#include <unordered_map>
#include <string>

std::unordered_map<std::string, size_t> count_words(const std::string& input_str);

class CustomError : public std::runtime_error {
public:
    explicit CustomError(const std::string& message) : std::runtime_error(message) {}
};

#endif