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
//#include "integration/integrate_serial.hpp"

int main(int argc, char* argv[]) {
    try {
        if (argc < 3) {
            std::cerr << "Usage: " << argv[0] << " <function_number> <cfg_file>" << std::endl;
            return 1;
        }

        int function_number = std::stoi(argv[1]);
        std::string cfg_file = argv[2];
        cfg_file = "../" + cfg_file;

        std::map<std::string, double> configMap;

        try {
            configMap = parseCFG(cfg_file);
        } catch (const FileException& fe) {
            std::cerr << "FileException: " << fe.what() << std::endl;
            return 3;
        }


        switch (function_number) {
            case 1:
                integrate_serial(function_1, configMap);
                break;
            case 2:
                integrate_serial(function_2, configMap);
                break;
            case 3:
                integrate_serial(function_3, configMap);
                break;
            case 4:
                integrate_serial(function_4, configMap);
                break;
            default:
                std::cerr << "Invalid function index." << std::endl;
                return 2;
        }

        return 0;
    } catch (FileException& e) {
        std::cerr << "FileException: " << e.what() << std::endl;
        return 5;
    } catch (std::runtime_error& e) {
        std::cerr << "Runtime error: " << e.what() << std::endl;
        return 16;
    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 4;
    }
}