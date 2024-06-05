# Lab work <mark>4</mark>: <mark>Integral calculation (part 2)</mark>
Authors (team): <mark>Андрій Агітольєв, Ігор Іванишин</mark><br>
## Prerequisites

c++11 and later

cmake 3.10 and later

## Compilation

All the .cpp and .hpp files essential for the lab are located in ```integration``` directory. main file and configs are located in root.

To compile the program, user first needs to create a ```build``` directory (```mkdir build```).

After that, type ```cd build``` into console. Then, run ```cmake -DCMAKE_BUILD_TYPE=Release ..``` and ```make``` in that order.
In build directory you will find ```integrate_parallel``` executive file.

As example, to run third function with third config file on 4 threads, run following command:
```./integrate_parallel 3 func3.cfg 4```. Example output:

<img width="554" alt="Screenshot 2024-02-29 at 21 10 57" src="images/Screenshot 2024-03-16 at 20.41.49.png">


## Usage

In ```utility``` directory you will find a ```utility.py``` script. To run the script, go to ```utility``` dir and use following command (as example):
```python3 ./utility.py ../build/integrate_parallel 100 4```. It will run each of the functions exactly 100 times on 4 threads each and display info about each function.

This python script also creates a ```output.csv``` file and puts it into ```data``` directory.

This is how it looks in the console:

<img width="512" alt="image" src="images/Screenshot 2024-03-16 at 20.50.43.png">


There is also a jypiter notebook file that provides analysis for the based o the outputs of the python script. The file is named ```analysis.ipynb```, plqced in ```utility```.

### Results

The general approach and function remain the same. Additionally the algorithm employs a parallel approach using multiple threads to improve the computational efficiency.

#### Key apects

The integration is parallelized using multiple threads, with each thread responsible for computing the integral over a subset of the total range.

The parallelization is achieved by dividing the range of x1 values among the available threads. For example, having 2 threads and 500 points, each thread recieves 250 points (500 / 2). Each thread computes the integration over its assigned range of x1 values and the entire range of x2 values. The individual results from each thread are stored in the results vector and then summed up to obtain the final result.

Standard std tools were used for parallelization: std::thread std::accumulate.

#### Results analysis

By launching the .ipynb file, we get a graph:

<img alt="image" src="images/Screenshot 2024-03-16 at 20.43.29.png">

*Notice, the for values of 100 and 1000 allign the threads axis are abscent, that is because the number of threads there is greater or equal to the number of points.

For a single thread (1 thread), the execution time is generally higher compared to using multiple threads, as there is no parallelization benefit (we just launch the program as in previous lab).

As the number of threads increases, the execution time decreases initially. However, beyond a certain number of threads (8-10 threads), the execution time stops decreasing or even increase slightly due to thread management and synchronization overhead. This means that at that number of threads its already too expensive (in time) to manage them. This is most probably due to the fact that with an increasing number of threads some the operations such as allocation, creation, synchronization take up relatively too much time, outweighing benefits of dividing subtasks, thus resulting in no change of time further (or even some ocasional spikes).

**The Amdahl's Law** might also be appliable here. It states that maximum speedup achievable through parallelization is limited by the non-parallelizable portion of the code. As the number of threads increases, the algorithm approaches this theoretical limit, beyond which adding more threads does not result in significant performance improvements.

The execution times for function 2 are consistently higher compared to functions 1 and 3, indicating that function 2 is more computationally expensive or has a higher complexity.

