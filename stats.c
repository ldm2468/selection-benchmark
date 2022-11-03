#include "stats.h"

#include <math.h>

float mean(const float values[], int count) {
    float sum = 0;
    for (int i = 0; i < count; i++) {
        sum += values[i];
    }
    return sum / (float) count;
}

float stddev(const float values[], int count) {
    /* two-step algorithm for better numerical stability */
    float m = mean(values, count);
    float sqsum = 0;
    for (int i = 0; i < count; i++) {
        sqsum += (values[i] - m) * (values[i] - m);
    }
    return sqrtf(sqsum / (float) (count - 1)); /* sample variance */
}
