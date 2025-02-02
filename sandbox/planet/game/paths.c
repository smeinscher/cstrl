#include "paths.h"
#include "cstrl/cstrl_util.h"
#include <stdlib.h>

bool paths_init(paths_t *paths)
{
    paths->count = 0;
    paths->capacity = 1;
    paths->start_positions = NULL;
    paths->end_positions = NULL;
    paths->progress = NULL;
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

    return true;
}

int paths_add(paths_t *paths, vec3 start_position, vec3 end_position, int prev)
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
    paths->completed[new_id] = false;
    paths->render[new_id] = true;
    paths->in_queue[new_id] = false;
    paths->active[new_id] = true;

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
    free(paths->start_positions);
    free(paths->end_positions);
    free(paths->progress);
    free(paths->completed);
    free(paths->active);
    cstrl_da_int_free(&paths->free_ids);
}

void paths_update(paths_t *paths)
{
    for (int i = 0; i < paths->count; i++)
    {
        if (!paths->active[i] || paths->in_queue[i] || paths->completed[i])
        {
            continue;
        }
        // TODO: replace 0.0025f with speed variable
        paths->progress[i] +=
            0.0025f / cstrl_vec3_length(cstrl_vec3_sub(paths->end_positions[i], paths->start_positions[i]));
        if (paths->progress[i] >= 1.0f)
        {
            paths->completed[i] = true;
            paths->progress[i] = 0.0f;
        }
    }
}
