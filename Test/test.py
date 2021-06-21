#!/usr/bin/env python3

from runner import Runner

if __name__ == "__main__":

    # RED-BLACK TREE
    #r = Runner('rbt_etx', 10000)
    #r.run(1)
    #r.write_csv()

    #r = Runner('rbt_etx', 100000)
    #r.run(1)
    #r.write_csv()

    #SKIPLIST

    r = Runner('rbt_cg', 10000)
    r.run(10)
    r.write_csv()

    #r = Runner('skip_etx', 100000)
    #r.run(1)
    #r.write_csv()
