#ifndef BUTTERFLY_H
#define BUTTERFLY_H

#include "cstrl/cstrl_util.h"
#include <stdbool.h>

#define BUTTERFLY_ANIMATION_TOTAL_FRAMES 5
#define BUTTERFLY_ANIMATION_TOTAL_TYPES 6
#define BUTTERFLY_ANIMATION_FRAME_TIME 0.1
#define BUTTERFLY_SIZE 32

typedef struct butterflies_t
{
    da_int freed_ids;
    vec2 *position;
    vec2 *velocity;
    int *animation_frame;
    double *animation_last_frame;
    bool *active;
    size_t count;
    size_t capacity;
} butterflies_t;

bool butterflies_init(butterflies_t *butterflies);

int butterflies_add(butterflies_t *butterflies, vec2 position);

void butterflies_update(butterflies_t *butterflies);

#endif // BUTTERFLY_H
