# Lab work <mark>3</mark>: <mark>Integral calculation</mark>
Authors (team): Andrii Ahitoliev, Ihor Ivanyshyn
## Prerequisites

c++11 and later

cmake 3.10 and later

## Compilation

All the .cpp and .hpp files essential for the lab are located in ```integration``` directory. main file and configs are located in root.

To compile the program, user first needs to create a ```build``` directory (```mkdir build```).

After that, type ```cd build``` into console. Then, run ```cmake -DCMAKE_BUILD_TYPE=Release ..``` and ```make``` in that order.
In build directory you will find ```integrate_serial``` executive file.

As example, to run third function with third config file, run following command:
```./integrate_serial 3 func3.cfg```. Example output (for all 3 cases):

<img width="554" alt="Screenshot 2024-02-29 at 21 10 57" src="images/Screenshot 2024-03-11 at 23.40.38.png">


## Usage

In ```utility``` directory you will find a ```utility.py``` script. To run the script, go to ```utility``` dir and use following command (as example):
```python3 ./utility.py ../build/integrate_serial 100```. It will run each of the functions exactly 100 times and display info about each function.

This python script also creates a ```output.csv``` file and puts it into ```data``` directory.

This is how it looks in the console:

<img width="512" alt="image" src="images/Screenshot 2024-03-11 at 23.33.00.png">

## Results

As a result our team managed to test the created integrating function in C++ on multiple complex functions of 2 variables.

For the analysis let's look at the results the python script produced for us in the picture below.

<img width="512" alt="image" src="images/Screenshot 2024-03-11 at 23.37.13.png">

#### Function 1

The integral value is quite large, and the absolute error is very small, which means that the integration function has performed well. The relative error is also extremely small, indicating a high level of accuracy. The average time taken is the highest among the three functions.

#### Function 2

The integral value is significantly lower than that of Function 1, and the absolute error is larger. However, the relative error is still small, suggesting that the integration function has performed well. The average time taken is significantly less than that for Function 1, which could be due to the function being less complex and requiring fewer iterations to reach the desired accuracy.

#### Function 3

The absolute and relative errors are the smallest among the three functions, indicating a high level of accuracy. The average time taken is the least among the three functions, which could be due to the function being less complex and requiring fewer iterations to reach the desired accuracy.

In all three cases, the absolute and relative errors are below the specified thresholds (abs_err = 0.1, rel_err = 0.001 as used in all configs), indicating that the integration function has performed well within the desired accuracy.

In addition, the deviation values are also low for all three cases, lowest for the second function. This means that time spent calculating each is roughly the same, the difference may be caused by specific machine the code runs on.

Results of launching for different configuration, the abs_err parameter is changed to 0.001:

<img width="512" alt="image" src="images/Screenshot 2024-03-11 at 23.53.46.png">

The time has risen for first two function significantly, bacause we ask of program to be 100 times more accurate. Still, it takes less then a minute to compute the result.

More interesting is to compare last two outputs. The time for third function with configuration of abs_err=0.001 is still ~70 ms. But once it is changed 0.000001 in config file, we see a significant change - now it takes 18995 to compute.

That is because on i-th iteration of execution the function is already computed accurately enough when it breakes on codition, so errors like 0.1, 0.01 and 0.001 do not make much difference. Once we make it even smaller, naturaly we get more time on computation. Specific function can be more complex than the other, also the value of integral matters (for 3rd function it is the lowest, so it takes less times to compute), that is why it may take more time to reach the answer inside the error range.

#### Integrationg function

The function uses a form of the Rectangular techinque for integration. It follows the approach used in one dimension and extrapolates to 2 dimesions. This technique approximates the definite integral by summing the areas of rectangles under the curve. The function also uses an approach, where the number of steps doubles in each iteration until the error is below a certain threshold (meaning until it it reached abs_err **or** rel_err). It gives some optimization to the calculation.


# Additional tasks
none

