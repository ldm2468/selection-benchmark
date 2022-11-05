#ifndef DETERMINISTIC_SELECT_H
#define DETERMINISTIC_SELECT_H

typedef int (*choose_pivot)(int *arr, int from, int to, int k);
int first_pivot(int *arr, int from, int to, int k);
int random_pivot(int *arr, int from, int to, int k);
int med3_pivot(int *arr, int from, int to, int k);
int ninther_pivot(int *arr, int from, int to, int k);
int deterministic_pivot(int *arr, int from, int to, int k);
int deterministic_adaptive_pivot(int *arr, int from, int to, int k);
int deterministic_strided_pivot(int *arr, int from, int to, int k);
int deterministic_adaptive_strided_pivot(int *arr, int from, int to, int k);
int guess_pivot(int *arr, int from, int to, int k);

int get_num_calls(void);
void reset_num_calls(void);

int select(int *arr, int from, int to, int k, choose_pivot strategy, int record);

int check_select(const int *arr, int from, int to, int k, int n);

#endif /* DETERMINISTIC_SELECT_H */
