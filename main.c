#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h> /* for getopt */

#include "select.h"
#include "array.h"
#include "util.h"
#include "stats.h"

enum array_type {
    ascending = 0,
    shuffled,
    random,
    array_type_end
};

static const char* array_type_chars = "asr";

static const char* array_type_names[] = {
    "ascending",
    "shuffled",
    "random"
};

#define PIVOT_ALG_COUNT 5

static choose_pivot pivots[] = {
    first_pivot,
    random_pivot,
    med3_pivot,
    deterministic_pivot,
    deterministic2_pivot
};

static const char* pivot_names[] = {
    "first",
    "random",
    "median_of_3",
    "deterministic",
    "deterministic2"
};

#define ITERATIONS 100

int main(int argc, char **argv) {
    int *arr = NULL;
    int n = 1000000, m = -1;
    enum array_type type = array_type_end;
    int opt;

    /* parse arguments */
    while ((opt = getopt(argc, argv, "n:t:m:")) != -1) {
        switch (opt) {
        case 'n':
            n = atoi(optarg);
            if (n <= 0) {
                printf("-n (array size) must be a positive integer\n");
                exit(1);
            }
            break;
        case 't':
            for (int i = 0; i < array_type_end; i++) {
                if (optarg[0] == array_type_chars[i]) {
                    type = i;
                    break;
                }
            }
            if (type == array_type_end) {
                printf("Invalid array type: valid types are 'ascending', 'shuffled', and 'random'");
                exit(1);
            }
            break;
        case 'm':
            m = atoi(optarg);
            if (m <= 0) {
                printf("-m (array modifier) must be a positive integer\n");
                exit(1);
            }
            break;
        default:
            printf("Usage: %s [-n number] [-t type] [-m number] \n", argv[0]);
            printf("-n: Size of array (default: 1000000)\n"
                   "-t: Type of array (ascending/shuffled/random, default: shuffled)\n"
                   "    The type name may also be shortened to its first character (a/s/r)\n"
                   "-m: A positive integer that affects the array in different ways depending on the type\n"
                   "    ascending/shuffled: the stride of the ascending (or shuffled) array (default: 1)\n"
                   "    random: the range of the random numbers in the array (default: n)\n");
            exit(1);
        }
    }

    if (type == array_type_end) {
        type = shuffled; /* default type */
    }

    if (m < 0) {
        switch (type) {
        case ascending: case shuffled:
            m = 1;
            break;
        case random:
            m = n;
            break;
        default:
            break;
        }
    }

    /* initialize array */
    arr = malloc(sizeof(int) * n);
    if (arr == NULL) {
        printf("Array allocation failed.\n");
        exit(1);
    }

    /* print array info (csv) */
    printf("array size,type,m\n");
    printf("%d,%s,%d\n", n, array_type_names[type], m);

    /* print statistics */
    printf("pivot alg,time (ms),stddev,fn calls,stddev\n");
    for (int i = 0; i < PIVOT_ALG_COUNT; i++) {
        float times[ITERATIONS];
        float calls[ITERATIONS];

        for (int j = 0; j < ITERATIONS; j++) {
            int res;
            int target = (n * j) / ITERATIONS;
            clock_t start, end;

            seed(1);

            switch (type) {
            case ascending:
                fill_sequence(arr, 0, n, 0, m);
                break;
            case shuffled:
                fill_sequence(arr, 0, n, 0, m);
                shuffle(arr, 0, n);
                break;
            case random:
                fill_random(arr, 0, n, 0, m);
                break;
            default:
                break;
            }

            reset_num_calls();

            start = clock();
            res = select(arr, 0, n, target, pivots[i]);
            end = clock();

            if (!check_select(arr, 0, n, target, res)) {
                fprintf(stderr, "Algorithm %s is invalid!\n", pivot_names[i]);
            }

            times[j] = (float) (end - start) * 1000.f / CLOCKS_PER_SEC;
            calls[j] = (float) get_num_calls();
        }

        printf("%s,%.3f,%.3f,%.3f,%.3f\n",
               pivot_names[i],
               mean(times, ITERATIONS),
               stddev(times, ITERATIONS),
               mean(calls, ITERATIONS),
               stddev(calls, ITERATIONS));
    }
    return 0;
}
