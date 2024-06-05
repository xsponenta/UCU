#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <map>
#include <string>

double function_1(double x1, double x2);
double function_2(double x1, double x2);
double function_3(double x1, double x2);
std::map<std::string, double> parseCFG(std::string& filename);
//IntegrationResult  integrate_serial(double function_name(double, double), std::map<std::string, double> data);
double integrate_serial(double function_name(double, double), std::map<std::string, double> data);
#endif
