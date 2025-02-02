#ifndef UNITS_H
#define UNITS_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"

#define UNIT_SIZE (vec3){0.075f, 0.075f, 0.0f}

typedef enum team_t
{
    RED,
    GREEN,
    BLUE,
    YELLOW,
    PURPLE,
    CYAN,
    WHITE,
    NO_TEAM
} team_t;

typedef struct units_t
{
    size_t count;
    size_t capacity;
    vec3 *position;
    int *team;
    bool *active;
    int *formation_id;
    da_int free_ids;
} units_t;

bool units_init(units_t *units);

int units_add(units_t *units, vec3 positions, team_t team);

void units_remove(units_t *units, int unit_id);

void units_free(units_t *units);

#endif // UNITS_H
