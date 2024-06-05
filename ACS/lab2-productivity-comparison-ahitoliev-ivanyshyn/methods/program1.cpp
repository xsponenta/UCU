#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <chrono>
#include "program1.hpp"

std::vector<std::string> readFileStreamIdiomatic(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> data;
    std::string word;

    if (!file) {
        throw Error(Error::READ_ERROR, "Error reading input file");
    }

    while (file >> word) {
        data.push_back(word);
    }

    return data;
}

std::vector<std::string> readFileInMemory(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> data;
    std::stringstream buffer;

    if (!file) {
        throw Error(Error::READ_ERROR, "Error reading input file");
    }

    buffer << file.rdbuf();
    std::string word;

    while (buffer >> word) {
        data.push_back(word);
    }

    return data;
}

std::vector<std::string> readFileAsString(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> data;
    std::stringstream buffer;

    if (!file) {
        throw Error(Error::READ_ERROR, "Error reading input file");
    }

    buffer << file.rdbuf();
    std::string fileContents = buffer.str();
    size_t start_pos = 0;

    while (start_pos < fileContents.length()) {
        size_t end_pos = fileContents.find_first_of(" \t\n\r", start_pos);
        if (end_pos == std::string::npos) {
            data.push_back(fileContents.substr(start_pos));
            break;
        }
        else {
            data.push_back(fileContents.substr(start_pos, end_pos - start_pos));
            start_pos = end_pos + 1;
        }
    }

    return data;
}

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

std::vector<std::string> readFileStreamIterators(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<std::string> data;

    if (!file) {
        throw Error(Error::READ_ERROR, "Error reading input file");
    }

    std::string fileContents(std::istreambuf_iterator<char>(file), {});
    size_t start = 0;

    while (start < fileContents.length()) {
        size_t end = fileContents.find_first_of(" \t\n\r", start);
        if (end == std::string::npos) {
            data.push_back(fileContents.substr(start));
            break;
        }
        else {
            data.push_back(fileContents.substr(start, end - start));
            start = end + 1;
        }
    }

    return data;
}