#include "guy.h"
#include "../random/cozy_random.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>

bool guys_init(guys_t *guys)
{
    cstrl_da_int_init(&guys->freed_ids, 2);

    guys->color = malloc(sizeof(vec3));
    if (!guys->color)
    {
        printf("Failed to malloc color\n");
        return false;
    }

    guys->position = malloc(sizeof(vec3));
    if (!guys->position)
    {
        printf("Failed to malloc position\n");
        return false;
    }

    guys->velocity = malloc(sizeof(vec3));
    if (!guys->velocity)
    {
        printf("Failed to malloc velocity\n");
        return false;
    }

    guys->animation_frame = malloc(sizeof(int));
    if (!guys->animation_frame)
    {
        printf("Failed to malloc animation_frame\n");
        return false;
    }

    guys->type = malloc(sizeof(int));
    if (!guys->type)
    {
        printf("Failed to malloc type\n");
        return false;
    }

    guys->collision_index = malloc(sizeof(int));
    if (!guys->type)
    {
        printf("Failed to malloc collision_index\n");
        return false;
    }

    guys->animation_last_frame = malloc(sizeof(double));
    if (!guys->animation_last_frame)
    {
        printf("Failed to malloc animation_last_frame\n");
        return false;
    }

    guys->animate = malloc(sizeof(bool));
    if (!guys->animate)
    {
        printf("Failed to malloc animate\n");
        return false;
    }

    guys->active = malloc(sizeof(bool));
    if (!guys->active)
    {
        printf("Failed to malloc active\n");
        return false;
    }

    guys->count = 0;
    guys->capacity = 1;

    return true;
}

int guys_add(guys_t *guys, aabb_tree_t *aabb_tree, vec3 position, vec3 color)
{
    int new_id = 0;
    if (guys->freed_ids.size == 0)
    {
        new_id = guys->count;
        if (guys->count + 1 > guys->capacity)
        {
            size_t new_capacity = guys->capacity * 2;
            if (!cstrl_realloc_vec3(&guys->color, new_capacity))
            {
                printf("Error reallocating color\n");
                return -1;
            }
            if (!cstrl_realloc_vec3(&guys->position, new_capacity))
            {
                printf("Error reallocating position\n");
                return -1;
            }
            if (!cstrl_realloc_vec3(&guys->velocity, new_capacity))
            {
                printf("Error reallocating velocity\n");
                return -1;
            }
            if (!cstrl_realloc_int(&guys->animation_frame, new_capacity))
            {
                printf("Error reallocating animation_frame\n");
                return -1;
            }
            if (!cstrl_realloc_int(&guys->type, new_capacity))
            {
                printf("Error reallocating type\n");
                return -1;
            }
            if (!cstrl_realloc_int(&guys->collision_index, new_capacity))
            {
                printf("Error reallocating collision_index\n");
                return -1;
            }
            if (!cstrl_realloc_double(&guys->animation_last_frame, new_capacity))
            {
                printf("Error reallocating animation_last_frame\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&guys->animate, new_capacity))
            {
                printf("Error reallocating animate\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&guys->active, new_capacity))
            {
                printf("Error reallocating active\n");
                return -1;
            }
            guys->capacity = new_capacity;
        }
        guys->count++;
    }
    else
    {
        new_id = cstrl_da_int_pop_back(&guys->freed_ids);
    }

    guys->color[new_id] = color;
    guys->position[new_id] = position;
    guys->velocity[new_id] =
        cstrl_vec3_normalize((vec3){cozy_random_float(-1.0f, 1.0f), 0.0f, cozy_random_float(-1.0f, 1.0f)});
    guys->animation_last_frame[new_id] = 0.0;
    guys->animation_frame[new_id] = -1;
    guys->type[new_id] = cozy_random_int(0, GUY_TOTAL_TYPES - 1);
    vec3 aabb[2];
    aabb[0] = (vec3){guys->position[new_id].x - GUY_SIZE / 2.0f, guys->position[new_id].y - GUY_SIZE / 2.0f,
                     guys->position[new_id].z - GUY_SIZE / 2.0f};
    aabb[1] = (vec3){guys->position[new_id].x + GUY_SIZE / 2.0f, guys->position[new_id].y + GUY_SIZE / 2.0f,
                     guys->position[new_id].z + GUY_SIZE / 2.0f};
    guys->animate[new_id] = false;
    guys->active[new_id] = true;
    int *id = malloc(sizeof(int));
    if (!id)
    {
        guys->collision_index[new_id] = -1;
        printf("Failed to malloc id\n");
        return new_id;
    }
    *id = new_id;
    guys->collision_index[new_id] = cstrl_collision_aabb_tree_insert(aabb_tree, id, aabb);

    return new_id;
}

void guys_update(guys_t *guys, aabb_tree_t *aabb_tree)
{
    for (int i = 0; i < guys->count; i++)
    {
        if (!guys->active[i])
        {
            continue;
        }
        int random = cozy_random_int(0, 1000);
        if (random < 20)
        {
            guys->velocity[i] =
                cstrl_vec3_normalize((vec3){cozy_random_float(-1.0f, 1.0f), 0.0f, cozy_random_float(-1.0f, 1.0f)});
        }
        if (guys->position[i].x < -1.0f)
        {
            guys->position[i].x = -1.0f;
            guys->velocity[i].x *= -1.0f;
        }
        else if (guys->position[i].x > 1.0f)
        {
            guys->position[i].x = 1.0f;
            guys->velocity[i].x *= -1.0f;
        }
        if (guys->position[i].z < -1.0f)
        {
            guys->position[i].z = -1.0f;
            guys->velocity[i].z *= -1.0f;
        }
        else if (guys->position[i].z > 1.0f)
        {
            guys->position[i].z = 1.0f;
            guys->velocity[i].z *= -1.0f;
        }

        // guys->velocity[i] =
        //     cstrl_vec2_normalize(cstrl_vec2_add(guys->velocity[i], compute_avoidance(aabb_tree, guys, i)));

        if (!guys->animate[i])
        {
            guys->position[i] = cstrl_vec3_add(guys->position[i], cstrl_vec3_mult_scalar(guys->velocity[i], GUY_SCALE));
            vec3 new_aabb[2];
            new_aabb[0] = (vec3){guys->position[i].x - GUY_SIZE / 2.0f, guys->position[i].y - GUY_SIZE / 2.0f,
                                 guys->position[i].z - GUY_SIZE / 2.0f};
            new_aabb[1] = (vec3){guys->position[i].x + GUY_SIZE / 2.0f, guys->position[i].y + GUY_SIZE / 2.0f,
                                 guys->position[i].z + GUY_SIZE / 2.0f};
            cstrl_collision_aabb_tree_update_node(aabb_tree, guys->collision_index[i], new_aabb);
            continue;
        }
        double current_time = cstrl_platform_get_absolute_time();
        if (current_time - guys->animation_last_frame[i] > GUY_ANIMATION_FRAME_TIME)
        {
            guys->animation_frame[i]++;
            guys->animation_last_frame[i] = current_time;
            if (guys->animation_frame[i] == GUY_ANIMATION_TOTAL_FRAMES)
            {
                guys->active[i] = false;
                cstrl_da_int_push_back(&guys->freed_ids, i);
            }
        }
    }
}

void guys_clean(guys_t *guys, aabb_tree_t *aabb_tree)
{
    cstrl_da_int_free(&guys->freed_ids);
    free(guys->color);
    guys->color = NULL;
    free(guys->position);
    guys->position = NULL;
    free(guys->velocity);
    guys->velocity = NULL;
    free(guys->animation_last_frame);
    guys->animation_last_frame = NULL;
    free(guys->animation_frame);
    guys->animation_frame = NULL;
    free(guys->type);
    guys->type = NULL;
    if (aabb_tree)
    {
        for (int i = 0; i < guys->count; i++)
        {
            cstrl_collision_aabb_tree_remove(aabb_tree, guys->collision_index[i]);
        }
    }
    free(guys->collision_index);
    guys->collision_index = NULL;
    free(guys->animate);
    guys->animate = NULL;
    free(guys->active);
    guys->active = NULL;
    guys->count = 0;
    guys->capacity = 0;
}
