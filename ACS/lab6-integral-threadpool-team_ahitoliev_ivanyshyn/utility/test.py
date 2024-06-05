# import pandas as pd
# import matplotlib.pyplot as plt
#
# pool_data = pd.read_csv("pool.csv")
# queue_data = pd.read_csv("queue.csv")
# threads_data = pd.read_csv("threads.csv")
#
# fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(10, 8))
#
# bar_width = 0.2
# index = pool_data['Threads'] - 0.3
# ax1.bar(index, pool_data['Average time'], bar_width, label='Pool', yerr=pool_data['Standard Deviation'])
# index = queue_data['Threads'] - 0.1
# ax1.bar(index, queue_data['Average time'], bar_width, label='Queue', yerr=queue_data['Standard Deviation'])
# index = threads_data['Threads'] + 0.1
# ax1.bar(index, threads_data['Average time'], bar_width, label='Threads', yerr=threads_data['Standard Deviation'])
# ax1.set_xlabel('Threads')
# ax1.set_ylabel('Average Time')
# ax1.set_xticks(range(1, 17))
# ax1.legend()
#
# bar_width = 0.2
# index = pool_data['Threads'] - 0.3
# ax2.bar(index, pool_data['Min time'], bar_width, label='Pool')
# index = queue_data['Threads'] - 0.1
# ax2.bar(index, queue_data['Min time'], bar_width, label='Queue')
# index = threads_data['Threads'] + 0.1
# ax2.bar(index, threads_data['Min time'], bar_width, label='Threads')
# ax2.set_xlabel('Threads')
# ax2.set_ylabel('Minimal Time')
# ax2.set_xticks(range(1, 17))
# ax2.legend()
#
# plt.tight_layout()
# plt.show()
import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv('pool.csv')

L1 = 187.0

speedup = []
efficiency = []
threads = []

for n in range(1, 17):
    Ln = data.loc[data['Threads'] == n, 'Min time'].values[0]
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
