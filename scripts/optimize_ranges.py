#! /usr/bin/env python3

from matplotlib import pyplot as plt

import math
import os
import sys


def get_downloaded_ranges():
    with open("zsync2_block_analysis.txt") as f:
        total_size = int(f.readline().split(":")[1]) // 4096

        data = []

        line = f.readline()

        while line:
            pair = tuple(map(int, (i / 4096 for i in map(int, line.split()))))

            data.append(pair)

            line = f.readline()

        return total_size, data


def plot(ranges, file_blocks, h_fields=128):
    v_fields = math.ceil(25692 / h_fields) + 1

    blocks = [0 for i in range(h_fields * v_fields)]

    for i in range(file_blocks):
        blocks[i] = 1

    for i, (start, end) in enumerate(ranges):
        for i in range(start, end+1):
            blocks[i] = 2

    rows = []
    for i in range(v_fields):
        cols = blocks[i*h_fields:(i+1)*h_fields]
        rows.append(cols)

    plt.imshow(rows)
    plt.show()


def main():
    file_blocks, ranges = list(get_downloaded_ranges())

    to_download = 0
    for r in ranges:
        to_download += r[1] - r[0] + 1

    optimized_ranges = [list(ranges[0])]

    for i, r in enumerate(ranges[1:]):
        dist = r[0] - optimized_ranges[-1][1]

        if dist <= 64:
            # extend previous range
            optimized_ranges[-1][1] = r[1]
            continue

        optimized_ranges.append(list(r))

    print(optimized_ranges)

    optimized_to_download = 0
    for r in optimized_ranges:
        print( r[1] - r[0] + 1)
        optimized_to_download += r[1] - r[0] + 1

    print("to_download vs optimized_to_download: {} vs. {}".format(to_download, optimized_to_download))

    plot(optimized_ranges, file_blocks)


if __name__ == "__main__":
    sys.exit(main())
