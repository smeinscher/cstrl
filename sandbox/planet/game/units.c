#include "units.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_util.h"
#include <stdlib.h>

bool units_init(units_t *units)
{
    units->count = 0;
    units->capacity = 1;
    units->positions = NULL;
    units->teams = NULL;
    units->active = NULL;
    units->formation_id = NULL;

    cstrl_da_int_init(&units->free_ids, 1);

    units->positions = malloc(sizeof(transform));
    if (!units->positions)
    {
        printf("Error allocating memory for unit positions\n");
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
    units->formation_id = malloc(sizeof(int));
    if (!units->formation_id)
    {
        printf("Error allocating memory for unit formation ids\n");
        units_free(units);
        return false;
    }

    return true;
}

int units_add(units_t *units, vec3 position, team_t team)
{
    float new_x0 = position.x - UNIT_SIZE.x / 2.0f;
    float new_x1 = position.x + UNIT_SIZE.x / 2.0f;
    float new_y0 = position.y - UNIT_SIZE.y / 2.0f;
    float new_y1 = position.y + UNIT_SIZE.y / 2.0f;
    float new_z = position.z;
    for (int i = 0; i < units->count; i++)
    {
        float taken_x0 = units->positions[i].x - UNIT_SIZE.x / 2.0f;
        float taken_x1 = units->positions[i].x + UNIT_SIZE.x / 2.0f;
        float taken_y0 = units->positions[i].y - UNIT_SIZE.y / 2.0f;
        float taken_y1 = units->positions[i].y + UNIT_SIZE.y / 2.0f;
        float taken_z = units->positions[i].z;

        bool same_side = cstrl_vec3_dot(position, units->positions[i]) > 0.0f;
        if ((new_x0 < taken_x1 && new_x1 > taken_x0 && new_y0 < taken_y1 && new_y1 > taken_y0) && same_side)
        {
            return -1;
        }
    }
    int new_id = 0;
    if (units->free_ids.size == 0)
    {
        new_id = units->count++;
        if (units->count > units->capacity)
        {
            units->capacity *= 2;
            if (!cstrl_realloc_vec3(&units->positions, units->capacity))
            {
                printf("Error allocating unit transforms\n");
                return -1;
            }
            if (!cstrl_realloc_int(&units->teams, units->capacity))
            {
                printf("Error allocating unit teams\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&units->active, units->capacity))
            {
                printf("Error allocating unit active status\n");
                return -1;
            }
            if (!cstrl_realloc_int(&units->formation_id, units->capacity))
            {
                printf("Error allocating unit formation_ids\n");
                return -1;
            }
        }
    }
    else
    {
        new_id = cstrl_da_int_pop_back(&units->free_ids);
    }

    units->positions[new_id] = position;
    units->teams[new_id] = team;
    units->active[new_id] = true;
    units->formation_id[new_id] = -1;

    return new_id;
}

void units_remove(units_t *units, int unit_id)
{
    units->positions[unit_id] = (vec3){0.0f, 0.0f, 0.0f};
    units->teams[unit_id] = NO_TEAM;
    units->active[unit_id] = false;
    cstrl_da_int_push_back(&units->free_ids, unit_id);
}

void units_free(units_t *units)
{
    units->count = 0;
    units->capacity = 0;
    free(units->positions);
    free(units->teams);
    free(units->active);
    cstrl_da_int_free(&units->free_ids);
}
