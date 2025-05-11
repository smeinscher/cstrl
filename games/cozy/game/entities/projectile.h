#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_util.h"
#include "guy.h"

#define PROJECTILE_TRAVEL_DISTANCE 250
#define PROJECTILE_SIZE 8
#define PROJECTILE_SPEED 10

typedef struct projectiles_t
{
    da_int freed_ids;
    vec3 *color;
    vec2 *position;
    vec2 *velocity;
    float *distance;
    bool *active;
    size_t count;
    size_t capacity;
} projectiles_t;

bool projectiles_init(projectiles_t *projectiles);

int projectiles_add(projectiles_t *projectiles, vec2 position, vec2 velocity, vec3 color);

void projectiles_update(projectiles_t *projectiles, aabb_tree_t *aabb_tree, guys_t *guys);

void projectiles_clean(projectiles_t *projectiles);

#endif // PROJECTILE_H
