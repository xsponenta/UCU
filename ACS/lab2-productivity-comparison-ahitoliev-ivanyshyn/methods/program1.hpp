#ifndef PROGRAM1_HPP
#define PROGRAM1_HPP

std::vector<std::string> readFileStreamIdiomatic(const std::string& filename);
std::vector<std::string> readFileInMemory(const std::string& filename);
std::vector<std::string> readFileAsString(const std::string& filename);
std::vector<std::string> readFileLargeFile(const std::string& filename);
std::vector<std::string> readFileStreamIterators(const std::string& filename);

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

