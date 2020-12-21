## How To Run
1. To generate the sequences that edit distance will be computed for run `python3 mutate.py`
2. To build the program run `make`
3. To run the single core implementation use `./main_single` for the multicore version use `./main_multi`
4. If enough ram cannot be allocated, i.e. it segfaults, for the generated sequences change SEQUENCE_LENGTH to a smaller number, redo step one and then try again.