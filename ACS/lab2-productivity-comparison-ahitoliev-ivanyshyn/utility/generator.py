import random


num_entries = 5 * 10**7
entry_range = 10**8
with open('data/data.txt', 'w') as f:
    for _ in range(num_entries):
        entry = random.randint(0, entry_range)
        f.write(str(entry) + '\n')