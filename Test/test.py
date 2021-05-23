#!/usr/bin/env python3

from runner import Runner

if __name__ == "__main__":
    r = Runner('./test_etx_1000')
    r.run(100)
    mean, sd = r.get_stats()
    print("mean: {}\nsd: {}".format(mean, sd))