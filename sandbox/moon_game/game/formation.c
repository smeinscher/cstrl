#include "formation.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>

bool formations_init(formations_t *formations)
{
    formations->count = 0;
    formations->capacity = 1;
    formations->active = NULL;
    formations->moving = NULL;
    formations->following_enemy = NULL;
    formations->path_heads = NULL;
    formations->unit_ids = NULL;

    cstrl_da_int_init(&formations->free_ids, 1);
    formations->active = malloc(sizeof(bool));
    if (!formations->active)
    {
        printf("Error allocating memory for formations active\n");
        formations_free(formations);
        return false;
    }
    formations->moving = malloc(sizeof(bool));
    if (!formations->moving)
    {
        printf("Error allocating memory for formations moving\n");
        formations_free(formations);
        return false;
    }
    formations->following_enemy = malloc(sizeof(bool));
    if (!formations->following_enemy)
    {
        printf("Error allocating memory for formations attacking\n");
        formations_free(formations);
        return false;
    }
    formations->path_heads = malloc(sizeof(da_int));
    if (!formations->path_heads)
    {
        printf("Error allocating memory for formations path_heads\n");
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

int formations_add(formations_t *formations)
{
    int new_id = 0;
    if (formations->free_ids.size == 0)
    {
        new_id = formations->count++;
        if (formations->count > formations->capacity)
        {
            formations->capacity *= 2;
            if (!cstrl_realloc_bool(&formations->active, formations->capacity))
            {
                printf("Error allocating formation active\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&formations->moving, formations->capacity))
            {
                printf("Error allocating formation moving\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&formations->following_enemy, formations->capacity))
            {
                printf("Error allocating formation attacking\n");
                return -1;
            }
            if (!cstrl_realloc_da_int(&formations->path_heads, formations->capacity))
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
    formations->active[new_id] = true;
    formations->moving[new_id] = false;
    formations->following_enemy[new_id] = false;
    cstrl_da_int_init(&formations->path_heads[new_id], 1);
    cstrl_da_int_init(&formations->unit_ids[new_id], 1);
    return new_id;
}

void formations_remove(formations_t *formations, int formation_id)
{
    formations->active[formation_id] = false;
    cstrl_da_int_free(&formations->path_heads[formation_id]);
    cstrl_da_int_free(&formations->unit_ids[formation_id]);
    cstrl_da_int_push_back(&formations->free_ids, formation_id);
}

void formations_free(formations_t *formations)
{
    for (int i = 0; i < formations->count; i++)
    {
        if (formations->active[i])
        {
            cstrl_da_int_free(&formations->path_heads[i]);
            cstrl_da_int_free(&formations->unit_ids[i]);
        }
    }
    formations->count = 0;
    formations->capacity = 0;
    free(formations->active);
    free(formations->moving);
    free(formations->following_enemy);
    free(formations->path_heads);
    free(formations->unit_ids);
    cstrl_da_int_free(&formations->free_ids);
}

void formations_add_unit(formations_t *formations, int formation_id, int unit_id)
{
    cstrl_da_int_push_back(&formations->path_heads[formation_id], -1);
    cstrl_da_int_push_back(&formations->unit_ids[formation_id], unit_id);
}

void formations_remove_unit(formations_t *formations, int formation_id, int unit_id)
{
    int index = cstrl_da_int_find_first(&formations->unit_ids[formation_id], unit_id);
    cstrl_da_int_remove(&formations->path_heads[formation_id], index);
    cstrl_da_int_remove(&formations->unit_ids[formation_id], index);
    if (formations->unit_ids[formation_id].size == 0)
    {
        formations_remove(formations, formation_id);
    }
}
