import itertools
import subprocess
import sys
import statistics
import platform
import pandas as pd
from scipy.stats import wilcoxon

def clear_disk_cache():
    system = platform.system()

    if system == "Linux":
        try:
            subprocess.run(["sudo", "sh", "-c", "sync; echo 3 > /proc/sys/vm/drop_caches"])
        except Exception as e:
            print(f"Error clearing disk cache: {e}")
    elif system == "Darwin":  # macOS
        try:
            subprocess.run(["sudo", "purge"])
        except Exception as e:
            print(f"Error clearing disk cache: {e}")
    elif system == "Windows":
        try:
            subprocess.run(["wmic.exe", "pagefilecache", "delete"])
        except Exception as e:
            print(f"Error clearing disk cache: {e}")
    else:
        print("Unsupported operating system for disk cache clearing.")

def main():
    if len(sys.argv) != 5:
        print(f"Usage: {sys.argv[0]} <executable> <input_file> <num_times> <clear_cash> <output_file>")
        return 1

    num_times = int(sys.argv[2])
    input_file = sys.argv[1]
    output_file = sys.argv[4]
    clear_cache_flag = bool(int(sys.argv[3]))

    all_times = {}

    if clear_cache_flag:
        clear_disk_cache()

    for j in range(1, 6):
        prev_result = None
        check = False
        times = []

        for i in range(1, num_times + 1):
            result = subprocess.run(["../build/cxx_perf_conv", input_file, str(j)], stdout=subprocess.PIPE)

            if result.returncode == 0:
                output_lines = result.stdout.decode().splitlines()
                if output_lines:
                    num_words = int(output_lines[1])
                    time = float(output_lines[0])
                    times.append(time)

                    if prev_result is not None:
                        if prev_result == num_words:
                            check = True
                        else:
                            check = False

                    prev_result = num_words

        if check:
            min_time = int(min(times))
            avg_time = int(sum(times) / len(times))
            std_dev = round(statistics.stdev(times), 2)

            print(min_time)
            print(avg_time)
            print(std_dev)
            print()

            all_times.update({f"Function {j}": times})
        else:
            print("Results don't coincide for same function")
            break

    df = pd.DataFrame.from_dict(all_times, orient='columns')

    # Initialize an empty DataFrame to store the p-values
    p_values = pd.DataFrame(index=df.columns, columns=df.columns, data=1.0)

    # Perform the Wilcoxon signed-rank test for each pair of functions
    for i, col1 in enumerate(df.columns):
        for j, col2 in enumerate(df.columns):
            if i < j:
                _, p_value = wilcoxon(df[col1], df[col2])
                p_values.iloc[i, j] = p_value
                p_values.iloc[j, i] = p_value

    # Save the p-values DataFrame as a CSV file
    p_values.to_csv(output_file, index=True, header=True)

if __name__ == "__main__":
    main()
