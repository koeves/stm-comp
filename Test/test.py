#!/usr/bin/env python3

from runner import Runner

if __name__ == "__main__":

    # RED-BLACK TREE

    r = Runner('rbt_etx_1000')
    r.run(100)
    r.write_csv()

    """ r = Runner('rbt_etx_tests_100000')
    r.run(100)
    r.write_csv()

    #SKIPLIST

    r = Runner('skip_etx_tests_1000')
    r.run(100)
    r.write_csv()

    r = Runner('skip_etx_tests_100000')
    r.run(100)
    r.write_csv() """