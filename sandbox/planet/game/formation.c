#include "formation.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>

bool formations_init(formations_t *formations)
{
    formations->count = 0;
    formations->capacity = 1;
    formations->moving = NULL;
    formations->path_ids = NULL;
    formations->unit_ids = NULL;

    formations->moving = malloc(sizeof(bool));
    if (!formations->moving)
    {
        printf("Error allocating memory for formations moving\n");
        formations_free(formations);
        return false;
    }
    formations->active = malloc(sizeof(bool));
    if (!formations->active)
    {
        printf("Error allocating memory for formations active\n");
        formations_free(formations);
        return false;
    }
    formations->path_ids = malloc(sizeof(da_int));
    if (!formations->path_ids)
    {
        printf("Error allocating memory for formations path_ids\n");
        formations_free(formations);
        return false;
    }
    formations->unit_ids = malloc(sizeof(da_int));
    if (!formations->unit_ids)
    {
        printf("Error allocating memory for formations unit_ids\n");
        formations_free(formations);
        return false;
    }
    return true;
}

bool formations_add(formations_t *formations, int *unit_ids)
{
    int new_id = 0;
    if (formations->free_ids.size == 0)
    {
        new_id = formations->count++;
        if (formations->count > formations->capacity)
        {
            formations->capacity *= 2;
            if (!cstrl_realloc_bool(&formations->moving, formations->capacity))
            {
                printf("Error allocating formation moving\n");
                return false;
            }
            if (!cstrl_realloc_bool(&formations->active, formations->capacity))
            {
                printf("Error allocating formation active\n");
                return false;
            }
            if (!cstrl_realloc_da_int(&formations->path_ids, formations->capacity))
            {
                printf("Error allocating formation path_ids\n");
                return false;
            }
            if (!cstrl_realloc_da_int(&formations->unit_ids, formations->capacity))
            {
                printf("Error allocating formation unit_ids\n");
                return false;
            }
        }
    }
    else
    {
        new_id = cstrl_da_int_pop_back(&formations->free_ids);
    }
    formations->moving[new_id] = false;
    formations->active[new_id] = true;
    cstrl_da_int_init(&formations->path_ids[new_id], 1);
    cstrl_da_int_init(&formations->unit_ids[new_id], 1);
    return true;
}

void formations_remove(formations_t *formations, int formation_id)
{
    formations->moving = false;
    formations->active[formation_id] = false;
    cstrl_da_int_free(&formations->path_ids[formation_id]);
    cstrl_da_int_free(&formations->unit_ids[formation_id]);
    cstrl_da_int_push_back(&formations->free_ids, formation_id);
}

void formations_free(formations_t *formations)
{
    for (int i = 0; i < formations->count; i++)
    {
        if (formations->active[i])
        {
            cstrl_da_int_free(&formations->path_ids[i]);
            cstrl_da_int_free(&formations->unit_ids[i]);
        }
    }
    formations->count = 0;
    formations->capacity = 0;
    free(formations->moving);
    free(formations->active);
    free(formations->path_ids);
    free(formations->unit_ids);
    cstrl_da_int_free(&formations->free_ids);
}

void formations_add_path(formations_t *formations, int formation_id, int path_id)
{
    cstrl_da_int_push_back(&formations->path_ids[formation_id], path_id);
}

void formations_remove_path(formations_t *formations, int formation_id, int path_id)
{
    cstrl_da_int_remove(&formations->path_ids[formation_id],
                        cstrl_da_int_find_first(&formations->path_ids[formation_id], path_id));
}

void formations_add_unit(formations_t *formations, int formation_id, int unit_id)
{
    cstrl_da_int_push_back(&formations->unit_ids[formation_id], unit_id);
}

void formations_remove_unit(formations_t *formations, int formation_id, int unit_id)
{
    cstrl_da_int_remove(&formations->unit_ids[formation_id],
                        cstrl_da_int_find_first(&formations->unit_ids[formation_id], unit_id));
}
