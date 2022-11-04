#include "util.h"

/* xoshiro128++ by David Blackman and Sebastiano Vigna (2019, public domain)
 * https://prng.di.unimi.it/xoshiro128plusplus.c                             */
static uint32_t rotl(const uint32_t x, int k) {
    return (x << k) | (x >> (32 - k));
}

static uint32_t s[4] = {1, 1, 1, 1};

void seed(uint32_t n) {
    s[0] = n;
    s[1] = s[2] = s[3] = 1;
    for (int i = 0; i < 100; i++) {
        randint();
    }
}

uint32_t randint(void) {
    const uint32_t result = rotl(s[0] + s[3], 7) + s[0];

    const uint32_t t = s[1] << 9;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 11);

    return result;
}
