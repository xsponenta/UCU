#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <map>
#include <string>
#include <stdexcept>
#include <deque>
#include <thread>
#include <mutex>
#include <future>
#include <iostream>
#include <vector>

class CustomError : public std::runtime_error {
public:
    explicit CustomError(const std::string& message) : std::runtime_error(message) {}
};

double function_1(double x1, double x2);
double function_2(double x1, double x2);
double function_3(double x1, double x2);
std::map<std::string, double> parseCFG(std::string& filename);
void integrate_threaded(double (*func)(double, double), double x1_min, double x1_max,
                        double x2_min, double x2_max, int start_point_x1, int end_point_x1,
                        int start_point_y, int end_point_y, double init_steps_x, double init_steps_y,
                        double& result, int iter);
double integrate_parallel(double (*func)(double, double), std::map<std::string, double> data, int num_threads, int points_per_thread);
//class FileException : public std::runtime_error {
//public:
//    FileException(const std::string& message);
//};

template <typename T>
class ThreadSafeQueue {
public:
    void enqueue(T item) {
        std::unique_lock<std::mutex> lock(mut);
        queue.push_back(item);
        data_cond.notify_one();
    }

    T dequeue() {
        std::unique_lock<std::mutex> lock(mut);
        data_cond.wait(lock, [&]{ return !queue.empty(); });
        T item = queue.front();
        queue.pop_front();
        return item;
    }

    bool empty(){
        return queue.empty();
    }

private:
    std::deque<T> queue;
    std::mutex mut;
    std::condition_variable data_cond;
};


class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { return stop || !tasks.empty(); });
                        if (stop && tasks.empty())
                            return;
                        task = std::move(tasks.dequeue());
                        // tasks.dequeue();
                    }
                    task();
                }
            });
        }
    }

    template<class F>
    auto submit(F f) -> std::future<decltype(f())> {
        auto task = std::make_shared<std::packaged_task<decltype(f())()>>(std::move(f));
        std::future<decltype(f())> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.enqueue([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &worker : threads)
            worker.join();
    }

private:
    std::vector<std::thread> threads;
    ThreadSafeQueue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};



#endif
