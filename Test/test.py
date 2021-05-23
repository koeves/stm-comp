#!/usr/bin/env python3

from runner import Runner

if __name__ == "__main__":
    r = Runner('./test_etx_1000')
    r.run(10)
    r.write_csv()