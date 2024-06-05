"Auto run module for cpp"

import argparse
import subprocess
import statistics

FILE_NAME = "build/countwords_par_tbb"
CFG_FILE = "first.cfg"

def flush():
    """Flushes cache"""
    commands = [
        "sync",
        "echo 3 > /proc/sys/vm/drop_caches"
    ]
    for cmd in commands:
        subprocess.run(cmd, shell=True, check=True)

def read_cfg_file(filename, comment_char='#'):
    'Reads cfg'
    cfg_dict = {}
    with open(filename, 'r', encoding='utf-8') as file:
        for line in file:
            line = line.strip()
            if line and not line.startswith(comment_char):
                parts = line.split(comment_char, 1)
                key_value_part = parts[0].strip()
                if key_value_part:
                    key, value = key_value_part.split('=', 1)
                    cfg_dict[key.strip()] = value.strip()
    return cfg_dict

def init_args() -> argparse.Namespace:
    "Inits args"
    parser = argparse.ArgumentParser()
    parser.add_argument('times_to_run', type=int)
    parser.add_argument('flush', type=int)

    args = parser.parse_args()
    return args

def dct_creator(output_file_path):
    "Read file to dict"
    res = {}
    with open(output_file_path, "r", encoding='utf-8') as file:
        for line in file:
            key, value = [item.strip() for item in line.split("\n")]
            res[key] = value
    return res

def run(times_to_run, do_flush):
    "Run func"
    output_file_path = read_cfg_file(CFG_FILE)['out_by_n']
    if output_file_path.startswith('"'):
        output_file_path = output_file_path[1:]
    if output_file_path.endswith('"'):
        output_file_path = output_file_path[:-1]
    total_lst = []
    write_lst = []
    finding_lst = []
    reading_lst = []
    previous_dct = {}
    for run_num in range(times_to_run):
        tmp = subprocess.run(f'{FILE_NAME} {CFG_FILE}',
            shell = True,
            check = True,
            text = True,
            stdout = subprocess.PIPE)
        if tmp.returncode == 0:
            result_total = int(tmp.stdout.split("\n")[0].split("=")[1])
            total_lst.append(result_total)
            result_finding = int(tmp.stdout.split("\n")[1].split("=")[1])
            finding_lst.append(result_finding)
            result_reading = int(tmp.stdout.split("\n")[2].split("=")[1])
            reading_lst.append(result_reading)
            result_writing = int(tmp.stdout.split("\n")[3].split("=")[1])
            write_lst.append(result_writing)
            if run_num == 0:
                previous_dct = dct_creator(output_file_path)
            else:
                new_dct = dct_creator(output_file_path)
                if new_dct != previous_dct:
                    print("Results are not the same")
                    return
                previous_dct = new_dct
            if do_flush:
                flush()
        else:
            print(f"Program ended wrongly with code: {tmp.returncode}")
            return
            
    print(min(total_lst))
    print(statistics.mean(total_lst))
    print(statistics.stdev(total_lst))
    print()
    print(min(finding_lst))
    print(statistics.mean(finding_lst))
    print(statistics.stdev(finding_lst))
    print()
    print(min(reading_lst))
    print(statistics.mean(reading_lst))
    print(statistics.stdev(reading_lst))
    print()
    print(min(write_lst))
    print(statistics.mean(write_lst))
    print(statistics.stdev(write_lst))
    

def main():
    "Main func"
    args = init_args()
    if args.times_to_run < 2:
        print("Times_to_run must be at least 2")
        return
    if args.flush not in [0, 1]:
        print("Flush must be 1 or 0")
        return
    run(args.times_to_run, True if args.flush == 1 else False)

if __name__ == "__main__":
    main()