#include <iostream>
#include <map>
#include <chrono>
#include <iomanip>

double integrate_serial(double function_name(double, double), std::map<std::string, double> data) {
    double x1_min = data.at("x_start");
    double x1_max = data.at("x_end");
    double x2_min = data.at("y_start");
    double x2_max = data.at("y_end");
    double init_steps_x = data.at("init_steps_x");
    double init_steps_y = data.at("init_steps_y");
    double abs_err = data.at("abs_err");
    double rel_err = data.at("rel_err");
    int max_iter = static_cast<int>(data.at("max_iter"));

    double prev_integral = 0.0;
    double current_integral = 0.0;
    double steps_x = init_steps_x;
    double steps_y = init_steps_y;
    auto start_time = std::chrono::high_resolution_clock::now();


    for (int iter = 0; iter < max_iter; ++iter) {
        prev_integral = current_integral;

        double dx1 = (x1_max - x1_min) / steps_x;
        double dx2 = (x2_max - x2_min) / steps_y;

        current_integral = 0.0;
        for (int i = 0; i < steps_x; ++i) {
            for (int j = 0; j < steps_y; ++j) {
                if (iter != 0) {
                    current_integral += function_name(x1_min + (1 + 2 * i) * dx1 / 2, x2_min + j * dx2) * dx1 * dx2 / 4;
                    current_integral += function_name(x1_min + i * dx1, x2_min + (1 + 2 * j) * dx2 / 2) * dx1 * dx2 / 4;
                    current_integral += function_name(x1_min + (1 + 2 * i) * dx1 / 2, x2_min + (1 + 2 * j) * dx2 / 2) * dx1 * dx2 / 4;
                } else {
                    current_integral += function_name(x1_min + i * dx1, x2_min + j * dx2) * dx1 * dx2;
                }
            }
        }
        current_integral += 0.25 * prev_integral;
        double absolute_diff = std::abs(current_integral - prev_integral);

        steps_x *= 2;
        steps_y *= 2;
        double relative_diff = absolute_diff / std::abs(prev_integral);

        if (absolute_diff < abs_err & relative_diff < rel_err) {
            break;
        }
    }


    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << /*std::fixed << std::setprecision(15) <<*/prev_integral << std::endl;
    std::cout << std::abs(current_integral - prev_integral) << std::endl;
    std::cout << std::abs(current_integral - prev_integral) / std::abs(prev_integral) << std::endl;
    std::cout << elapsed_time << std::endl;
    return current_integral;
}
