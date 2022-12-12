#ifndef DETERMINISTIC_SELECT_ARRAY_H
#define DETERMINISTIC_SELECT_ARRAY_H

void fill_random(int *arr, int from, int to, int min, int max);
void fill_sequence(int *arr, int from, int to, int first, int step, int modulo);
/* fills array with 1, 2, 2, 3, 3, 3, 4, 4, 4, 4, ... */
void fill_pyramid(int *arr, int from, int to, int first);
void shuffle(int *arr, int from, int to);
void partial_shuffle(int *arr, int from, int to, int sample_last);
void swap_random(int *arr, int from, int to, int reps);
void insertion_sort(int *arr, int from, int to);
void insertion_sort_stride(int *arr, int from, int to, int stride);
void selection_sort(int *arr, int from, int to);
void print_arr(const int *arr, int from, int to);
int xor_sum(const int *arr, int from, int to);

#endif /* DETERMINISTIC_SELECT_ARRAY_H */
