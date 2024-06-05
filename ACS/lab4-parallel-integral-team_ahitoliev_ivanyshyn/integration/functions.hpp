#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <map>
#include <string>
#include <stdexcept>

class CustomError : public std::runtime_error {
public:
    CustomError(const std::string& message) : std::runtime_error(message) {}
};

double function_1(double x1, double x2);
double function_2(double x1, double x2);
double function_3(double x1, double x2);
double function_4(double x1, double x2);
std::map<std::string, double> parseCFG(std::string& filename);
void integrate_threaded(double (*func)(double, double), double x1_min, double x1_max,
                        double x2_min, double x2_max, int start_point_x1, int end_point_x1,
                        int start_point_y, int end_point_y, double init_steps_x, double init_steps_y,
                        double& result, int iter);
double integrate_parallel(double (*func)(double, double), std::map<std::string, double> data, int num_threads);
class FileException : public std::runtime_error {
public:
    FileException(const std::string& message);
};
#endif
