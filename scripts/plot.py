#!/usr/bin/python3

import matplotlib
matplotlib.use('Agg')

import os
import sys
import argparse
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.backends.backend_pdf import PdfPages

def get_cpu_name():
    if 'darwin' == sys.platform:
        return os.popen("sysctl -n machdep.cpu.brand_string").read().strip()

    with open('/proc/cpuinfo') as f:
        for line in f:
            if line.startswith('model name'):
                return line.split(':', 1)[-1].strip()

    return 'unknown CPU'

def plot_levels(input, attempt, output, cpu_name):
    page_table_size = 1024 * 4
    ncache_lines = page_table_size / 64

    expected = np.loadtxt(os.path.join(input, '{}-reference.csv'.format(attempt)))
    solutions = np.loadtxt(os.path.join(input, '{}-solutions.csv'.format(attempt)))

    with PdfPages(output) as pdf:
        for i in range(1, len(expected) + 1):
            plt.figure()
            data = np.loadtxt(os.path.join(input, '{}-level{}.csv'.format(attempt, i)))
            data = np.array([(row - np.min(row)) / ((np.max(row) - np.min(row) or np.max(row))) for row in data])
            #data = data > 0.1

            plt.pcolormesh(data, cmap=plt.cm.Blues, vmin=0, vmax=1)

            [npages_per_line, line, page] = expected[i - 1]

            ys = np.arange(0, data.shape[0] + npages_per_line, npages_per_line) - page
            xs = (line + (ys + page) // npages_per_line) % data.shape[1]

            for x, y in zip(xs, ys):
                rect = patches.Rectangle((x, y), 1, npages_per_line, linewidth=1,
                    edgecolor='lime', facecolor='none', hatch='/' * 8)
                plt.gca().add_patch(rect)

            [npages_per_line, line, page] = solutions[i - 1]

            ys = np.arange(0, data.shape[0] + npages_per_line, npages_per_line) - page
            xs = (line + (ys + page) // npages_per_line) % data.shape[1]

            for x, y in zip(xs, ys):
                rect = patches.Rectangle((x, y), 1, npages_per_line, linewidth=1,
                    edgecolor='red', facecolor='none')
                plt.gca().add_patch(rect)

            plt.xlabel('Cache line offset in page table')
            plt.ylabel('Consecutive pages')
            plt.axis([0, data.shape[1], 0, data.shape[0]])
            plt.title('Level {} signal'.format(i))

            pdf.savefig()
            plt.close()

        d = pdf.infodict()
        d['Title'] = 'AnC signal ({})'.format(cpu_name)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('-i', '--input', action='store', default='results')
    parser.add_argument('-o', '--output', action='store', default='mmugram.pdf')
    parser.add_argument('--cpu-name', action='store')
    parser.add_argument('--attempt', action='store', type=int,
        default=0)
    args = parser.parse_args()

    plot_levels(args.input, args.attempt, args.output,
        args.cpu_name or get_cpu_name())

if __name__ == '__main__':
    main()
