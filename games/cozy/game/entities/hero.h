#ifndef HERO_H
#define HERO_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"

#define HERO_ROW 0
#define HERO_COL 2

typedef enum hero_movement_t
{
    MOVE_UP = 0x1,
    MOVE_DOWN = 0x2,
    MOVE_LEFT = 0x4,
    MOVE_RIGHT = 0x8
} hero_movement_t;

typedef struct hero_t
{
    vec2 position;
    vec2 velocity;
    int collision_index;
    float speed;
} hero_t;

void hero_update(aabb_tree_t *aabb_tree, hero_t *hero, hero_movement_t movement);

#endif // HERO_H
