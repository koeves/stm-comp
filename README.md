**Concurrent Datastructure Design for Software Transactional Memory**   

Bachelor Thesis 2021    

Vrije Universiteit Amsterdam   

_Abstract_

The following paper set out to investigate how transactional memory can be applied to concurrent datastructure design. Specifically, how it can provide a clean and accessible interface in replacement of locks and built on top of lock-free primitives for concurrent algorithms. Two of such datastructures are implemented, a transactional Red-Black Tree and a Skiplist, with underlying encounter-order and commit-time transactions taking care of concurrent insertions. Results show that word-based commit-time transactions cannot be successfully applied to Red-Black Trees without imposing an unnatural design on the datastructure, due to the fact that the commit-time transaction's write-list consist of (address, value) 2-tuples that are oblivious to chain of dependencies between the writes. For Skiplists, no such restrictions exist, with both encounter-order and commit-time transactions exhibiting optimal scaling properties. As opposed to findings of other papers like \cite{tl, tl2}, which promote the use of the commit-time mechanism for high-contention, this paper finds that due to the large amount of aborting transactions, which are expected in case of high-contention, and the more expensive commit-time API operations, the encounter-order locking mechanism outperforms commit-time locking on all metrics with a factor of two. Furthermore, it can be empirically concluded that the use of the transactional algorithm design is much less error-prone than its lock based counterpart, without further restrictions on its structure posed by lock-free primitives. While performance-wise, hand-crafted concurrent datastructures might prove to be more effictive, the transactional design can open up the process to a wider range of programmers by providing a straight-forward abstraction for concurrency.

_**keywords**_: transactional memory, concurrent datastructures, parallelization
