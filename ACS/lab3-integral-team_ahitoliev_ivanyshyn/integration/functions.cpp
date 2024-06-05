//#include "functions.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

double function_1(double x1, double x2) {
    double sum = 0.002;

    for (int i = -2; i <= 2; i++) {
        for (int j = -2; j <= 2; j++) {
            sum += 1 / (5.0 * (i + 2) + j + 3 + pow(x1 - 16 * j, 6) + pow(x2 - 16 * i, 6));
        }
    }
    return pow(sum, -1);
}

double function_2(double x1, double x2) {
    double a = 20.0;
    double b = 0.2;
    double c = 2*M_PI;

    double result = -a * exp(-b * sqrt(0.5 * (pow(x1, 2) + pow(x2, 2)))) - exp(0.5 * (cos(c * x1) + cos(c * x2))) + a + exp(1);

    return result;
}

double function_3(double x_1, double x_2) {
    double c[5] = {2, 1, 4, 7, 2};
    double a[5][2] = {{1, 4}, {2, 5}, {1, 1}, {1, 2}, {5, 4}};
    double sum = 0;

    for (int i = 0; i < 5; i++) {
        sum += c[i] * exp((-1.0 / M_PI) *
                        (pow(x_1 - a[i][0], 2) + pow(x_2 - a[i][1], 2))) *
             cos(M_PI * (pow(x_1 - a[i][0], 2) + pow(x_2 - a[i][1], 2)));
    }
    return -sum;
}

double function_4(double x_1, double x_2){
    double sum_a = 0.0;
    double sum_b = 0.0;
    int m = 5;
    for (int i = 1; i <= m; i++){
        sum_a += i* cos((i+1)*x_1+1);
        sum_b += i* cos((i+1)*x_2+1);
    }
    return -sum_a + sum_b;
}


class FileException : public std::runtime_error {
public:
    FileException(const std::string& message) : std::runtime_error(message) {}
};

std::map<std::string, double> parseCFG(std::string& filename) {
    std::map<std::string, double> configMap;

    std::ifstream configFile(filename);
    if (!configFile.is_open()) {
        throw FileException("Error opening configuration file: " + filename);
    }

    std::vector<std::string> required_args = {
        "abs_err", "rel_err", "x_start", "x_end", "y_start", "y_end",
        "init_steps_x", "init_steps_y", "max_iter"
    };

    std::string line;
    while (std::getline(configFile, line)) {
        std::istringstream iss(line);
        std::string key;
        double value;

        if (std::getline(iss, key, '=') && iss >> value) {
            key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
            configMap[key] = value;
            auto it = find(required_args.begin(), required_args.end(), key);
            if (it != required_args.end()) {
                required_args.erase(it);
            }
        }
    }

    configFile.close();

    if (!required_args.empty()) {
        std::string missingArgs;
        for (const auto& arg : required_args) {
            missingArgs += arg + " ";
        }
        throw FileException("Missing required arguments: " + missingArgs);
    }

    return configMap;
}

