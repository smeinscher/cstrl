#ifndef GUY_H
#define GUY_H

#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_util.h"
#include <stdbool.h>

#define GUY_ANIMATION_TOTAL_FRAMES 5
#define GUY_ANIMATION_FRAME_TIME 0.1
#define GUY_SCALE 0.0025
#define GUY_SIZE (32 * GUY_SCALE)
#define GUY_TOTAL_TYPES 8
#define GUY_TOTAL_BODY_TYPES 2
#define GUY_TOTAL_ROWS 10
#define GUY_TOTAL_COLS 5

typedef struct guys_t
{
    da_int freed_ids;
    vec3 *color;
    vec3 *position;
    vec3 *velocity;
    double *animation_last_frame;
    int *animation_frame;
    int *type;
    int *collision_index;
    bool *animate;
    bool *active;
    size_t count;
    size_t capacity;
} guys_t;

bool guys_init(guys_t *guys);

int guys_add(guys_t *guys, aabb_tree_t *aabb_tree, vec3 position, vec3 color);

void guys_update(guys_t *guys, aabb_tree_t *aabb_tree);

void guys_clean(guys_t *guys, aabb_tree_t *aabb_tree);

#endif // GUY_H
