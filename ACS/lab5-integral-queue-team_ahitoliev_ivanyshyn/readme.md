<mark>Template for your README. Remove all unused parts and instructions</mark>

# Lab work <mark>5</mark>: <mark>Integral calculation(part3)</mark>
Authors (team): <mark>Андрій Агітольєв, Ігор Іванишин</mark><br>
## Prerequisites

c++11 and later

cmake 3.10 and later

## Compilation

All the .cpp and .hpp files essential for the lab are located in ```integration``` directory. main file and configs are located in root.

To compile the program, user first needs to create a ```build``` directory (```mkdir build```).

After that, type ```cd build``` into console. Then, run ```cmake -DCMAKE_BUILD_TYPE=Release ..``` and ```make``` in that order.
In build directory you will find ```integrate_parallel_queue``` executive file.

As example, to run first function with first config file on 4 threads with 600 points per thread, run following command:
```./integrate_parallel_queue 1 func1.cfg 4 600```. Example output:

<img src="images/Screenshot 2024-03-22 at 04.43.07.png">

### Usage

In ```utility``` directory you will find a ```utility.py``` script. To run the script, go to ```utility``` dir and use following command (as example):
```python3 ./utility.py ../build/integrate_parallel 100 4 200```. It will run each of the functions exactly 100 times on 4 threads each and 200 poimts per thread and display info about each function.

This python script also creates a ```output.csv``` file and puts it into ```data``` directory.

This is how it looks in the console:

<img src="images/Screenshot 2024-03-22 at 04.42.36.png">

### Results

#### Thread Safe Queue

 The **ThreadSafeQueue** class is a thread-safe implementation of a queue data structure. It allows multiple threads to safely enqueue and dequeue items from the queue without causing data races or corruption. The class uses a std::deque to store the elements, and it employs a std::mutex and a std::condition_variable to synchronize access to the queue.

 #### Modified Algorithm Using ThreadSafeQueue

 The program creates a fixed number of threads, the number recieved from the command line. Two instances are used. First - the result queue for storing results of threads' work, used to recieve final result. Second - task queue stores the tasks for integration that threads will perform.

The main loop divides the integration interval into smaller subintervals (points_per_thread) along both the x and y axes. These subintervals are then enqueued as tasks in the task_queue.

Each thread dequeues a task from the task_queue, computes the integration for the given subinterval using the integrate_threaded function, and enqueues the result in the result_queue.

After all tasks for a given iteration are enqueued and computed, the main thread dequeues the results from the result_queue and accumulates them into final_result.

The error calculation mechanism is the same as it was, comparing current final result with the previous.

After the desired accuracy is achieved or the maximum number of iterations is reached, the main thread enqueues empty tasks to signal the worker threads to exit, and then joins all the threads.

#### Results Analysis

First the optimal number of threads and points was determined. Closest results to the previous lab are achieved with 4 threads and 100 points.

Also the choice of 50 points was chosen simply from analyzing this table, the reults with these parameters are most optimal both in time and accuracy at the same time.

Now let us test it for different numbers of threads. For test we will pick first function and plot Threads-Time diagram. We will study the average time.

<img src="images/Screenshot 2024-03-22 at 06.59.17.png">

We can clearly see that program with queue performed gradually faster and faster, it still is slower (more than twice in some places) than the program without queue on same numbers of threads.

Aside from that, using queue and proposed algorithm proved to be still much faster than the program without threading at all.

#### Coefficient of Parallelization

The is E(n) = S(n)/n. n - number of threads, we will choose as 1 to 16 and depict on graph. S(n) = L(1)/L(n), L(1) - minimal time for non-parallelized program.

As L(1) let's choose 178.0 ms.

Now let's look  at the graph:

<img src="images/Screenshot 2024-03-22 at 07.07.42.png">

The value of efficiency of 1.0 is understandable, as there is even no optimization to talk about, the ratio is basically 1.

As the number of threads increases, the efficiency of parallelization decreases. This may be a common thing in parallel computing due to various overheads associated with parallelization, such as thread creation, communication, synchronization etc. that take much time. Although, there are noticable spikes at the beginning of the graph that show short spikes in efficiency, then steadily goes downwards.

Also, near 14-16 threads the efficiency graph starts to flatten out. That is the point where hardware limitations of the machine come in place and further acceleration is impossible without upgrading hardware.

Good balance in efficiency can be found for 4 to 6 threads. Here it is relatively high.

#### Conclusion

By using thread-safe queue in this lab, we managed to distribute the workload across multiple threads, ensure thread safety, and manage efficient communication between the main thread and the worker threads. Thus, by taking advantage of parallelization we made accelerated computations with a whole new approach.
