# Lab work <mark>5</mark>: <mark>Integral calculation (part 6) </mark>
Authors (team): <mark>Андрій Агітольєв, Ігор Іванишин</mark><br>

## Prerequisites

c++11 and later

cmake 3.10 and later

## Compilation

All the .cpp and .hpp files essential for the lab are located in ```integration``` directory. main file and configs are located in root.

To compile the program, user first needs to create a ```build``` directory (```mkdir build```).

After that, type ```cd build``` into console. Then, run ```cmake -DCMAKE_BUILD_TYPE=Release ..``` and ```make``` in that order.
In build directory you will find ```integrate_parallel_tpool``` executive file.

As example, to run first function with first config file on 4 threads with 600 points per thread, run following command:
```./integrate_parallel_tpool 1 func1.cfg 4 200```. Example output:

<img alt="" src="images/Screenshot 2024-03-28 at 21.17.38.png">

### Usage

In ```utility``` directory you will find a ```utility.py``` script. To run the script, go to ```utility``` dir and use following command (as example):
```python3 ./utility.py ../build/integrate_parallel_tpool 100 4 200```. It will run each of the functions exactly 100 times on 4 threads each and 200 points per thread and display info about each function.

This python script also creates a ```output.csv``` file and puts it into ```data``` directory.

This is how it looks in the console:

<img alt="" src="images/Screenshot 2024-03-28 at 21.21.06.png">

### Results

#### Main calculation function

The program's main goal is, once again, to compute a double integral using, now, a thread pool. 
The integrate_parallel function is responsible for performing this process.

In this new implementation, the function creates an instance of the ThreadPool class, passing num_threads
to the constructor. This initializes the thread pool with the desired number of worker threads.

The function enters a loop that runs for a maximum of max_iter iterations (specified in the data map). 
In each iteration, the following steps are performed:

1. The initial step sizes (init_steps_x and init_steps_y) are determined from the data map. 


2. A vector of std::future<double> objects **futures** is created to store the results of the tasks submitted to the
thread pool.


3. The range of points for the x and y dimensions is divided into segments based on the points_per_thread value.


4. For each segment of points, a task is submitted to the thread pool using the **pool.submit** function call.
This task then calls the integrate_threaded function, which computes the integral over the specified range of points.


5. The submit function returns a std::future<double> object, which is stored in the futures vector.
After submitting all the tasks, the program waits for their completion by calling future.get() on each std::future<double>
object in the futures vector. This blocks until the corresponding task completes and returns the partial result computed by that task.
The partial results from all tasks are summed up into the **final_result** variable.

Other steps of calculation did not change.

#### Comparing the three parallelization methods

First of all let's take the optimal size for calculations. In previous lab we took
4 threads and an interval for each thread as 50. For the regular program with multithreading
from Lab 3 we will just use 4 threads. Analysis is performed over Function 1.

First let's look at the two diagrams:

<img alt="" src="images/Screenshot 2024-03-29 at 00.18.52.png">

**Thread Pool** 

The blue bars represent the thread pool implementation.

At one thread, the thread pool implementation takes the least amount of time among the three implementations (by min time parameter). 
This can be explained by the reduced overhead associated with thread creation and management, as the thread pool reuses a fixed number of worker threads.
As the number of threads increases, the average time for the thread pool implementation initially decreases, 
indicating improved performance due to parallel execution. However, beyond a certain number of threads (about 4-6 threads here), 
the average time starts to increase gradually.
This behavior can be attributed to several factors.

**Thread pool size:** 

The thread pool implementation typically creates a fixed number of worker threads based on the specified thread count. 
If the thread count exceeds the number of available CPU cores or hardware threads, the additional threads may not contribute 
to parallelism and faster computation and can instead introduce overhead due to context switching.

**Task scheduling and queueing:** 

As the number of threads increases, the overhead of queueing
and synchronization within the thread pool can become more significant, potentially overwhelming the benefits of parallelism.

In contrast to simple multithreading, the thread pool maintains a fixed number of worker threads that are reused for executing tasks. 
This takes advantage of the cost of thread creation and management over multiple tasks, leading to better performance, 
especially for problems with a large number of small tasks.

Compared to ThreadSafeQueue, after 1-2 threads, Threadpool shows higher efficiency in both average and minimal times,
demonstrating better resource management for this implementation.

Now let's look at efficiency graph.

<img alt="" src="images/Screenshot 2024-03-29 at 00.40.41.png">

The graph once again shows steady downward trend, with highest optimal values at 5-6 threads.

Beyond the peak, the efficiency of parallelization starts to decline as more threads are added.

There is a theoretical limit to the achievable speedup through parallelization due to the serial
portion of the code that **cannot be** parallelized. As more threads are added, the overhead associated with the serial
portion becomes more significant, leading to such returns.

As the number of threads continues to increase, the efficiency curve appears to approach a straight "line" form,
potentially indicating a limit beyond which adding more threads does not provide any significant benefits.

The sudden drop at 3-4 threads can be attributed to hardware reasons or accessing data on the same cache line.