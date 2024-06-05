import subprocess
import sys
import statistics
import csv

def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <executable> <num_times> <threads_number>")
        return 1

    executable = sys.argv[1]
    num_times = int(sys.argv[2])
    threads_number = int(sys.argv[3])
    epsilon = 10**-7

    with open('../data/output.csv', 'w', newline='') as csvfile:
        fieldnames = ['Function', 'Integral Value', 'Absolute Error', 'Relative Error', 'Minimal Time', \
                      'Average Time', 'Corrected Sample Standard Deviation', 'Threads']
        writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
        writer.writeheader()

        for j in range(1, 4):
            prev_result = None
            check = False
            times = []
            for i in range(1, num_times + 1):
                cfg_file = f"func{j}.cfg"
                result = subprocess.run(["./" + executable, str(j), cfg_file, str(threads_number)], stdout=subprocess.PIPE)

                if result.returncode == 0:
                    output_lines = result.stdout.decode().splitlines()
                    if output_lines:
                        integral_value = float(output_lines[0])
                        abs_error = float(output_lines[1])
                        rel_error = float(output_lines[2])
                        time = float(output_lines[3])
                        times.append(time)

                        if prev_result is not None:
                            diff = abs(prev_result - integral_value)
                            if diff < epsilon:
                                check = True
                            else:
                                check = False
                        prev_result = integral_value

            if check:
                print(f"Function {j}:\n")
                print(f"Integral value: {integral_value}")
                print(f"Absolute error: {abs_error}")
                print(f"Relative error: {rel_error}")
                print(f"Minimal time: {min(times)}")
                print(f"Average time: {sum(times) / len(times)}")
                print(f"Corrected sample standard deviation: {statistics.stdev(times)}")
                writer.writerow({'Function': j, 'Integral Value': integral_value, 'Absolute Error': abs_error, \
                                 'Relative Error': rel_error, 'Minimal Time': min(times), 'Average Time': sum(times) / len(times), \
                                    'Corrected Sample Standard Deviation': statistics.stdev(times), 'Threads': threads_number})
            else:
                print("Result is out of error range")
                break

if __name__ == "__main__":
    main()
