#include "select.h"
#include "util.h"
#include "array.h"

#include <math.h>

#define G 5
#define g ((G + 1) / 2)

static int med3(int a, int b, int c) {
    return a >= b ? b >= c ? b : a >= c ? c : a :
           c >= b ? b : a >= c ? a : c;
}

static double med3d(double a, double b, double c) {
    return a >= b ? b >= c ? b : a >= c ? c : a :
           c >= b ? b : a >= c ? a : c;
}

static int med3i(int a, int b, int c) {
    return a >= b ? b >= c ? 1 : a >= c ? 2 : 0 :
           c >= b ? 1 : a >= c ? 0 : 2;
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
        return med3i(arr[from], arr[from + 1], arr[from + 2]) + from;
    }

    insertion_sort(arr, from, to);
    return (to + from) / 2;
}

int first_pivot(int *arr, int from, int to, int k) {
    (void) to;
    (void) k;
    return arr[from];
}

int random_pivot(int *arr, int from, int to, int k) {
    (void) k;
    return arr[from + randint() % (to - from)]; /* note: introduces slight bias towards lower values */
}

int med3_pivot(int *arr, int from, int to, int k) {
    (void) k;
    return med3(arr[from], arr[(from + to) / 2], arr[to - 1]);
}

int ninther_pivot(int *arr, int from, int to, int k) {
    int len = to - from;
    (void) k;
    return med3(
        med3(arr[from + 0 * len / 8], arr[from + 3 * len / 8], arr[from + 6 * len / 8]),
        med3(arr[from + 1 * len / 8], arr[from + 4 * len / 8], arr[from + 7 * len / 8]),
        med3(arr[from + 2 * len / 8], arr[from + 5 * len / 8], arr[to - 1])
    );
}

int deterministic_pivot(int *arr, int from, int to, int k) {
    int j = from;
    (void) k;
    for (int i = from; i < to; i += G) {
        swap(&arr[mediani(arr, i, (i + G) > to ? to : i + G)], &arr[j++]);
    }
    int sel = (from + j) / 2;
    int pivot = select(arr, from, j, sel, deterministic_pivot, 0);

    return pivot;
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
    int pivot = select(arr, from, j, sel, deterministic_adaptive_pivot, 0);
    while (arr[++sel] == pivot && sel < j) { }
    int offset = to - j;
    for (int i = sel; i < j; i++) {
        swap(&arr[i], &arr[offset + i]);
    }

    return pivot;
}

int deterministic_strided_pivot(int *arr, int from, int to, int k) {
    if (to - from <= (G - 1) * (G - 1)) {
        insertion_sort(arr, from, to);
        return arr[k];
    }
    int stride = (to - from + G - 1) / G;
    for (int i = from; i < stride; i++) {
        insertion_sort_stride(arr, i, to, stride);
    }
    int offset = from + (to - from) * (g - 1) / G;
    int sel = stride / 2;
    int pivot = select(arr, offset, offset + stride, offset + sel, deterministic_strided_pivot, 0);

    return pivot;
}

int deterministic_adaptive_strided_pivot(int *arr, int from, int to, int k) {
    if (to - from <= (G - 1) * (G - 1)) {
        insertion_sort(arr, from, to);
        return arr[k];
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
    int pivot = select(arr, offset, offset + stride, offset + sel, deterministic_adaptive_strided_pivot, 0);

    return pivot;
}

#define MIN_GUESS_RATIO 2

static double introduce_bias(double d, double b) {
    return med3d(d + b, 0.5, d - b);
}

int guess_pivot(int *arr, int from, int to, int k) {
    int sq = (int) sqrt((double) (to - from));
    int len = MIN(sq, (to - from) / MIN_GUESS_RATIO);

    double N = to - from;
    double K = len;
    double T = k - from;
    double loc = ((K + 1.) / (N + 1.) * T - (N - K) / (N + 1.)) / (K - 1);
    double R = T / (N - 1.);
    int sel = (int) (introduce_bias(loc, 2. * sqrt((R) * (1 - R) / (K + 1))) * (K - 1) + 0.5);
    sel = med3(0, sel, len - 1);

    partial_shuffle(arr, from, from + len, to);
    int pivot = select(arr, from, from + len, from + sel, guess_pivot, 0);

    return pivot;
}

static int num_calls = 0;
static int bad_pivots = 0;

static void partition(int *arr, int from, int to, int pivot, int *mid, int *hi) {
    int i = from;
    *mid = from;
    *hi = to;
    /* skip already partitioned parts
    while (i < *hi && arr[i] < pivot) {
        i++;
        (*mid)++;
    }
    while (i < *hi && arr[*hi - 1] > pivot) {
        (*hi)--;
    } */
    while (i < *hi) {
        if (arr[i] < pivot) {
            swap(&arr[i++], &arr[(*mid)++]);
        } else if (arr[i] > pivot) {
            swap(&arr[i], &arr[--(*hi)]);
        } else {
            i++;
        }
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
    while (to - from > 1) {
        int pivot = strategy(arr, from, to, k);
        int mid, hi;
        partition(arr, from, to, pivot, &mid, &hi);
        if (record) {
            num_calls++;
            int left_len = mid - from;
            int right_len = to - hi;
            if ((left_len * 2 < right_len && k >= hi) || (left_len > right_len * 2 && k < mid)) {
                bad_pivots++;
            }
        }
        if (k >= hi) {
            from = hi;
        } else if (k < mid) {
            to = mid;
        } else {
            return arr[k];
        }
    }
    return arr[from];
}

int check_select(const int *arr, int from, int to, int k, int n) {
    int less = from, more = 0;
    for (int i = from; i < to; i++) {
        less += arr[i] < n;
        more += arr[i] > n;
    }
    return k >= less && k < to - more;
}
