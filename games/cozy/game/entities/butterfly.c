#include "butterfly.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>

bool butterflies_init(butterflies_t *butterflies)
{
    cstrl_da_int_init(&butterflies->freed_ids, 2);

    butterflies->position = malloc(sizeof(vec2));
    if (!butterflies->position)
    {
        printf("Failed to malloc position\n");
        return false;
    }

    butterflies->velocity = malloc(sizeof(vec2));
    if (!butterflies->position)
    {
        printf("Failed to malloc velocity\n");
        return false;
    }

    butterflies->animation_frame = malloc(sizeof(int));
    if (!butterflies->animation_frame)
    {
        printf("Failed to malloc animation_frame\n");
        return false;
    }

    butterflies->animation_last_frame = malloc(sizeof(double));
    if (!butterflies->animation_last_frame)
    {
        printf("Failed to malloc animation_last_frame\n");
        return false;
    }

    butterflies->active = malloc(sizeof(bool));
    if (!butterflies->active)
    {
        printf("Failed to malloc active\n");
        return false;
    }

    butterflies->count = 0;
    butterflies->capacity = 1;

    return true;
}

int butterflies_add(butterflies_t *butterflies, vec2 position)
{
    int new_id = 0;
    if (butterflies->freed_ids.size == 0)
    {
        new_id = butterflies->count;
        if (butterflies->count > butterflies->capacity)
        {
            size_t new_capacity = butterflies->capacity * 2;
            if (!cstrl_realloc_vec2(&butterflies->position, new_capacity))
            {
                printf("Error reallocating position\n");
                return -1;
            }
            if (!cstrl_realloc_vec2(&butterflies->velocity, new_capacity))
            {
                printf("Error reallocating velocity\n");
                return -1;
            }
            if (!cstrl_realloc_int(&butterflies->animation_frame, new_capacity))
            {
                printf("Error reallocating animation_frame\n");
                return -1;
            }
            if (!cstrl_realloc_double(&butterflies->animation_last_frame, new_capacity))
            {
                printf("Error reallocating animation_last_frame\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&butterflies->active, new_capacity))
            {
                printf("Error reallocating active\n");
                return -1;
            }
            butterflies->capacity = new_capacity;
        }
        butterflies->count++;
    }
    else
    {
        new_id = cstrl_da_int_pop_back(&butterflies->freed_ids);
    }

    butterflies->position[new_id] = position;
    butterflies->velocity[new_id] = (vec2){0.0f, 0.0f};
    butterflies->animation_frame[new_id] = 0;
    butterflies->animation_last_frame[new_id] = 0.0;
    butterflies->active[new_id] = true;

    return new_id;
}

void butterflies_update(butterflies_t *butterflies)
{
    for (int i = 0; i < butterflies->count; i++)
    {
        if (!butterflies->active[i])
        {
            continue;
        }

        butterflies->position[i] = cstrl_vec2_add(butterflies->position[i], butterflies->velocity[i]);

        double current_time = cstrl_platform_get_absolute_time();
        if (current_time - butterflies->animation_last_frame[i] > BUTTERFLY_ANIMATION_FRAME_TIME)
        {
            butterflies->animation_frame[i] = (butterflies->animation_frame[i] + 1) % BUTTERFLY_ANIMATION_TOTAL_FRAMES;
            butterflies->animation_last_frame[i] = current_time;
        }
    }
}
