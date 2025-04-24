#ifndef CUP_H
#define CUP_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"
#include <stdbool.h>

#define TOTAL_CUP_COUNT 20
#define CUP_SIZE 25.0f

typedef struct cups_t
{
    vec2 position[TOTAL_CUP_COUNT];
    bool active[TOTAL_CUP_COUNT];
    da_int freed;
} cups_t;

bool cups_init(cups_t *cups);

int cups_shot_test(cups_t *cups, vec2 hit_position);

void cups_free(cups_t *cups);

#endif // CUP_H
