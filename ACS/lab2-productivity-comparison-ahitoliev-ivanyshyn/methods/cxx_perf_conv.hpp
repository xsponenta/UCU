#ifndef CXX_PERF_CONV_HPP
#define CXX_PERF_CONV_HPP


#include <vector>
#include <string>
#include <chrono>

std::vector<unsigned> stringstream_method(std::vector<std::string> data);
std::vector<unsigned> atoi_method(std::vector<std::string> data);
std::vector<unsigned> stod_method(std::vector<std::string> data);
std::vector<unsigned> from_chars_method(std::vector<std::string> data);
std::vector<unsigned> sscanf_method(std::vector<std::string> data);

class Error {
public:
    enum ErrorCode {
        SUCCESS = 0,
        INVALID_ARGS,
        INVALID_METHOD,
        CANNOT_OPEN_INPUT,
        CANNOT_OPEN_OUTPUT,
        READ_ERROR,
        WRITE_ERROR
    };

    Error(ErrorCode code, const std::string& message)
        : code_(code), message_(message) {}

    ErrorCode code() const { return code_; }
    const std::string& message() const { return message_; }

private:
    ErrorCode code_;
    std::string message_;
};


#endif
