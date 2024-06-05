#ifndef INTEGRATE_PARALLEL_QUEUE_THREAD_QUEUE_HPP
#define INTEGRATE_PARALLEL_QUEUE_THREAD_QUEUE_HPP

#include <queue>
#include <condition_variable>

template<typename T>
class ThreadQueue {
private:
    std::queue<T> q;
    std::condition_variable cv;
    std::mutex m;
    size_t max_size;
public:
    ThreadQueue(size_t max_size) : max_size(max_size) {}
    void push(const T& component) {
        {
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock, [this] { return q.size() < max_size; });
            q.push(component);
        }
        cv.notify_one();
    }
    T pop() {
        T front_component{};
        {
            std::unique_lock<std::mutex> lock(m);
            cv.wait(lock, [this] { return !q.empty(); });
            front_component = q.front();
            q.pop();
        }
        return front_component;
    }
    bool empty() {
        std::unique_lock<std::mutex> lock(m);
        return q.empty();
    }
};

#endif
