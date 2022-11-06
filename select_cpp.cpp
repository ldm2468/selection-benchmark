#include "select_cpp.h"

#include <algorithm>

int select_cpp(int *arr, int from, int to, int k) {
    std::nth_element(arr + from, arr + k, arr + to);
    return arr[k];
}
