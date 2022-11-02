#ifndef DETERMINISTIC_SELECT_ARRAY_H
#define DETERMINISTIC_SELECT_ARRAY_H

void fill_random(int *arr, int from, int to, int min, int max);
void fill_sequence(int *arr, int from, int to, int first, int step);
void shuffle(int *arr, int from, int to);
void insertion_sort(int *arr, int from, int to);
void selection_sort(int *arr, int from, int to);
void print_arr(int *arr, int from, int to);

#endif /* DETERMINISTIC_SELECT_ARRAY_H */
