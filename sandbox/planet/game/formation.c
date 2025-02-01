#include "formation.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>

bool formations_init(formations_t *formations)
{
    formations->count = 0;
    formations->capacity = 1;
    formations->path_head = NULL;
    formations->moving = NULL;
    formations->active = NULL;
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
    formations->path_head = malloc(sizeof(int));
    if (!formations->path_head)
    {
        printf("Error allocating memory for formations path_head\n");
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

int formations_add(formations_t *formations, int *unit_ids)
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
                return -1;
            }
            if (!cstrl_realloc_bool(&formations->active, formations->capacity))
            {
                printf("Error allocating formation active\n");
                return -1;
            }
            if (!cstrl_realloc_int(&formations->path_head, formations->capacity))
            {
                printf("Error allocating formation path_heads\n");
                return -1;
            }
            if (!cstrl_realloc_da_int(&formations->unit_ids, formations->capacity))
            {
                printf("Error allocating formation unit_ids\n");
                return -1;
            }
        }
    }
    else
    {
        new_id = cstrl_da_int_pop_back(&formations->free_ids);
    }
    formations->moving[new_id] = false;
    formations->active[new_id] = true;
    formations->path_head[new_id] = -1;
    cstrl_da_int_init(&formations->unit_ids[new_id], 1);
    // TODO: go through list
    cstrl_da_int_push_back(&formations->unit_ids[new_id], unit_ids[0]);
    return new_id;
}

void formations_remove(formations_t *formations, int formation_id)
{
    formations->moving[formation_id] = false;
    formations->active[formation_id] = false;
    formations->path_head[formation_id] = -1;
    cstrl_da_int_free(&formations->unit_ids[formation_id]);
    cstrl_da_int_push_back(&formations->free_ids, formation_id);
}

void formations_free(formations_t *formations)
{
    for (int i = 0; i < formations->count; i++)
    {
        if (formations->active[i])
        {
            cstrl_da_int_free(&formations->unit_ids[i]);
        }
    }
    formations->count = 0;
    formations->capacity = 0;
    free(formations->moving);
    free(formations->active);
    free(formations->unit_ids);
    cstrl_da_int_free(&formations->free_ids);
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
