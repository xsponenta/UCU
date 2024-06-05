#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <numeric>
#include <future>
#include <iomanip>
#include <codecvt>
#include <map>
#include <sstream>
#include <thread>
#include <options_parser.h>
#include <queue>
#include <condition_variable>

std::unordered_map<std::wstring, std::vector<double>> read_file(const std::string& filename) {
    std::unordered_map<std::wstring, std::vector<double>> station_temps;
    std::wifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return station_temps;
    }

    file.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));

    std::wstring line;
    while (std::getline(file, line)) {
        std::size_t delimiter = line.find(L';');
        if (delimiter != std::wstring::npos) {
            std::wstring station = line.substr(0, delimiter);
            double temperature = std::stod(line.substr(delimiter + 1));
            station_temps[station].push_back(temperature);
        }
    }

    file.close();
    return station_temps;
}

std::tuple<double, double, double> compute_stats(const std::vector<double>& temperatures) {
    double min_temp = *std::min_element(temperatures.begin(), temperatures.end());
    double max_temp = *std::max_element(temperatures.begin(), temperatures.end());
    double mean_temp = std::accumulate(temperatures.begin(), temperatures.end(), 0.0) / temperatures.size();
    return std::make_tuple(min_temp, mean_temp, max_temp);
}

std::string format_stats(double min_temp, double mean_temp, double max_temp) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << min_temp << "/" << mean_temp << "/" << max_temp;
    return ss.str();
}

int main() {
    const std::string filename = "../data.txt";
    const int max_threads = 8;

    std::unordered_map<std::wstring, std::vector<double>> station_temps = read_file(filename);
    std::map<std::wstring, std::tuple<double, double, double>> station_stats;
    std::mutex mutex;
    std::queue<std::pair<std::wstring, std::vector<double>>> task_queue;
    std::condition_variable cv;
    bool finished = false;


    for (auto& pair : station_temps) {
        task_queue.push(std::move(pair));
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < max_threads; ++i) {
        threads.emplace_back([&]() {
            while (true) {
                std::pair<std::wstring, std::vector<double>> task;
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    cv.wait(lock, [&]() { return !task_queue.empty() || finished; });
                    if (task_queue.empty() && finished) {
                        break;
                    }
                    task = std::move(task_queue.front());
                    task_queue.pop();
                }
                auto stats = compute_stats(task.second);
                {
                    std::lock_guard<std::mutex> lock(mutex);
                    station_stats[task.first] = stats;
                }
            }
        });
    }

    {
        std::unique_lock<std::mutex> lock(mutex);
        finished = true;
    }
    cv.notify_all();
    for (auto& thread : threads) {
        thread.join();
    }

    std::ofstream output_file("../output.txt");
    if (output_file.is_open()) {
        output_file << "{";
        for (const auto& pair : station_stats) {
            output_file << std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(pair.first) << "=";
            output_file << format_stats(std::get<0>(pair.second), std::get<1>(pair.second), std::get<2>(pair.second)) << ", ";
        }
        output_file.seekp(-2, std::ios_base::end);
        output_file << "}";
        output_file.close();
    } else {
        std::cerr << "Error opening output file!" << std::endl;
    }

    return 0;
}
