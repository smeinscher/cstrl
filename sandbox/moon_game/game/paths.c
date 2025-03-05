#include "paths.h"
#include "../helpers/helpers.h"
#include "cstrl/cstrl_util.h"
#include "units.h"
#include <stdio.h>
#include <stdlib.h>

bool paths_init(paths_t *paths)
{
    paths->count = 0;
    paths->capacity = 1;
    paths->start_positions = NULL;
    paths->end_positions = NULL;
    paths->progress = NULL;
    paths->speed = NULL;
    paths->completed = NULL;
    paths->render = NULL;
    paths->in_queue = NULL;
    paths->active = NULL;

    cstrl_da_int_init(&paths->free_ids, 1);

    paths->prev = malloc(sizeof(int));
    if (!paths->prev)
    {
        printf("Error allocating memory for path prev\n");
        paths_free(paths);
        return false;
    }
    paths->next = malloc(sizeof(int));
    if (!paths->next)
    {
        printf("Error allocating memory for path next\n");
        paths_free(paths);
        return false;
    }
    paths->start_positions = malloc(sizeof(vec3));
    if (!paths->start_positions)
    {
        printf("Error allocating memory for path start_positions\n");
        paths_free(paths);
        return false;
    }
    paths->end_positions = malloc(sizeof(vec3));
    if (!paths->end_positions)
    {
        printf("Error allocating memory for path end_positions\n");
        paths_free(paths);
        return false;
    }
    paths->progress = malloc(sizeof(float));
    if (!paths->progress)
    {
        printf("Error allocating memory for path progress\n");
        paths_free(paths);
        return false;
    }
    paths->speed = malloc(sizeof(float));
    if (!paths->speed)
    {
        printf("Error allocating memory for path speed\n");
        paths_free(paths);
        return false;
    }
    paths->completed = malloc(sizeof(bool));
    if (!paths->completed)
    {
        printf("Error allocating memory for path completed\n");
        paths_free(paths);
        return false;
    }
    paths->render = malloc(sizeof(bool));
    if (!paths->render)
    {
        printf("Error allocating memory for path render\n");
        paths_free(paths);
        return false;
    }
    paths->in_queue = malloc(sizeof(bool));
    if (!paths->in_queue)
    {
        printf("Error allocating memory for path in_queue\n");
        paths_free(paths);
        return false;
    }
    paths->active = malloc(sizeof(bool));
    if (!paths->active)
    {
        printf("Error allocating memory for path active\n");
        paths_free(paths);
        return false;
    }
    paths->tracked_unit = malloc(sizeof(unit_data_t));
    if (!paths->tracked_unit)
    {
        printf("Error allocating memory for path tracked unit\n");
        paths_free(paths);
        return false;
    }

    return true;
}

int paths_add(paths_t *paths, vec3 start_position, vec3 end_position, int prev, float speed, unit_data_t tracked_unit)
{
    int new_id = 0;
    if (paths->free_ids.size == 0)
    {
        new_id = paths->count++;
        if (paths->count > paths->capacity)
        {
            paths->capacity *= 2;
            if (!cstrl_realloc_vec3(&paths->start_positions, paths->capacity))
            {
                printf("Error allocating paths start_positions\n");
                return -1;
            }
            if (!cstrl_realloc_vec3(&paths->end_positions, paths->capacity))
            {
                printf("Error allocating paths end_positions\n");
                return -1;
            }
            if (!cstrl_realloc_int(&paths->prev, paths->capacity))
            {
                printf("Error allocating paths previous path\n");
                return -1;
            }
            if (!cstrl_realloc_int(&paths->next, paths->capacity))
            {
                printf("Error allocating paths next path\n");
                return -1;
            }
            if (!cstrl_realloc_float(&paths->progress, paths->capacity))
            {
                printf("Error allocating paths progress\n");
                return -1;
            }
            if (!cstrl_realloc_float(&paths->speed, paths->capacity))
            {
                printf("Error allocating paths speed\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&paths->completed, paths->capacity))
            {
                printf("Error allocating paths completed\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&paths->render, paths->capacity))
            {
                printf("Error allocating paths render\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&paths->in_queue, paths->capacity))
            {
                printf("Error allocating paths in_queue\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&paths->active, paths->capacity))
            {
                printf("Error allocating paths active\n");
                return -1;
            }
            {
                unit_data_t *temp = realloc(paths->tracked_unit, paths->capacity * sizeof(unit_data_t));
                if (!temp)
                {
                    printf("Error allocating path tracked unit\n");
                    return -1;
                }
                paths->tracked_unit = temp;
            }
        }
    }
    else
    {
        new_id = cstrl_da_int_pop_back(&paths->free_ids);
    }

    paths->prev[new_id] = prev;
    if (prev != -1)
    {
        paths->next[prev] = new_id;
    }
    paths->next[new_id] = -1;
    paths->start_positions[new_id] = start_position;
    paths->end_positions[new_id] = end_position;
    paths->progress[new_id] = 0.0f;
    paths->speed[new_id] = speed;
    paths->completed[new_id] = false;
    paths->render[new_id] = true;
    paths->in_queue[new_id] = false;
    paths->active[new_id] = true;
    paths->tracked_unit[new_id] = tracked_unit;

    return new_id;
}

void paths_remove(paths_t *paths, int path_id)
{
    paths->prev[path_id] = -1;
    paths->next[path_id] = -1;
    paths->start_positions[path_id] = (vec3){0.0f, 0.0f, 0.0f};
    paths->end_positions[path_id] = (vec3){0.0f, 0.0f, 0.0f};
    paths->progress[path_id] = 0.0f;
    paths->completed[path_id] = false;
    paths->active[path_id] = false;
    cstrl_da_int_push_back(&paths->free_ids, path_id);
}

void paths_recursive_remove(paths_t *paths, int path_id)
{
    if (paths->next[path_id] != -1)
    {
        paths_recursive_remove(paths, paths->next[path_id]);
    }
    paths_remove(paths, path_id);
}

void paths_free(paths_t *paths)
{
    paths->count = 0;
    paths->capacity = 0;
    free(paths->prev);
    free(paths->next);
    free(paths->start_positions);
    free(paths->end_positions);
    free(paths->progress);
    free(paths->speed);
    free(paths->completed);
    free(paths->render);
    free(paths->in_queue);
    free(paths->active);
    free(paths->tracked_unit);
    cstrl_da_int_free(&paths->free_ids);
}

void path_update(paths_t *paths, int path_id)
{
    if (path_id == -1 || !paths->active[path_id] || paths->in_queue[path_id] || paths->completed[path_id])
    {
        return;
    }
    // TODO: this might be incorrect
    paths->progress[path_id] += paths->speed[path_id] / get_spherical_path_length(paths->end_positions[path_id],
                                                                                  paths->start_positions[path_id]);
    if (paths->progress[path_id] >= 1.0f)
    {
        paths->completed[path_id] = true;
        paths->progress[path_id] = 0.0f;
    }
}
