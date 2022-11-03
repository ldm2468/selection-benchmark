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

float min(const float values[], int count) {
    int m = 0;
    for (int i = 1; i < count; i++) {
        if (values[i] < values[m]) {
            m = i;
        }
    }
    return values[m];
}

float max(const float values[], int count) {
    int m = 0;
    for (int i = 1; i < count; i++) {
        if (values[i] > values[m]) {
            m = i;
        }
    }
    return values[m];
}
