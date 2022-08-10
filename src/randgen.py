#!/usr/bin/env python3

###############################################################
# sample program for #WeekendDevPuzzle of 2022-02-05
#
# Sample invocations:
# To generate dataset:
#     ./find.py gen 16000 4096 > data
# To count:
#     cat data | ./find.py count 9999 10000
#
# During counting step, we take the "times" parameter only to
# allow sufficiently enough time measurement
#
# During generation step, modulo is not strictly necessary.
# I've kept it here to play around with different number
# ranges.
#
###############################################################

import array
import random
import sys
import time


def read_dataset():
    """Reads numbers one line at a time and returns an array of them"""
    dataset = []
    for line in sys.stdin:
        s = line.rstrip()
        if s.isnumeric():
            dataset.append(int(s))
    return array.array('i', dataset)


def gen_dataset(count, modulo):
    """ Generate a new dataset and dump it out to stdout """
    for n in range(count):
        num = random.randint(0, modulo-1)
        print(f"{num}")


def count_times(arr, lessthan, times):
    """Return number of entries less than threshold, *times"""
    count = 0
    for i in range(times):
        for n in arr:
            if n < lessthan:
                count += 1
    return count


def usage(err = None):
    """Print usage and exit"""
    if err:
        print(f"error: {err}\n", file=sys.stderr)

    print(f"usage: {sys.argv[0]} <options>", file=sys.stderr)
    print("where options could be", file=sys.stderr)
    print("  gen <count> <modulo>", file=sys.stderr)
    print("  count <lessthan> <times>", file=sys.stderr)

    sys.exit(1 if err else 0)


if __name__ == "__main__":
    # parse arguments
    if len(sys.argv) < 2:
        usage("missing args")

    elif sys.argv[1] == "-h" or sys.argv[1] == "help":
        usage()

    elif sys.argv[1] == "gen":
        if len(sys.argv) < 4: usage("missing options for gen command")
        count = int(sys.argv[2])
        modulo = int(sys.argv[3])
        gen_dataset(count, modulo)

    elif sys.argv[1] == "count":
        if len(sys.argv) < 4: usage("missing options for count command")
        lessthan = int(sys.argv[2])
        times = int(sys.argv[3])
        data = read_dataset()
        # warm up
        count_times(data, lessthan, 1)
        # start and measure
        tt = time.process_time()
        c = count_times(data, lessthan, times)
        tt = time.process_time() - tt
        print(f"Number of matching entries = {c}. Time taken = {tt}s")