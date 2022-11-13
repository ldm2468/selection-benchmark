#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <getopt.h> /* for getopt */

#include "select.h"
#include "array.h"
#include "util.h"
#include "stats.h"
#include "select_cpp.h"

enum print_type {
    all = 0,
    times_only,
    calls_only,
    ratios_only
};

enum array_type {
    ascending = 0,
    shuffled,
    uniform,
    rotated,
    nearly_sorted,
    array_type_end
};

static const char* array_type_chars = "asurn";

static const char* array_type_names[] = {
    "ascending",
    "shuffled",
    "uniform",
    "rotated",
    "nearly sorted",
};

#define PIVOT_ALG_COUNT 5
#define ALG_COUNT 6

static choose_pivot pivots[] = {
//    first_pivot,
    random_pivot,
//    med3_pivot,
    ninther_pivot,
    deterministic_pivot,
//    deterministic_adaptive_pivot,
//    deterministic_strided_pivot,
    deterministic_adaptive_strided_pivot,
    guess_pivot
};

static const char* alg_names[] = {
//    "First",
    "Random",
//    "Median of 3",
    "Ninther",
    "BFPRT",
//    "BFPRTA",
//    "BFPRT+",
    "BFPRTA+",
    "Guess",
    "libstdc++",
};

#define DEFAULT_ITERATIONS 51

static int parse_int_arg(const char *err_msg, int min) {
    int n = (int) strtol(optarg, NULL, 0);
    if (n < min) {
        fprintf(stderr, "%s\n", err_msg);
        exit(1);
    }
    return n;
}

static int do_select(int *arr, int size, int k, int alg, int record) {
    if (alg < PIVOT_ALG_COUNT) {
        return select(arr, 0, size, k, pivots[alg], record);
    } else {
        return select_cpp(arr, 0, size, k);
    }
}

static void print_stats(int alg_mask, int fixed_k, int iterations, int print, int n, float **arr, const char *name) {
    if (print == all) {
        printf("\n%s\n", name);
    }
    printf("k/L");
    for (int i = 0; i < ALG_COUNT; i++) {
        if ((alg_mask & (1 << i)) == 0) {
            continue;
        }
        printf(",%s", alg_names[i]);
    }
    printf("\n");
    for (int j = 0; j < iterations; j++) {
        printf("%g", fixed_k < 0 ? (float) j / (iterations - 1) : (float) fixed_k / n);
        for (int i = 0; i < ALG_COUNT; i++) {
            if ((alg_mask & (1 << i)) == 0) {
                continue;
            }
            printf(",%.3f", arr[i][j]);
        }
        printf("\n");
    }
}

int main(int argc, char **argv) {
    int *arr = NULL;
    int n = 1000000, m = 0, r = 10, fixed_k = -1;
    enum array_type type = array_type_end;
    enum print_type print = all;
    int iterations = DEFAULT_ITERATIONS;
    int alg_mask = 0xFFFF;
    int opt;

    /* parse arguments */
    while ((opt = getopt(argc, argv, "n:t:m:r:p:k:i:a:")) != -1) {
        switch (opt) {
        case 'n':
            n = parse_int_arg("-n (array size) must be a positive integer", 1);
            break;
        case 't':
            for (int i = 0; i < array_type_end; i++) {
                if (optarg[0] == array_type_chars[i]) {
                    type = i;
                    break;
                }
            }
            if (type == array_type_end) {
                fprintf(stderr, "Invalid array type: valid types are\n"
                                "'ascending', 'shuffled', 'uniform', 'rotated', and 'nearly_sorted'");
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
            r = parse_int_arg("-r (number of reps) must be a positive integer", 1);
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
            case 'r':
                print = ratios_only;
                break;
            default:
                fprintf(stderr, "-p option (print type) must be one of 'a', 't', 'c', or 'r'\n");
                exit(1);
            }
            break;
        case 'k':
            fixed_k = parse_int_arg("-k (element order) must be a non-negative integer", 0);
            break;
        case 'i':
            iterations = parse_int_arg("-i (iterations) must be a positive integer", 1);
            break;
        case 'a':
            alg_mask = (int) strtol(optarg, NULL, 2);
            if ((alg_mask & ((1 << ALG_COUNT) - 1)) == 0) {
                fprintf(stderr, "At least one algorithm must be enabled");
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "Usage: %s [-n size] [-t type] [options]... \n", argv[0]);
            fprintf(stderr, "    -n: Size of array (default: 1000000)\n"
                            "    -t: Type of array (ascending/shuffled/uniform/rotated/nearly_sorted, default: shuffled)\n"
                            "        The type name may also be shortened to its first character (a/s/r)\n"
                            "    -m: A non-zero integer that affects the array in different ways depending on the type\n"
                            "        ascending/shuffled: the stride of the ascending (or shuffled) array (default: 1)\n"
                            "        random: the range of the random numbers in the array (default: n)\n"
                            "    -r: Number of times to repeat each run (default: 10)\n"
                            "    -p: What data to print. (a: all, t: times only, c: calls only, r: ratios only)\n"
                            "    -k: The order of the element to find.\n"
                            "        If not specified, a range of values are uniformly selected from 0 to n - 1.\n"
                            "    -i: The number of iterations (number of columns output, default: %d)\n"
                            "    -a: A binary mask of algorithms to run. (ex. 100101)\n", DEFAULT_ITERATIONS);
            exit(1);
        }
    }

    if (type == array_type_end) {
        type = shuffled; /* default type */
    }

    if (m == 0) {
        switch (type) {
        case ascending: case shuffled: case rotated:
            m = 1;
            break;
        case uniform:
            m = n;
            break;
        case nearly_sorted:
            m = n / 10;
        default:
            break;
        }
    }

    if (type == uniform && m < 0) {
        fprintf(stderr, "-m must be > 0 for array type = uniform\n");
        exit(1);
    }

    if (type == rotated && (m < 0 || m >= n)) {
        fprintf(stderr, "-m must be in [1..n) for array type = rotated\n");
        exit(1);
    }

    if (type == nearly_sorted && (m < 0 || m >= n)) {
        fprintf(stderr, "-m must be in [1..n) for array type = nearly_sorted\n");
        exit(1);
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

    float *times[ALG_COUNT];
    float *calls[ALG_COUNT];
    float *ratios[ALG_COUNT];

    for (int i = 0; i < ALG_COUNT; i++) {
        times[i] = malloc(sizeof(float) * iterations);
        calls[i] = malloc(sizeof(float) * iterations);
        ratios[i] = malloc(sizeof(float) * iterations);
    }

    /* do the benchmarks */
    for (int i = 0; i < ALG_COUNT; i++) {
        if ((alg_mask & (1 << i)) == 0) {
            continue;
        }
        for (int j = 0; j < iterations; j++) {
            int res;
            int target = fixed_k < 0 ? ((n - 1) * j) / (iterations - 1) : fixed_k;
            clock_t start, end;
            float time_sum = 0.f;
            float time_max = 0.f;
            float time_min = 1.f / 0.f; /* infinity */
            float calls_sum = 0.f;
            float bad_pivot_sum = 0.f;

            for (int k = 0; k < r; k++) {
                float curr_time;

                int checksum;
                fprintf(stderr, "\r%s: %3d/%3d (%2d/%2d)", alg_names[i], j, iterations - 1, k + 1, r);

                seed(fixed_k < 0 ? k + 1 : j + 1);

                switch (type) {
                case ascending:
                    fill_sequence(arr, 0, n, 0, m, n);
                    break;
                case shuffled:
                    fill_sequence(arr, 0, n, 0, m, n);
                    shuffle(arr, 0, n);
                    break;
                case uniform:
                    fill_random(arr, 0, n, 0, m);
                    break;
                case rotated:
                    fill_sequence(arr, 0, n - m, m, 1, n);
                    fill_sequence(arr, n - m, n, 0, 1, n);
                    break;
                case nearly_sorted:
                    fill_sequence(arr, 0, n, 0, 1, n);
                    swap_random(arr, 0, n, m);
                    break;
                default:
                    break;
                }

                checksum = xor_sum(arr, 0, n);

                reset_num_calls();

                start = clock();
                res = do_select(arr, n, target, i, print != times_only);
                end = clock();

                curr_time = (float) (end - start) * 1000.f / CLOCKS_PER_SEC;
                time_sum += curr_time;
                calls_sum += (float) get_num_calls();
                bad_pivot_sum += (float) get_bad_pivot_count();

                if (curr_time < time_min) {
                    time_min = curr_time;
                }
                if (curr_time > time_max) {
                    time_max = curr_time;
                }

                if (!check_select(arr, 0, n, target, res) || checksum != xor_sum(arr, 0, n)) {
                    fprintf(stderr, "Algorithm %s is incorrect!\n", alg_names[i]);
                }
            }

            /* eliminate outliers */
            times[i][j] = r < 3 ? (time_sum / (float) r) : (time_sum - time_min - time_max) / (float) (r - 2);
            calls[i][j] = calls_sum / (float) r;
            ratios[i][j] = bad_pivot_sum / (calls_sum + 1E-9f); // prevent division by zero
        }
        fprintf(stderr, " OK\n");
    }

    /* print statistics */

    if (print == all || print == times_only) {
        print_stats(alg_mask, fixed_k, iterations, print, n, times, "times (ms)");
    }

    if (print == all || print == calls_only) {
        print_stats(alg_mask, fixed_k, iterations, print, n, calls, "calls to partition()");
    }

    if (print == all || print == ratios_only) {
        print_stats(alg_mask, fixed_k, iterations, print, n, ratios, "ratio of bad pivot choices");
    }

    if (print == all) {
        printf("\npivot alg,time (ms),min,max,stddev,fn calls,min,max,stddev,bad pivot ratio,min,max,stddev\n");
        for (int i = 0; i < ALG_COUNT; i++) {
            if ((alg_mask & (1 << i)) == 0) {
                continue;
            }
            printf("%9s,%9.3f,%9.3f,%9.3f,%9.3f,%6.3f,%6.3f,%6.3f,%6.3f,%5.3f,%5.3f,%5.3f,%5.3f\n",
                   alg_names[i],
                   mean(times[i], iterations),
                   min(times[i], iterations),
                   max(times[i], iterations),
                   stddev(times[i], iterations),
                   mean(calls[i], iterations),
                   min(calls[i], iterations),
                   max(calls[i], iterations),
                   stddev(calls[i], iterations),
                   mean(ratios[i], iterations),
                   min(ratios[i], iterations),
                   max(ratios[i], iterations),
                   stddev(ratios[i], iterations));
        }
    }
    free(arr);
    for (int i = 0; i < ALG_COUNT; i++) {
        free(times[i]);
        free(calls[i]);
        free(ratios[i]);
    }
    return 0;
}
