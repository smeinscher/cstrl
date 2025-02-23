#include "projectile.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>

bool projectiles_init(projectiles_t *projectiles)
{
    projectiles->count = 0;
    projectiles->capacity = 1;
    projectiles->direction = NULL;
    projectiles->position = NULL;
    projectiles->active = NULL;

    cstrl_da_int_init(&projectiles->free_ids, 1);

    projectiles->direction = malloc(sizeof(vec3));
    if (!projectiles->direction)
    {
        printf("Error allocating memory for projectile direction\n");
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
    projectiles->uvs = malloc(sizeof(vec4));
    if (!projectiles->uvs)
    {
        printf("Error allocating memory for projectile uvs\n");
        projectiles_free(projectiles);
        return false;
    }
    projectiles->transition_time = malloc(sizeof(double));
    if (!projectiles->transition_time)
    {
        printf("Error allocating memory for projectile transition_time\n");
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

int projectiles_add(projectiles_t *projectiles, int player_id, vec3 position, vec3 direction)
{
    int new_id = 0;
    if (projectiles->free_ids.size == 0)
    {
        new_id = projectiles->count++;
        if (projectiles->count > projectiles->capacity)
        {
            projectiles->capacity *= 2;
            if (!cstrl_realloc_vec3(&projectiles->direction, projectiles->capacity))
            {
                printf("Error allocating projectile direction\n");
            }
            if (!cstrl_realloc_vec3(&projectiles->position, projectiles->capacity))
            {
                printf("Error allocating projectile position\n");
            }
            if (!cstrl_realloc_vec4(&projectiles->uvs, projectiles->capacity))
            {
                printf("Error allocating projectile uvs\n");
            }
            if (!cstrl_realloc_double(&projectiles->transition_time, projectiles->capacity))
            {
                printf("Error allocating projectile transition time\n");
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
    projectiles->direction[new_id] = direction;
    projectiles->position[new_id] = position;
    // TODO: paramaterize this
    projectiles->uvs[new_id] = (vec4){0.0f, 0.0f, 1.0f / 20.0f, 1.0f};
    projectiles->transition_time[new_id] = cstrl_platform_get_absolute_time();
    projectiles->active[new_id] = true;

    return new_id;
}

void projectiles_remove(projectiles_t *projectiles, int projectile_id)
{
    projectiles->position[projectile_id] = (vec3){0.0f, 0.0f, 0.0f};
    projectiles->active[projectile_id] = false;
    cstrl_da_int_push_back(&projectiles->free_ids, projectile_id);
}

void projectiles_free(projectiles_t *projectiles)
{
}
