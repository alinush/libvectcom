#!/usr/bin/env python3

import matplotlib.pyplot as plt
import matplotlib.ticker as plticker
from cycler import cycler
import pandas
import sys
import time
import numpy as np

setTitle=False

if len(sys.argv) < 4:
    print("Usage:", sys.argv[0], "<output-png-file> <min-N> <max-N> <csv-file> [<csv-file>] ...")
    print()
    sys.exit(0)

del sys.argv[0]

out_png_file = sys.argv[0]
del sys.argv[0]

minN = int(sys.argv[0])
del sys.argv[0]

maxN = int(sys.argv[0])
del sys.argv[0]

data_files = [f for f in sys.argv]
if len(data_files) == 0:
    print("ERROR: You did not give me any CSV files")
    sys.exit(1)

print("Reading CSV files:", data_files, "...")

csv_data = pandas.concat((pandas.read_csv(f) for f in data_files))
csv_data.sort_values('n', inplace=True) # WARNING: Otherwise, plotting unsorted CSV file be messed up, with incorrect y-values for a x-coordinates

#print "Raw:"
#print csv_data.to_string()
#print csv_data.columns
#print csv_data['dictSize'].values

#print "Averaged:"

if minN == 0:
    minN = csv_data.n.unique().min();
if maxN == 0:
    maxN = csv_data.n.unique().max();

print("min N:", minN)
print("max N:", maxN)

csv_data.scheme.replace('fk', 'TAB+20 (quasilinear, fast)', inplace=True)
csv_data.scheme.replace('pointproofs-naive', 'Pointproofs (quadratic, slow)', inplace=True)
csv_data.scheme.replace('pointproofs-fast', 'Pointproofs (quasilinear, fast)', inplace=True)

#csv_data = csv_data[csv_data.dkg != 'Feldman']
csv_data = csv_data[csv_data.n >= minN]
csv_data = csv_data[csv_data.n <= maxN]

#print(csv_data.to_string())  # print all data

#SMALL_SIZE = 10
MEDIUM_SIZE = 20
BIG_SIZE = 24
BIGGER_SIZE = 28
BIGGEST_SIZE = 32

plt.rc('lines', linewidth=4, markersize=10, markeredgewidth=4)   # width of graph line & size of marker on graph line for data point
plt.rc('font',   size=      BIGGER_SIZE)    # controls default text sizes
#plt.rc('axes',   titlesize= BIGGER_SIZE)    # fontsize of the axes title
#plt.rc('axes',   labelsize= MEDIUM_SIZE)    # fontsize of the x and y labels
plt.rc('xtick',  labelsize= BIG_SIZE)
plt.rc('ytick',  labelsize= BIG_SIZE)
plt.rc('legend', fontsize=  BIG_SIZE)
#plt.rc('figure', titlesize=BIGGER_SIZE)     # fontsize of the figure title

    ##plt.rcParams['axes.prop_cycle'] = cycler(color='bgrcmyk')
    #plt.rcParams['axes.prop_cycle'] = cycler(color=[
    #    # for naive Lagrange lines
    #    '#1f77b4', # blue
    #    #'#9467bd', # purple/magenta/whatever,
    #    '#1f77b4', # blue
    #    
    #    # for fast Lagrange lines
    #    '#ff7f0e', # orange
    #    '#ff7f0e', # orange
    #    #'#d62728', # red

    #    # for multiexp line
    #    #'#bcbd22', # yellow
    #    #'#d62728', # red
    #    '#2ca02c', # green
    #    ])

plt.rcParams['axes.prop_cycle'] = cycler(color=[
    # for TAB+20 line
    '#ff7f0e', # orange
    # for Pointproofs slow line
    '#d62728', # red
    # for Pointproofs fast line
    '#2ca02c', # green
    ])

def plotNumbers(data):
    #logBase = 10
    logBase = 2
    
    #print "Plotting with log base", logBase

    # adjust the size of the plot: (20, 10) is bigger than (10, 5) in the
    # sense that fonts will look smaller on (20, 10)
    fig, ax1 = plt.subplots(figsize=(12,7.5))
    ax1.set_xscale("log", base=logBase)
    ax1.set_yscale("log")
    if setTitle:
        ax1.set_title('Time to compute all proofs')

    ax1.grid(True)
    #ax1.set_xlabel("Total # of signers n")
    #ax1.set_ylabel("Time (in seconds)")   #, fontsize=fontsize)
    
    plots = []
    names = []

    fileSuffix = ""
    
    # NOTE: see 'marker' and 'linestyle' and 'linewidth' options here
    # https://matplotlib.org/3.1.1/api/markers_api.html#module-matplotlib.markers
    # https://matplotlib.org/3.1.1/api/_as_gen/matplotlib.lines.Line2D.html#matplotlib.lines.Line2D.set_linestyle
    # https://matplotlib.org/3.1.1/api/_as_gen/matplotlib.lines.Line2D.html#matplotlib.lines.Line2D.set_linewidth

    # plot multiexp
    marker = 'o'
    x = data.n.unique()
    ax1.set_xticks(x)

    ax1.set_xticklabels(np.log2(x).astype(int), rotation=30)
    #locmaj = plticker.LogLocator(base=10,numticks=12)
    #ax1.yaxis.set_major_locator(locmaj)
    #locmin = plticker.LogLocator(base=10.0,numticks=12)    #,subs=(0.2,0.4,0.6,0.8)
    #ax1.yaxis.set_minor_locator(locmin)
    #ax1.yaxis.set_minor_formatter(plticker.NullFormatter())

    # plot Lagr and total times for each scheme
    schemes = data.scheme.unique()
    for scheme in schemes:
        print("Plotting scheme type: ", scheme)

        filtered = data[data.scheme == scheme]
        # NOTE: In case, we don't have enough numbers for one of the schemes
        x = filtered.n.unique()    #.unique() # x-axis is the number of players aggregating
        print("n = ", x)

        col1 = filtered.total_usec.values

        assert len(x) == len(col1)

        plt2, = ax1.plot(x, filtered.total_usec.values, linestyle="-", marker=marker) #, markersize=10, linewidth=2.0)
        plots.append(plt2)
        names.append(scheme)

    ax1.legend(plots,
               names,
               loc='upper left')#, fontsize=fontsize

    plt.tight_layout()

    out_file = out_png_file
    print("Saving graph to", out_file)

    #date = time.strftime("%Y-%m-%d-%H-%M-%S")
    plt.savefig(out_file, bbox_inches='tight')
    plt.close()

plotNumbers(csv_data)

#plt.xticks(fontsize=fontsize)
#plt.yticks(fontsize=fontsize)
plt.show()
