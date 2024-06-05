#include <iostream>
#include <chrono>
#include <vector>
#include <future>
#include <map>
#include "functions.hpp"


void integrate_threaded(double (*func)(double, double), double x1_min, double x1_max,
                          double x2_min, double x2_max, int start_point_x1, int end_point_x1,
                          int start_point_y, int end_point_y, double init_steps_x, double init_steps_y,
                          double& result, int iter) {
    double current_integral = 0.0;
    double dx1 = (x1_max - x1_min) / init_steps_x;
    double dx2 = (x2_max - x2_min) / init_steps_y;

    for (int i = start_point_x1; i < end_point_x1; ++i) {
        for (int j = start_point_y; j < end_point_y; ++j) {
            if (iter == 0) {
                current_integral += func(x1_min + (1 + 2 * i) * dx1 / 2, x2_min + j * dx2) * dx1 * dx2 / 4;
                current_integral += func(x1_min + i * dx1, x2_min + (1 + 2 * j) * dx2 / 2) * dx1 * dx2 / 4;
                current_integral += func(x1_min + (1 + 2 * i) * dx1 / 2, x2_min + (1 + 2 * j) * dx2 / 2) * dx1 * dx2 / 4;
            } else {
                current_integral += func(x1_min + i * dx1, x2_min + j * dx2) * dx1 * dx2;
            }
        }
    }
    result = current_integral;
}

double integrate_parallel(double (*func)(double, double), std::map<std::string, double> data, int num_threads, int points_per_thread) {
    double x1_min = data.at("x_start");
    double x1_max = data.at("x_end");
    double x2_min = data.at("y_start");
    double x2_max = data.at("y_end");
    double init_steps_x = data.at("init_steps_x");
    double init_steps_y = data.at("init_steps_y");
    double abs_err = data.at("abs_err");
    double rel_err = data.at("rel_err");
    int max_iter = static_cast<int>(data.at("max_iter"));

    ThreadPool pool(num_threads);
    double final_result = 0.0;
    double abs_error = 0.0;
    double rel_error = 0.0;

    auto start_time = std::chrono::high_resolution_clock::now();

    for (int iter = 0; iter < max_iter; ++iter) {
        int total_points_x = static_cast<int>(init_steps_x);
        int total_points_y = static_cast<int>(init_steps_y);

        std::vector<std::future<double>> futures;

        for (int i = 0; i < total_points_x; i += points_per_thread) {
            int start_point_x1 = i;
            int end_point_x1 = std::min(i + points_per_thread, total_points_x);

            for (int j = 0; j < total_points_y; j += points_per_thread) {
                int start_point_y = j;
                int end_point_y = std::min(j + points_per_thread, total_points_y);

                futures.push_back(pool.submit([&, start_point_x1, end_point_x1, start_point_y, end_point_y]() {
                    double result = 0.0;
                    integrate_threaded(func, x1_min, x1_max, x2_min, x2_max, start_point_x1, end_point_x1, start_point_y, end_point_y,
                                       init_steps_x, init_steps_y, result, iter);
                    return result;
                }));
            }
        }

        double prev_final_result = final_result;
        final_result = 0.0;

        for (auto& future : futures) {
            final_result += future.get();
        }

        abs_error = std::abs(final_result - prev_final_result);
        rel_error = abs_error / std::abs(final_result);

        if (abs_error < abs_err && rel_error < rel_err) {
            break;
        }

        init_steps_y *= 2;
        init_steps_x *= 2;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
    std::cout << final_result << std::endl;
    std::cout << abs_error << std::endl;
    std::cout << rel_error << std::endl;
    std::cout << elapsed_time << std::endl;

    return final_result;
}
