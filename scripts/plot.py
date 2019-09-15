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


def main():
    file_blocks, ranges = list(get_downloaded_ranges())

    to_download = 0
    for r in ranges:
        to_download += r[1] - r[0] + 1

    h_fields = 128

    # add one extra row
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


if __name__ == "__main__":
    sys.exit(main())
