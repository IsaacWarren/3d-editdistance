Optimizing main loop memory access for cache coherence resulted in an 88% speedup with O3 and 22% with O0.

## 3 sequences of 1700 base pairs
### Compiled with -O0
Single Core on i7 3770k: 10m 16s
Single Core on Ryzen 5 2700: 9m  35s
Multi Core: Too long to test
### Compiled with -O3
Single Core on i7 3770k: 1m52s
Single Core on Ryzen 5 2700: 1m 3.5s
Multi Core: Too long to test

## 3 sequences of 1000 base pairs
### Compiled with -O0
Single Core on i7 3770k: 1m 57s
Single Core on Ryzen 5 2700: 1m 56s
Multi Core: Too long to test
### Compiled with -O3
Single Core on i7 3770k: 16s
Single Core on Ryzen 5 2700: 17s
Multi Core: Too long to test

## 3 sequences of 300 base pairs
### Compiled with -O0
Single Core on Ryzen 5 2700: 4.3s
8 threads on R5 2700: 44.5s
### Compiled with -O3
Single Core on Ryzen 5 2700: 0.5s
8 threads on R5 2700: 38s