#include <stdio.h>
#include <stdlib.h>

#include "read_sequences.h"
#include "edit_distance.h"

#define K 3
//Only  3d scoring implemented


int main(int argc, char** argv) {
    char **sequences = read_sequences(K);
    printf("%d\n", edit_distance(sequences, K));
    free_sequences(sequences, K);
    return 0;
}