comp_args = -O3 -pthread

all: main_single.o read_sequences.o edit_distance.o main_multi.o sts_queue.o
	gcc $(comp_args) -o main_single main_single.o read_sequences.o edit_distance.o sts_queue.o
	gcc $(comp_args) -o main_multi read_sequences.o edit_distance.o main_multi.o sts_queue.o

debug: main_single_debug.o read_sequences_debug.o edit_distance_debug.o main_multi_debug.o sts_queue.o
	gcc -g -pthread -o main_single_debug main_single_debug.o read_sequences_debug.o edit_distance_debug.o sts_queue.o
	gcc -g -pthread -o main_multi_debug read_sequences_debug.o edit_distance_debug.o main_multi_debug.o sts_queue.o

main_single.o: main_single.c
	gcc $(comp_args) -c main_single.c

main_multi.o: main_single.c
	gcc $(comp_args) -c main_multi.c

read_sequences.o: read_sequences.c
	gcc $(comp_args) -c read_sequences.c

sts_queue.o: sts_queue.c
	gcc $(comp_args) -pthread -c sts_queue.c

edit_distance.o: edit_distance.c
	gcc $(comp_args) -c -o $@ edit_distance.c

main_single_debug.o: main_single.c
	gcc -g -o $@ -c main_single.c

main_multi_debug.o: main_multi.c
	gcc -g -pthread -o $@ -c main_multi.c

read_sequences_debug.o: read_sequences.c
	gcc -g -o $@ -c read_sequences.c

edit_distance_debug.o: edit_distance.c
	gcc -g -pthread -c -o $@ edit_distance.c

clean:
	rm *.o