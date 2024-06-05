import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('../data/data.csv')

L1 = 178.0

speedup = []
efficiency = []
threads = []

for n in range(1, 17):
    Ln = data.loc[data['Threads'] == n, 'Minimal Time'].values[0]
    Sn = L1 / Ln
    En = Sn / n
    speedup.append(Sn)
    efficiency.append(En)
    threads.append(n)

plt.figure(figsize=(10, 6))
plt.plot(threads, efficiency, marker='o')
plt.xlabel('Number of Threads')
plt.ylabel('Efficiency of Parallelization, E(n)')
plt.title('Efficiency of Parallelization')
plt.grid(True)
plt.show()