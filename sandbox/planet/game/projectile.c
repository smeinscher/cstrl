#include "projectile.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>

bool projectiles_init(projectiles_t *projectiles)
{
    projectiles->count = 0;
    projectiles->capacity = 1;
    projectiles->position = NULL;
    projectiles->active = NULL;

    cstrl_da_int_init(&projectiles->free_ids, 1);

    projectiles->start_position = malloc(sizeof(vec3));
    if (!projectiles->start_position)
    {
        printf("Error allocating memory for projectile start_position\n");
        projectiles_free(projectiles);
        return false;
    }
    projectiles->position = malloc(sizeof(vec3));
    if (!projectiles->position)
    {
        printf("Error allocating memory for projectile position\n");
        projectiles_free(projectiles);
        return false;
    }
    projectiles->active = malloc(sizeof(bool));
    if (!projectiles->active)
    {
        printf("Error allocating memory for projectile active\n");
        projectiles_free(projectiles);
        return false;
    }

    return true;
}

int projectiles_add(projectiles_t *projectiles, int player_id, vec3 position)
{
    int new_id = 0;
    if (projectiles->free_ids.size == 0)
    {
        new_id = projectiles->count++;
        if (projectiles->count > projectiles->capacity)
        {
            projectiles->capacity *= 2;
            if (!cstrl_realloc_vec3(&projectiles->start_position, projectiles->capacity))
            {
                printf("Error allocating projectile start position\n");
            }
            if (!cstrl_realloc_vec3(&projectiles->position, projectiles->capacity))
            {
                printf("Error allocating projectile position\n");
            }
            if (!cstrl_realloc_bool(&projectiles->active, projectiles->capacity))
            {
                printf("Error allocating unit active status\n");
                return -1;
            }
        }
    }
    else
    {
        new_id = cstrl_da_int_pop_back(&projectiles->free_ids);
    }
    projectiles->start_position[new_id] = position;
    projectiles->position[new_id] = position;
    projectiles->active[new_id] = true;

    return new_id;
}

void projectiles_remove(projectiles_t *projectiles, int projectile_id)
{
}

void projectiles_free(projectiles_t *projectiles)
{
}
