#include "guy.h"
#include "../random/cozy_random.h"
#include "cstrl/cstrl_math.h"
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

    guys->position = malloc(sizeof(vec2));
    if (!guys->position)
    {
        printf("Failed to malloc position\n");
        return false;
    }

    guys->velocity = malloc(sizeof(vec2));
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

int guys_add(guys_t *guys, vec2 position, vec3 color)
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
            if (!cstrl_realloc_vec2(&guys->position, new_capacity))
            {
                printf("Error reallocating position\n");
                return -1;
            }
            if (!cstrl_realloc_vec2(&guys->velocity, new_capacity))
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
        cstrl_vec2_normalize((vec2){cozy_random_float(-1.0f, 1.0f), cozy_random_float(-1.0f, 1.0f)});
    guys->animation_last_frame[new_id] = 0.0;
    guys->animation_frame[new_id] = -1;
    guys->type[new_id] = cozy_random_int(0, GUY_TOTAL_TYPES - 1);
    guys->animate[new_id] = false;
    guys->active[new_id] = true;

    return new_id;
}

void guys_update(guys_t *guys)
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
                cstrl_vec2_normalize((vec2){cozy_random_float(-1.0f, 1.0f), cozy_random_float(-1.0f, 1.0f)});
        }
        if (guys->position[i].x < 0)
        {
            guys->position[i].x = 0;
            guys->velocity[i].x *= -1.0f;
        }
        else if (guys->position[i].x > 1280)
        {
            guys->position[i].x = 1280;
            guys->velocity[i].x *= -1.0f;
        }
        if (guys->position[i].y < 0)
        {
            guys->position[i].y = 0;
            guys->velocity[i].y *= -1.0f;
        }
        else if (guys->position[i].y > 720)
        {
            guys->position[i].y = 720;
            guys->velocity[i].y *= -1.0f;
        }
        if (random == 995)
        {
            guys->animate[i] = true;
        }

        if (!guys->animate[i])
        {
            guys->position[i] = cstrl_vec2_add(guys->position[i], guys->velocity[i]);
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
