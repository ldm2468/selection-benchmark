#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "select.h"
#include "array.h"

int main(void) {
    int *arr = malloc(sizeof(int) * 100000000);
    int res;
    clock_t start, end;

    fill_random(arr, 0, 100000000, 0, 10000);

    start = clock();
    res = select(arr, 0, 100000000, 22222222, deterministic2_pivot);
    end = clock();
    printf("%d\n", res);
    printf("num calls: %d\n", get_num_calls());
    printf("time: %.3f ms\n", (float) (end - start) * 1000.f / CLOCKS_PER_SEC);
    return 0;
}
