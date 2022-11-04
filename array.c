#include "array.h"

#include <stdio.h>
#include "util.h"

static void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void fill_random(int *arr, int from, int to, int min, int max) {
    for (int i = from; i < to; i++) {
        arr[i] = min + (int) (randint() % (max - min));
    }
}

void fill_sequence(int *arr, int from, int to, int first, int step) {
    for (int i = from; i < to; i++) {
        arr[i] = first;
        first += step;
    }
}

void shuffle(int *arr, int from, int to) {
    /* fisher-yates shuffle */
    for (int i = to - 1; i > from; i--) {
        int r = (int) (randint() % (i - from + 1));
        swap(&arr[i], &arr[r + from]);
    }
}

void insertion_sort(int *arr, int from, int to) {
    for (int i = from + 1; i < to; i++) {
        int tmp = arr[i], j;
        for (j = i - 1; j >= from && arr[j] > tmp; j--) {
            arr[j + 1] = arr[j];
        }
        arr[j + 1] = tmp;
    }
}

void insertion_sort_stride(int *arr, int from, int to, int stride) {
    for (int i = from + stride; i < to; i += stride) {
        int tmp = arr[i], j;
        for (j = i - stride; j >= from && arr[j] > tmp; j -= stride) {
            arr[j + stride] = arr[j];
        }
        arr[j + stride] = tmp;
    }
}

void selection_sort(int *arr, int from, int to) {
    for (int i = from; i < to - 1; i++) {
        int minj = i;
        for (int j = i + 1; j < to; j++) {
            if (arr[j] < arr[minj]) {
                minj = j;
            }
        }
        swap(&arr[i], &arr[minj]);
    }
}

void print_arr(const int *arr, int from, int to) {
    for (int i = from; i < to; i++) {
        printf("%d", arr[i]);
        printf(i == to - 1 ? "\n" : " ");
    }
}

int xor_sum(const int *arr, int from, int to) {
    int xor = 0;
    for (int i = from; i < to; i++) {
        xor ^= arr[i];
    }
    return xor;
}
