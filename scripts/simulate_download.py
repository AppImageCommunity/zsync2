#! /usr/bin/env python3

import os
import requests
import sys


def get_downloaded_ranges():
    with open("zsync2_block_analysis.txt") as f:
        total_size = int(f.readline().split(":")[1])

        data = []

        line = f.readline()

        while line:
            data.append(tuple(map(int, line.split())))

            line = f.readline()

        return total_size, data


def main():
    file_size, ranges = list(get_downloaded_ranges())

    with open(os.devnull, "wb") as f:
        for i, range in enumerate(ranges):
            print("Requesting range {}".format(i))

            start, end = range

            headers = {
                "Range": "bytes={}-{}".format(start, end)
            }

            with requests.get(sys.argv[1], headers=headers, stream=True) as r:
                f.write(r.raw.read())

    print("done!")


if __name__ == "__main__":
    sys.exit(main())

