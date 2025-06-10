#ifndef BUILDING_H
#define BUILDING_H

#include "cstrl/cstrl_util.h"
#define BUILDING_BLOCK_SIZE 288

typedef struct buildings_t
{
    da_int freed_ids;
    vec2 *position;
    int *type;
    int *collision_index;
    size_t count;
    size_t capacity;
} buildings_t;

bool buildings_init(buildings_t *buildings);

int buildings_add(buildings_t *buildings, vec2 position);

void buildings_clean(buildings_t *buildings);

#endif // BUILDING_H
