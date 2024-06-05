// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <iostream>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <cctype>
#include "integration/functions.hpp"

int main(int argc, char* argv[]) {
    try {
        if (argc < 5) {
            std::cerr << "Usage: " << argv[0] << " <function_number> <cfg_file> <threads_number> <point_per_task>" << std::endl;
            return 1;
        }

        int function_number = std::stoi(argv[1]);
        std::string cfg_file = argv[2];
        cfg_file = "../" + cfg_file;
        int points_per_thread = std::stoi(argv[4]);
        std::map<std::string, double> configMap;

        try {
            configMap = parseCFG(cfg_file);
        } catch (const CustomError& fe) {
            std::cerr << fe.what() << std::endl;
            return 3;
        }

        int threads_number = std::stoi(argv[3]);
        int points_per_task = std::stoi(argv[4]);

        switch (function_number) {
            case 1:
                integrate_parallel(function_1, configMap, threads_number, points_per_thread);
                break;
            case 2:
                integrate_parallel(function_2, configMap, threads_number, points_per_thread);
                break;
            case 3:
                integrate_parallel(function_3, configMap, threads_number, points_per_thread);
                break;
            default:
                std::cerr << "Invalid function index." << std::endl;
                return 2;
        }

        return 0;
    } catch (const CustomError& e) {
        std::cerr << "Caught wrong number of threads exception: " << e.what() << std::endl;
        return 64;
    } catch (std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 16;
    }
}