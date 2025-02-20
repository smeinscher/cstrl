#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"

typedef struct projectiles_t
{
    size_t count;
    size_t capacity;
    vec3 *position;
    bool *active;
    da_int free_ids;
} projectiles_t;

bool projectiles_init(projectiles_t *projectiles);

int projectiles_add(projectiles_t *projectiles, int player_id, vec3 position);

void projectiles_remove(projectiles_t *projectiles, int projectile_id);

void projectiles_free(projectiles_t *projectiles);

#endif // PROJECTILE_H
