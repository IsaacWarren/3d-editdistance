#include "read_sequences.h"

char **read_sequences(int num_sequences) {
    char *filename = malloc(24);
    char **seq_arr = malloc(sizeof(char*) * num_sequences);

    for (int i = 0; i < num_sequences; ++i) {
        long length;
        char *seq = NULL;
        
        sprintf(filename, "seq%d.txt", i);
        FILE *f = fopen (filename, "rb");

        if (f) {
            fseek (f, 0, SEEK_END);
            length = ftell (f);
            fseek (f, 0, SEEK_SET);
            seq = malloc (length + 1);
            seq[length] = 0;

            if (seq) {
                fread (seq, 1, length, f);
            } else {
                printf("Failed to allocate memory for %s\n", filename);
                return NULL;
            }

          fclose (f);
        } else {
            printf("Failed to read %s\n", filename);
            return NULL;
        }
        seq_arr[i] = seq;
    }
    free(filename);
    return seq_arr;
}

void free_sequences(char **sequences, int k) {
    for (int i = 0; i < k; ++i) {
        free(sequences[i]);
    }
    free(sequences);
}