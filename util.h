#ifndef DETERMINISTIC_SELECT_UTIL_H
#define DETERMINISTIC_SELECT_UTIL_H

#include <stdint.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

uint32_t randint(void);

#endif /* DETERMINISTIC_SELECT_UTIL_H */
