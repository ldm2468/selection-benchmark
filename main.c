#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h> /* for getopt */

#include "select.h"
#include "array.h"
#include "util.h"
#include "stats.h"

enum print_type {
    all = 0,
    times_only,
    calls_only
};

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

#define PIVOT_ALG_COUNT 7

static choose_pivot pivots[] = {
    first_pivot,
    random_pivot,
    med3_pivot,
    ninther_pivot,
    deterministic_pivot,
    deterministic2_pivot,
    deterministic3_pivot,
};

static const char* pivot_names[] = {
    "first",
    "random",
    "median_of_3",
    "ninther",
    "deterministic",
    "deterministic2",
    "deterministic3",
};

#define ITERATIONS 51

int main(int argc, char **argv) {
    int *arr = NULL;
    int n = 1000000, m = 0, r = 10, fixed_k = -1;
    enum array_type type = array_type_end;
    enum print_type print = all;
    int opt;

    /* parse arguments */
    while ((opt = getopt(argc, argv, "n:t:m:r:p:k:")) != -1) {
        switch (opt) {
        case 'n':
            n = (int) strtol(optarg, NULL, 0);
            if (n <= 0) {
                fprintf(stderr, "-n (array size) must be a positive integer\n");
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
                fprintf(stderr, "Invalid array type: valid types are 'ascending', 'shuffled', and 'random'");
                exit(1);
            }
            break;
        case 'm':
            m = (int) strtol(optarg, NULL, 0);
            if (m == 0) {
                fprintf(stderr, "-m (array modifier) must be a non-zero integer\n");
                exit(1);
            }
            break;
        case 'r':
            r = (int) strtol(optarg, NULL, 0);
            if (r <= 0) {
                fprintf(stderr, "-r (number of reps) must be a positive integer\n");
                exit(1);
            }
            break;
        case 'p':
            switch(optarg[0]) {
            case 'a':
                print = all;
                break;
            case 't':
                print = times_only;
                break;
            case 'c':
                print = calls_only;
                break;
            default:
                fprintf(stderr, "-p option (print type) must be one of 'a', 't', or 'c'\n");
                exit(1);
            }
            break;
        case 'k':
            fixed_k = (int) strtol(optarg, NULL, 0);
            if (fixed_k <= 0) {
                fprintf(stderr, "-k (element order) must be a positive integer\n");
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "Usage: %s [-n size] [-t type] [options]... \n", argv[0]);
            fprintf(stderr, "    -n: Size of array (default: 1000000)\n"
                            "    -t: Type of array (ascending/shuffled/random, default: shuffled)\n"
                            "        The type name may also be shortened to its first character (a/s/r)\n"
                            "    -m: A non-zero integer that affects the array in different ways depending on the type\n"
                            "        ascending/shuffled: the stride of the ascending (or shuffled) array (default: 1)\n"
                            "        random: the range of the random numbers in the array (default: n)\n"
                            "    -r: Number of times to repeat each run (default: 10)\n"
                            "    -p: What data to print. (a: all, t: times only, c: calls only)\n"
                            "    -k: The order of the element to find.\n"
                            "        If not specified, a range of values are uniformly selected from 0 to n - 1.\n");
            exit(1);
        }
    }

    if (type == array_type_end) {
        type = shuffled; /* default type */
    }

    if (m == 0) {
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

    if (fixed_k >= n) {
        fprintf(stderr, "-k (element order) must be < n\n");
        exit(1);
    }

    /* initialize array */
    arr = malloc(sizeof(int) * n);
    if (arr == NULL) {
        fprintf(stderr, "Array allocation failed.\n");
        exit(1);
    }

    fprintf(stderr, "Note: progress information will be written to stderr.\n"
                    "It is recommended to redirect stdout to a separate file, "
                    "otherwise the text will be intermixed and confusing.\n");

    /* print array info (csv) */
    if (print == all) {
        printf("array size,type,m\n");
        printf("%d,%s,%d\n", n, array_type_names[type], m);
    }

    float times[PIVOT_ALG_COUNT][ITERATIONS];
    float calls[PIVOT_ALG_COUNT][ITERATIONS];

    /* do the benchmarks */
    for (int i = 0; i < PIVOT_ALG_COUNT; i++) {
        for (int j = 0; j < ITERATIONS; j++) {
            int res;
            int target = fixed_k < 0 ? ((n - 1) * j) / (ITERATIONS - 1) : fixed_k;
            clock_t start, end;
            float time_sum = 0.f;
            float calls_sum = 0.f;

            for (int k = 0; k < r; k++) {
                int checksum;
                fprintf(stderr, "\r%s: %3d/%3d (%2d/%2d)", pivot_names[i], j, ITERATIONS - 1, k + 1, r);

                seed(fixed_k < 0 ? k + 1 : j + 1);

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

                checksum = xor_sum(arr, 0, n);

                reset_num_calls();

                start = clock();
                res = select(arr, 0, n, target, pivots[i]);
                end = clock();

                time_sum += (float) (end - start) * 1000.f / CLOCKS_PER_SEC;
                calls_sum += (float) get_num_calls();

                if (!check_select(arr, 0, n, target, res) || checksum != xor_sum(arr, 0, n)) {
                    fprintf(stderr, "Algorithm %s is incorrect!\n", pivot_names[i]);
                }
            }

            times[i][j] = time_sum / (float) r;
            calls[i][j] = calls_sum / (float) r;
        }
        fprintf(stderr, " OK\n");
    }

    /* print statistics */

    if (print == all || print == times_only) {
        if (print == all) {
            printf("\ntimes (ms)\n");
        }
        printf("k/L");
        for (int i = 0; i < PIVOT_ALG_COUNT; i++) {
            printf(",%s", pivot_names[i]);
        }
        printf("\n");
        for (int j = 0; j < ITERATIONS; j++) {
            printf("%g", fixed_k < 0 ? (float) j / (ITERATIONS - 1) : (float) fixed_k / (ITERATIONS - 1));
            for (int i = 0; i < PIVOT_ALG_COUNT; i++) {
                printf(",%.3f", times[i][j]);
            }
            printf("\n");
        }
    }

    if (print == all || print == calls_only) {
        if (print == all) {
            printf("\ncalls to partition()\n");
        }
        printf("k/L");
        for (int i = 0; i < PIVOT_ALG_COUNT; i++) {
            printf(",%s", pivot_names[i]);
        }
        printf("\n");
        for (int j = 0; j < ITERATIONS; j++) {
            printf("%g", fixed_k < 0 ? (float) j / (ITERATIONS - 1) : (float) fixed_k / (ITERATIONS - 1));
            for (int i = 0; i < PIVOT_ALG_COUNT; i++) {
                printf(",%.3f", calls[i][j]);
            }
            printf("\n");
        }
    }

    if (print == all) {
        printf("\npivot alg,time (ms),min,max,stddev,fn calls,min,max,stddev\n");
        for (int i = 0; i < PIVOT_ALG_COUNT; i++) {
            printf("%s,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f,%.3f\n",
                   pivot_names[i],
                   mean(times[i], ITERATIONS),
                   min(times[i], ITERATIONS),
                   max(times[i], ITERATIONS),
                   stddev(times[i], ITERATIONS),
                   mean(calls[i], ITERATIONS),
                   min(calls[i], ITERATIONS),
                   max(calls[i], ITERATIONS),
                   stddev(calls[i], ITERATIONS));
        }
    }
    free(arr);
    return 0;
}
