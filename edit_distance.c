#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "sts_queue.h"
#include "edit_distance.h"

struct dp_table {
    short *table;
    uint lengths[3];
} typedef dp_table;

struct sync_table {
    u_int8_t *table;
    uint lengths[3];
    pthread_mutex_t mtx;
} typedef sync_table;

struct job {
    StsHeader* queue;
    u_int8_t num_threads;
    sync_table *sync;
    dp_table *table;
    char **sequences;
} typedef job_t;

dp_table *alloc_dp_table(char **sequences, int k) {
    long num_elements = 1;
    for (int i = 0; i < k; ++i) {
        num_elements *= strlen(sequences[i]) + 1;
    }
    dp_table *table = malloc(sizeof(dp_table));
    table->table = malloc(sizeof(short) * num_elements);
    table->lengths[0] = strlen(sequences[0]) + 1;
    table->lengths[1] = strlen(sequences[1]) + 1;
    table->lengths[2] = strlen(sequences[2]) + 1;
    return table;
}

void dealloc_dp_table(dp_table *table) {
    free(table->table);
    free(table);
}

sync_table *alloc_sync_table(char **sequences, int k) {
    long num_elements = 1;
    for (int i = 0; i < k; ++i) {
        num_elements *= strlen(sequences[i]) + 1;
    }
    sync_table *table = malloc(sizeof(sync_table));
    table->table = malloc(sizeof(u_int8_t) * num_elements);
    table->lengths[0] = strlen(sequences[0]) + 1;
    table->lengths[1] = strlen(sequences[1]) + 1;
    table->lengths[2] = strlen(sequences[2]) + 1;

    //TODO Should let threads do this concurrently
    for (int i = 0; i < table->lengths[0]; ++i) {
        for (int j = 0; j < table->lengths[1]; ++j) {
            for (int k = 0; k < table->lengths[2]; ++k) {
                int num_zeros = 0;
                if (!i) {
                    num_zeros++;
                } 
                if (!j) {
                    num_zeros++;
                }
                if (!k) {
                    num_zeros++;
                }

                if (num_zeros == 0) {
                    table->table[k + table->lengths[2] * (j + i * table->lengths[1])] = 7;
                } else if (num_zeros == 1) {
                    table->table[k + table->lengths[2] * (j + i * table->lengths[1])] = 3;
                } else {
                    table->table[k + table->lengths[2] * (j + i * table->lengths[1])] = 1;
                }
            }
        }
    }

    pthread_mutex_init(&table->mtx, NULL);
    return table;
}

void *dealloc_sync_table(sync_table *table) {
    pthread_mutex_destroy(&table->mtx);
    free(table->table);
    free(table);
}

//Decrement value at indices and return 1 if the location becomes 0
int dec_sync_table(int indices[3], sync_table *table) {
    pthread_mutex_lock(&table->mtx);
    table->table[indices[2] + table->lengths[2] * (indices[1] + indices[0] * table->lengths[1])] -= 1;
    if (!(table->table[indices[2] + table->lengths[2] * (indices[1] + indices[0] * table->lengths[1])])) {
        pthread_mutex_unlock(&table->mtx);
        return 1;
    }
    pthread_mutex_unlock(&table->mtx);
    return 0;
}

short delta(char a, char b, char c) {
    short sum = 0;
    if (a != b) {
        sum++;
    }
    if (a != c) {
        sum++;
    }
    if (b != c) {
        sum++;
    }
    return sum;
}

short get_value_at_index(uint indices[3], dp_table *table) {
    return table->table[indices[2] + table->lengths[2] * (indices[1] + indices[0] * table->lengths[1])];
}

void set_value_at_index(uint indices[3], dp_table *table, short value) {
    table->table[indices[2] + table->lengths[2] * (indices[1] + indices[0] * table->lengths[1])] = value;
}

short calculate_score(dp_table *table, char **sequences, int *indices) {
    //TODO index 0 solution
    short scores[7] = {-1,-1,-1,-1,-1,-1,-1};

    int idx_mod[3];

    if (indices[0] == 0 && indices[1] == 0 && indices[2] == 0) {
        return 0;
    }

    if (indices[0] != 0) {
        idx_mod[0] = indices[0]-1,idx_mod[1] = indices[1], idx_mod[2] = indices[2];
        scores[4] = get_value_at_index(idx_mod, table) + delta(sequences[0][indices[0] - 1], '-', '-');

        if (indices[1] != 0) {
            idx_mod[0] = indices[0] - 1; idx_mod[1] = indices[1] - 1; idx_mod[2] = indices[2];
            scores[1] = get_value_at_index(idx_mod, table) + delta(sequences[0][indices[0] - 1], sequences[1][indices[1] - 1], '-');

            if (indices[2] != 0) {
                idx_mod[0] = indices[0] - 1; idx_mod[1] = indices[1] - 1; idx_mod[2] = indices[2] - 1;
                scores[0] = get_value_at_index(idx_mod, table) + delta(sequences[0][indices[0] - 1], sequences[1][indices[1] - 1], sequences[2][indices[2] - 1]);
            }
        }

        if (indices[2] != 0) {    
            idx_mod[0] = indices[0] - 1;idx_mod[1] = indices[1];idx_mod[2] = indices[2] - 1;
            scores[2] = get_value_at_index(idx_mod, table) + delta(sequences[0][indices[0] - 1], '-', sequences[2][indices[2] - 1]);
        }
    }

    if (indices[1] != 0) {
        idx_mod[0] = indices[0];idx_mod[1] = indices[1]-1;idx_mod[2] = indices[2];
        scores[5] = get_value_at_index(idx_mod, table) + delta('-', sequences[1][indices[1] - 1], '-');
        
        if (indices[2] != 0) {
            idx_mod[0] = indices[0];idx_mod[1] = indices[1] - 1;idx_mod[2] = indices[2] - 1;
            scores[3] = get_value_at_index(idx_mod, table) + delta('-', sequences[1][indices[1] - 1], sequences[2][indices[2] - 1]);
        }
    }

    if (indices[2] != 0) {
        idx_mod[0] = indices[0];idx_mod[1] = indices[1];idx_mod[2] = indices[2] - 1;
        scores[6] = get_value_at_index(idx_mod, table) + delta('-', '-', sequences[2][indices[2] - 1]);
    }

    short min_score = __INT16_MAX__;
    int min_idx = 0;
    for (int i = 0; i < 7; ++i) {
        if (scores[i] < min_score && scores[i] != -1) {
            min_score = scores[i];
            min_idx = i;
        }
    }
    //printf("\n%d\n", min_idx);
    return min_score;
    
}

int edit_distance(char **sequences, int k) {
    // Only implemented for k=3
    dp_table *table = alloc_dp_table(sequences, k);
    for (int i = 0; i < table->lengths[0]; ++i) {
        for (int j = 0;  j < table->lengths[1]; ++j) {
            for (int k = 0; k < table->lengths[2]; ++k) {
                int indices[3] = {i,j,k};
                short value = calculate_score(table, sequences, indices);
                set_value_at_index(indices, table, value);
                //printf("%d, %d, %d: %d, %d\n", indices[0], indices[1], indices[2], value, get_value_at_index(indices, table));
            }
        }
    }

    int return_idx[] = {table->lengths[0] - 1, table->lengths[1] - 1, table->lengths[2] - 1};
    short ret_val = get_value_at_index(return_idx, table);

    dealloc_dp_table(table);
    return ret_val;
}

//Dec all indices in sync table that depend on idx, add any new 0s in sync table to queue
void sync_with_table(int *idx, sync_table *sync, StsHeader *queue, dp_table *table) {
    int idx_mod[3];
    if (idx[0] + 1 < table->lengths[0]) {
        idx_mod[0] = idx[0]+1,idx_mod[1] = idx[1], idx_mod[2] = idx[2];
        if(dec_sync_table(idx_mod, sync)) {
            int *add_idx= malloc(sizeof(int) * 3);
            memcpy(add_idx, idx_mod, sizeof(int) * 3);
            StsQueue.push(queue, add_idx);
        }

        if (idx[1] + 1 < table->lengths[1]) {
            idx_mod[0] = idx[0] + 1; idx_mod[1] = idx[1] + 1; idx_mod[2] = idx[2];
            if(dec_sync_table(idx_mod, sync)) {
                int *add_idx= malloc(sizeof(int) * 3);
                memcpy(add_idx, idx_mod, sizeof(int) * 3);
                StsQueue.push(queue, add_idx);
            }

            if (idx[2] + 1 < table->lengths[2]) {
                idx_mod[0] = idx[0] + 1; idx_mod[1] = idx[1] + 1; idx_mod[2] = idx[2] + 1;
                if(dec_sync_table(idx_mod, sync)) {
                    int *add_idx= malloc(sizeof(int) * 3);
                    memcpy(add_idx, idx_mod, sizeof(int) * 3);
                    StsQueue.push(queue, add_idx);
                }
            }
        }

        if (idx[2] + 1 < table->lengths[2]) {    
            idx_mod[0] = idx[0] + 1;idx_mod[1] = idx[1];idx_mod[2] = idx[2] + 1;
            if(dec_sync_table(idx_mod, sync)) {
                int *add_idx= malloc(sizeof(int) * 3);
                memcpy(add_idx, idx_mod, sizeof(int) * 3);
                StsQueue.push(queue, add_idx);
            }
        }
    }

    if (idx[1] + 1 < table->lengths[1]) {
        idx_mod[0] = idx[0];idx_mod[1] = idx[1] + 1;idx_mod[2] = idx[2];
        if(dec_sync_table(idx_mod, sync)) {
            int *add_idx= malloc(sizeof(int) * 3);
            memcpy(add_idx, idx_mod, sizeof(int) * 3);
            StsQueue.push(queue, add_idx);
        }
        
        if (idx[2] + 1 < table->lengths[2]) {
            idx_mod[0] = idx[0];idx_mod[1] = idx[1] + 1;idx_mod[2] = idx[2] + 1;
            if(dec_sync_table(idx_mod, sync)) {
                int *add_idx= malloc(sizeof(int) * 3);
                memcpy(add_idx, idx_mod, sizeof(int) * 3);
                StsQueue.push(queue, add_idx);
            }
        }
    }

    if (idx[2] + 1 < table->lengths[2]) {
        idx_mod[0] = idx[0];idx_mod[1] = idx[1];idx_mod[2] = idx[2] + 1;
        if(dec_sync_table(idx_mod, sync)) {
            int *add_idx= malloc(sizeof(int) * 3);
            memcpy(add_idx, idx_mod, sizeof(int) * 3);
            StsQueue.push(queue, add_idx);
        }
    }
}

void *thread_job(void *job) {
    StsHeader *queue = ((job_t*)job)->queue;
    u_int8_t num_threads = ((job_t*)job)->num_threads;
    dp_table *table = ((job_t*)job)->table;
    sync_table *sync = ((job_t*)job)->sync;
    char **sequences = ((job_t*)job)->sequences;

    while(1) {
        if (!queue) {
            int *retval = malloc(sizeof(int));
            *retval = -1;
            return retval;
        }
        int *idxs = StsQueue.pop(queue);
        

        if (!idxs) {
            continue;
        }
        if (*idxs == -1) {
            free(idxs);
            int *retval = malloc(sizeof(int));
            *retval = -1;
            return retval;
        }
        short value = calculate_score(table, sequences, idxs);
        set_value_at_index(idxs, table, value);

        sync_with_table(idxs, sync, queue, table);

        if (idxs[0] + 1 == table->lengths[0] && idxs[1] + 1 == table->lengths[1] && idxs[2] + 1 == table->lengths[2]) {
            int *retval = malloc(sizeof(int));
            *retval = value;
            for (int i = 0; i < num_threads - 1; ++i) {
                int *to_push = malloc(sizeof(int));
                *to_push = -1;
                StsQueue.push(queue, to_push);
            }
            free(idxs);
            return retval;
        }
        free(idxs);
    }


    return NULL;
}

int edit_distance_multi(char **sequences, int k, int num_threads) {
    dp_table *table = alloc_dp_table(sequences, k);
    sync_table *sync = alloc_sync_table(sequences, k);
    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    StsHeader *queue = StsQueue.create();
    job_t *job = malloc(sizeof(job_t));
    job->queue = queue;
    job->num_threads = num_threads;
    job->table = table;
    job->sync = sync;
    job->sequences = sequences;

    int *origin = calloc(3, sizeof(int));
    StsQueue.push(queue, origin);


    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&(threads[i]), NULL, thread_job, job);
    }
    free(job);

    int retval = 0;
    for (int i = 0; i < num_threads; ++i) {
        int *thread_ret_val;
        pthread_join(threads[i], (void**)(&thread_ret_val));
        if (*thread_ret_val >= 0) {
            retval = *thread_ret_val;
        }
        free(thread_ret_val);
    }

    StsQueue.destroy(queue);
    free(threads);
    dealloc_dp_table(table);
    dealloc_sync_table(sync);
    return retval;
}