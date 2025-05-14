#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np

v1avg, v2avg, vnavg = [], [], []
v1err, v2err, vnerr = [], [], []
x      = list(range(1, 11))          # [1,2,…,10]

for i in range(1, 11):
    p = 2 ** i
    d = np.loadtxt(f"./{p}.out")     # same as "./%d.out" % p

    v1avg.append(np.mean(d[:, 0]))
    v2avg.append(np.mean(d[:, 1]))
    vnavg.append(np.mean(d[:, 2]))

    v1err.append(1.96*np.std(d[:, 0], ddof=1)/np.sqrt(len(d)))
    v2err.append(1.96*np.std(d[:, 1], ddof=1)/np.sqrt(len(d)))
    vnerr.append(1.96*np.std(d[:, 2], ddof=1)/np.sqrt(len(d)))

# ---- plot only after all lists have ten points ----
plt.errorbar(x, v1avg, yerr=v1err, lw=0.7,
             elinewidth=0.7, capsize=2, markersize=5,
             label='One thread')
plt.errorbar(x, v2avg, yerr=v2err, lw=0.7,
             elinewidth=0.7, capsize=2, markersize=5,
             label='Two threads')
plt.errorbar(x, vnavg, yerr=vnerr, lw=0.7,
             elinewidth=0.7, capsize=2, markersize=5,
             label='N threads')

plt.legend(loc='best')
plt.xlabel(r'N=$2^x$')
plt.ylabel('Turnaround time (ns)')
plt.tight_layout()
plt.savefig("matrix_mul_times.png", dpi=200)
plt.show()
