#!/usr/bin/env python3

from runner import Runner

if __name__ == "__main__":

#RBTREE

    # encounter order: 1000 insert
    r = Runner('test_etx_1000')
    r.run(10)
    r.write_csv()


#SKIPLIST

