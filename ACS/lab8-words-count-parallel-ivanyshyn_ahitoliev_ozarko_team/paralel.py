import os
import sys
import threading
from queue import Queue
import time
import zipfile
from collections import defaultdict
import unicodedata
from collections import defaultdict

def count_words(input_str):
    normalized_lower = input_str.decode('utf-8').lower()
    words = normalized_lower.split()
    word_counts = defaultdict(int)
    for word in words:
        word_counts[word] += 1

    return word_counts



def split(s, delimiter):
    return s.split(delimiter)[0]  # Return the first element before the delimiter

def trim_quotes(s):
    s = s.strip()  # Remove leading and trailing spaces
    s = s.split('#')[0].strip()  # Remove comments
    if len(s) >= 2 and s[0] == '"' and s[-1] == '"':
        return s[1:-1]  # Return the string without quotes
    return s


def file_filler(input_dir, txt_queue, zip_queue, max_file_size, indexing_extensions, archives_extensions):
    t1 = time.time()
    for root, _, files in os.walk(input_dir):
        for file in files:
            file_path = os.path.join(root, file)
            file_size = os.path.getsize(file_path)
            _, extension = os.path.splitext(file)
            if extension in indexing_extensions and file_size <= max_file_size:
                txt_queue.put(file_path)
            elif extension in archives_extensions and file_size <= max_file_size:
                zip_queue.put(file_path)
    txt_queue.put(None)
    t2 = time.time()
    t3 = t2 - t1
    return t1, t2, t3

def unpacker(txt_queue, zip_queue):
    t1 = time.time()
    while True:
        file_path = zip_queue.get()
        if file_path is None:
            zip_queue.put(None)
            break
        with zipfile.ZipFile(file_path, 'r') as zip_ref:
            for file_name in zip_ref.namelist():
                txt_queue.put(os.path.join(file_path, file_name))
    t2 = time.time()
    t3 = t2 - t1
    return t1, t2, t3

def binary_filler(txt_queue, binary_queue):
    t1 = time.time()
    while True:
        file_path = txt_queue.get()
        if file_path is None:
            txt_queue.put(None)
            break
        with open(file_path, 'rb') as file:
            content = file.read()
            binary_queue.put(content)
    binary_queue.put(b'End')
    t2 = time.time()
    t3 = t2 - t1
    return t1, t2, t3

def dictionary_filler(binary_queue, dict_queue):
    while True:
        content = binary_queue.get()
        if content == b'End':
            binary_queue.put(b'End')
            break
        word_counts = defaultdict(int)
        # Assuming count_words is implemented elsewhere
        tmp_counts = count_words(content)
        for word, count in tmp_counts.items():
            word_counts[word] += count
        dict_queue.put(word_counts)
def merger(dict_queue):
    poison = "A"
    while True:
        one = dict_queue.get()
        if one == poison:
            dict_queue.put(poison)
            break
        two = dict_queue.get()
        if two == poison:
            dict_queue.put(one)
            dict_queue.put(poison)
            break
        for word, count in two.items():
            one[word] += count
        dict_queue.put(one)


def main():
    if len(sys.argv) != 2:
        print("Usage: python script.py <config_file>")
        return 1

    config_file = sys.argv[1]

    if not os.path.exists(config_file):
        print(f"Error: Configuration file '{config_file}' does not exist.")
        return 2

    # Parse configuration file and setup variables

    # Read configuration file and set variables
    with open(config_file, 'r') as f:
        config_data = f.read().splitlines()

    config = {}
    for line in config_data:
        key, value = map(str.strip, line.split('='))
        config[key] = value

    input_dir = trim_quotes(config.get('indir', ''))
    out_by_a = trim_quotes(config.get('out_by_a', ''))
    out_by_n = trim_quotes(config.get('out_by_n', ''))
    indexing_extensions = trim_quotes(config.get('indexing_extensions', '.txt'))
    archives_extensions = trim_quotes(config.get('archives_extensions', '.zip'))
    max_file_size = trim_quotes(config.get('max_file_size', 10000000000000000))
    indexing_threads = trim_quotes(config.get('indexing_threads', 8))
    merging_threads = trim_quotes(config.get('merging_threads', 8))
    filenames_queue = trim_quotes(config.get('filenames_queue_size', 10000))
    raw_files_queue = trim_quotes(config.get('raw_files_queue_size', 1000))
    dictionaries_queue = trim_quotes(config.get('dictionaries_queue_size', 1000))

    # Verify input directory existence
    if not os.path.exists(input_dir) or not os.path.isdir(input_dir):
        print(f"Error: Input directory '{input_dir}' does not exist or is not a directory.")
        return 3



    # Start threads
    thread_amount = 4
    merge_amount = 2

    file_thread = threading.Thread(target=file_filler, args=(input_dir, filenames_queue, raw_files_queue, max_file_size, indexing_extensions, archives_extensions))
    file_thread.start()

    zip_thread = threading.Thread(target=unpacker, args=(filenames_queue, raw_files_queue))
    zip_thread.start()

    binary_thread = threading.Thread(target=binary_filler, args=(raw_files_queue, dictionaries_queue))
    binary_thread.start()

    worker_threads = []
    for _ in range(thread_amount):
        thread = threading.Thread(target=dictionary_filler, args=(dictionaries_queue, raw_files_queue))
        worker_threads.append(thread)
        thread.start()

    merge_threads = []
    for _ in range(merge_amount):
        thread = threading.Thread(target=merger, args=(dictionaries_queue,))
        merge_threads.append(thread)
        thread.start()

    # Wait for all threads to finish
    file_thread.join()
    zip_thread.join()
    binary_thread.join()
    for thread in worker_threads:
        thread.join()
    for thread in merge_threads:
        thread.join()

    # Process results and write to files

if __name__ == "__main__":
    main()
