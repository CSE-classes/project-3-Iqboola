## Assignment 3: my_list_forming Performance vs Original

| K    | num_threads | Original (us) | Optimized (us) | Speedup |
|------|-------------|---------------|----------------|---------|
| 200  | 4           | 943           | 817            | ~1.2x   |
| 200  | 8           | 2088          | 1828           | ~1.1x   |
| 800  | 8           | 8086          | 1312           | ~6.2x   |
| 1600 | 8           | 15086         | 1380           | ~10.9x  |

**Key optimization:** Each thread builds a local list of K nodes without 
any locking, then appends the entire local list to the global list in a 
single lock acquisition. This reduces lock acquisitions from 
(num_threads × K) down to num_threads.