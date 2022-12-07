#include "select.h"
#include "util.h"
#include "array.h"

#include <math.h>

#define G 5
#define g ((G + 1) / 2)
#define INSERTION_SORT_THRESHOLD 32

static int med3(int a, int b, int c) {
    return a >= b ? b >= c ? b : a >= c ? c : a :
           c >= b ? b : a >= c ? a : c;
}

static double med3d(double a, double b, double c) {
    return a >= b ? b >= c ? b : a >= c ? c : a :
           c >= b ? b : a >= c ? a : c;
}

static int med3i(const int *arr, int i, int j, int k) {
    int a = arr[i], b = arr[j], c = arr[k];
    return a >= b ? b >= c ? j : a >= c ? k : i :
           c >= b ? j : a >= c ? i : k;
}

static void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

/* If the range has even elements, returns either the n/2'th or n/2 + 1'th element. */
static int mediani(int *arr, int from, int to) {
    if (to - from < 3) {
        return from;
    } else if (to - from < 5) {
        return med3i(arr, from, from + 1, from + 2);
    }

    insertion_sort(arr, from, to);
    return (to + from) / 2;
}

int first_pivot(int *arr, int from, int to, int k) {
    (void) to;
    (void) k;
    (void) arr;
    return from;
}

int random_pivot(int *arr, int from, int to, int k) {
    (void) k;
    (void) arr;
    return from + randint() % (to - from); /* note: introduces slight bias towards lower values */
}

int med3_pivot(int *arr, int from, int to, int k) {
    (void) k;
    return med3i(arr, from, (from + to) / 2, to - 1);
}

int ninther_pivot(int *arr, int from, int to, int k) {
    int len = to - from;
    (void) k;
    return med3i(arr,
        med3i(arr, from + 0 * len / 8, from + 3 * len / 8, from + 6 * len / 8),
        med3i(arr, from + 1 * len / 8, from + 4 * len / 8, from + 7 * len / 8),
        med3i(arr, from + 2 * len / 8, from + 5 * len / 8, to - 1)
    );
}

int deterministic_pivot(int *arr, int from, int to, int k) {
    int j = from;
    (void) k;
    for (int i = from; i < to; i += G) {
        swap(&arr[mediani(arr, i, (i + G) > to ? to : i + G)], &arr[j++]);
    }
    int sel = (from + j) / 2;
    select(arr, from, j, sel, deterministic_pivot, 0);

    return sel;
}

int deterministic_adaptive_pivot(int *arr, int from, int to, int k) {
    int j = from;
    for (int i = from; i < to; i += G) {
        swap(&arr[mediani(arr, i, (i + G) > to ? to : i + G)], &arr[j++]);
    }
    int sel = med3(
        (j + from) / 2,
        (k - from) / g + from,
        j - 1 - (to - k) / g
    );
    select(arr, from, j, sel, deterministic_adaptive_pivot, 0);

    return sel;
}

int deterministic_strided_pivot(int *arr, int from, int to, int k) {
    if (to - from <= (G - 1) * (G - 1)) {
        insertion_sort(arr, from, to);
        return k;
    }
    int stride = (to - from + G - 1) / G;
    for (int i = from; i < stride; i++) {
        insertion_sort_stride(arr, i, to, stride);
    }
    int offset = from + (to - from) * (g - 1) / G;
    int sel = stride / 2;
    select(arr, offset, offset + stride, offset + sel, deterministic_strided_pivot, 0);

    return offset + sel;
}

int deterministic_adaptive_strided_pivot(int *arr, int from, int to, int k) {
    if (to - from <= (G - 1) * (G - 1)) {
        insertion_sort(arr, from, to);
        return k;
    }
    int stride = (to - from + G - 1) / G;
    for (int i = from; i < stride; i++) {
        insertion_sort_stride(arr, i, to, stride);
    }
    int offset = from + (to - from) * (g - 1) / G;
    int sel = med3(
        stride / 2,
        (k - from) / g,
        stride - 1 - (to - k) / g
    );
    select(arr, offset, offset + stride, offset + sel, deterministic_adaptive_strided_pivot, 0);

    return offset + sel;
}

#define MIN_GUESS_RATIO 2

static double introduce_bias(double d, double b) {
    return med3d(d + b, 0.5, d - b);
}

int sampling_pivot(int *arr, int from, int to, int k) {
    if (to - from <= INSERTION_SORT_THRESHOLD) {
        return random_pivot(arr, from, to, k);
    }

    int sq = (int) sqrt((double) (to - from));
    int len = MIN(sq, (to - from) / MIN_GUESS_RATIO);

    double N = to - from;
    double n = len;
    double T = k - from;
    double loc = ((n + 1.) / (N + 1.) * (T + 1.) - 1.) / (n - 1.);
    double R = (T + 1.) / (N + 1.);
    double sigma = sqrt(R * (1. - R) * (N - n) * (N + 1.) / (n + 2.)) / (n + 1.) / (n - 1.);
    int sel = (int) (introduce_bias(loc, 2. * sigma) * (n - 1) + 0.5);
    sel = med3(0, sel, len - 1);

    partial_shuffle(arr, from, from + len, to);
    select(arr, from, from + len, from + sel, sampling_pivot, 0);

    return from + sel;
}

static int num_calls = 0;
static int bad_pivots = 0;

static int partition(int *arr, int from, int to, int pivot) {
    /* basic hoare partition that also divides same values evenly */
    /* pivot must not be at the last element!! */
    int i = from - 1, j = to;
    while (1) {
        do {
            ++i;
        } while (arr[i] < pivot);
        do {
            --j;
        } while (arr[j] > pivot);
        if (i >= j) {
            return j + 1;
        }
        swap(&arr[i], &arr[j]);
    }
}

int get_num_calls(void) {
    return num_calls;
}

int get_bad_pivot_count(void) {
    return bad_pivots;
}

void reset_num_calls(void) {
    num_calls = 0;
    bad_pivots = 0;
}

int select(int *arr, int from, int to, int k, choose_pivot strategy, int record) {
    while (to - from > INSERTION_SORT_THRESHOLD) {
        int pivot_loc = strategy(arr, from, to, k);
        swap(&arr[from], &arr[pivot_loc]); /* prevent pivot element from being at the end */
        int p = partition(arr, from, to, arr[from]);
        if (record) {
            num_calls++;
            int left_len = p - from;
            int right_len = to - p;
            if ((left_len * 2 < right_len && k >= p) || (left_len > right_len * 2 && k < p)) {
                bad_pivots++;
            }
        }
        if (k >= p) {
            from = p;
        } else {
            to = p;
        }
    }
    insertion_sort(arr, from, to);
    return arr[k];
}

int check_select(const int *arr, int from, int to, int k, int n) {
    int less = from, more = 0;
    for (int i = from; i < to; i++) {
        less += arr[i] < n;
        more += arr[i] > n;
    }
    return k >= less && k < to - more;
}
