#include "units.h"
#include "cstrl/cstrl_util.h"
#include <stdlib.h>

bool units_init(units_t *units)
{
    units->count = 0;
    units->capacity = 1;
    units->transforms = NULL;
    units->teams = NULL;
    units->active = NULL;

    cstrl_da_int_init(&units->free_ids, 1);

    units->transforms = malloc(sizeof(transform));
    if (!units->transforms)
    {
        printf("Error allocating memory for unit transforms\n");
        units_free(units);
        return false;
    }
    units->teams = malloc(sizeof(team_t));
    if (!units->teams)
    {
        printf("Error allocating memory for unit teams\n");
        units_free(units);
        return false;
    }
    units->active = malloc(sizeof(bool));
    if (!units->active)
    {
        printf("Error allocating memory for unit active\n");
        units_free(units);
        return false;
    }

    return true;
}

bool units_add(units_t *units, transform transform, team_t team)
{
    int new_id = 0;
    if (units->free_ids.size == 0)
    {
        new_id = units->count++;
        if (units->count > units->capacity)
        {
            units->capacity *= 2;
            if (!cstrl_realloc_transform(&units->transforms, units->capacity))
            {
                printf("Error allocating unit transforms\n");
                return false;
            }
            if (!cstrl_realloc_int(&units->teams, units->capacity))
            {
                printf("Error allocating unit teams\n");
                return false;
            }
            if (!cstrl_realloc_bool(&units->active, units->capacity))
            {
                printf("Error allocating unit active status\n");
                return false;
            }
        }
    }
    else
    {
        new_id = cstrl_da_int_pop_back(&units->free_ids);
    }

    units->transforms[new_id] = transform;
    units->teams[new_id] = team;
    units->active[new_id] = true;

    return true;
}

void units_remove(units_t *units, int unit_id)
{
    units->transforms[unit_id].position = (vec3){0.0f, 0.0f, 0.0f};
    units->transforms[unit_id].rotation = (quat){1.0f, 0.0f, 0.0f, 0.0f};
    units->transforms[unit_id].scale = (vec3){0.0f, 0.0f, 0.0f};
    units->teams[unit_id] = NO_TEAM;
    units->active[unit_id] = false;
    cstrl_da_int_push_back(&units->free_ids, unit_id);
}

void units_free(units_t *units)
{
    units->count = 0;
    units->capacity = 0;
    free(units->transforms);
    free(units->teams);
    free(units->active);
    cstrl_da_int_free(&units->free_ids);
}
