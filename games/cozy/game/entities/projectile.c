#include "projectile.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>

bool projectiles_init(projectiles_t *projectiles)
{
    cstrl_da_int_init(&projectiles->freed_ids, 2);

    projectiles->color = malloc(sizeof(vec3));
    if (!projectiles->color)
    {
        printf("Failed to malloc color\n");
        projectiles_clean(projectiles);
        return false;
    }

    projectiles->position = malloc(sizeof(vec3));
    if (!projectiles->position)
    {
        printf("Failed to malloc position\n");
        projectiles_clean(projectiles);
        return false;
    }

    projectiles->velocity = malloc(sizeof(vec3));
    if (!projectiles->velocity)
    {
        printf("Failed to malloc velocity\n");
        projectiles_clean(projectiles);
        return false;
    }

    projectiles->distance = malloc(sizeof(float));
    if (!projectiles->distance)
    {
        printf("Failed to malloc distance\n");
        projectiles_clean(projectiles);
        return false;
    }

    projectiles->active = malloc(sizeof(bool));
    if (!projectiles->active)
    {
        printf("Failed to malloc active\n");
        projectiles_clean(projectiles);
        return false;
    }

    projectiles->count = 0;
    projectiles->capacity = 1;

    return true;
}

int projectiles_add(projectiles_t *projectiles, vec3 position, vec3 velocity, vec3 color)
{
    int new_id = 0;
    if (projectiles->freed_ids.size == 0)
    {
        new_id = projectiles->count;
        if (projectiles->count + 1 > projectiles->capacity)
        {
            size_t new_capacity = projectiles->capacity * 2;
            if (!cstrl_realloc_vec3(&projectiles->color, new_capacity))
            {
                printf("Error reallocating color\n");
                return -1;
            }
            if (!cstrl_realloc_vec3(&projectiles->position, new_capacity))
            {
                printf("Error reallocating position\n");
                return -1;
            }
            if (!cstrl_realloc_vec3(&projectiles->velocity, new_capacity))
            {
                printf("Error reallocating velocity\n");
                return -1;
            }
            if (!cstrl_realloc_float(&projectiles->distance, new_capacity))
            {
                printf("Error reallocating distance\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&projectiles->active, new_capacity))
            {
                printf("Error reallocating active\n");
                return -1;
            }
            projectiles->capacity = new_capacity;
        }
        projectiles->count++;
    }
    else
    {
        new_id = cstrl_da_int_pop_back(&projectiles->freed_ids);
    }

    projectiles->color[new_id] = color;
    projectiles->position[new_id] = position;
    projectiles->velocity[new_id] = velocity;
    projectiles->distance[new_id] = 0.0f;
    projectiles->active[new_id] = true;

    return new_id;
}

void projectiles_update(projectiles_t *projectiles, aabb_tree_t *aabb_tree, guys_t *guys)
{
    for (int i = 0; i < projectiles->count; i++)
    {
        if (!projectiles->active[i])
        {
            continue;
        }
        if (projectiles->distance[i] > PROJECTILE_TRAVEL_DISTANCE)
        {
            projectiles->active[i] = false;
            continue;
        }
        projectiles->distance[i] +=
            cstrl_vec3_length(cstrl_vec3_mult_scalar(projectiles->velocity[i], PROJECTILE_SPEED));
        projectiles->position[i] = cstrl_vec3_add(projectiles->position[i],
                                                  cstrl_vec3_mult_scalar(projectiles->velocity[i], PROJECTILE_SPEED));

        vec3 aabb[2];
        aabb[0] = (vec3){projectiles->position[i].x - PROJECTILE_SIZE / 2.0f,
                         projectiles->position[i].y - PROJECTILE_SIZE / 2.0f,
                         projectiles->position[i].z - PROJECTILE_SIZE / 2.0f};
        aabb[1] = (vec3){projectiles->position[i].x + PROJECTILE_SIZE / 2.0f,
                         projectiles->position[i].y + PROJECTILE_SIZE / 2.0f,
                         projectiles->position[i].z + PROJECTILE_SIZE / 2.0f};
        da_int intersecting_nodes;
        cstrl_da_int_init(&intersecting_nodes, 4);
        cstrl_collision_aabb_tree_query(aabb_tree, aabb, &intersecting_nodes);
        if (intersecting_nodes.size > 0)
        {
            bool stay_active = true;
            for (int j = 0; j < intersecting_nodes.size; j++)
            {
                int *id = aabb_tree->nodes[intersecting_nodes.array[j]].user_data;
                if (!id || *id < 0)
                {
                    continue;
                }
                stay_active = false;
                guys->animate[*id] = true;
                cstrl_collision_aabb_tree_remove(aabb_tree, intersecting_nodes.array[j]);
            }
            projectiles->active[i] = stay_active;
        }
    }
}

void projectiles_clean(projectiles_t *projectiles)
{
    cstrl_da_int_free(&projectiles->freed_ids);
    free(projectiles->color);
    projectiles->color = NULL;
    free(projectiles->position);
    projectiles->position = NULL;
    free(projectiles->velocity);
    projectiles->velocity = NULL;
    free(projectiles->distance);
    projectiles->distance = NULL;
    free(projectiles->active);
    projectiles->active = NULL;
    projectiles->count = 0;
    projectiles->capacity = 0;
}
