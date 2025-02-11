#ifndef UNITS_H
#define UNITS_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"

#define UNIT_SIZE (vec3){0.075f, 0.075f, 0.0f}

typedef enum team_t
{
    RED,
    BLUE,
    GREEN,
    YELLOW,
    WHITE,
    CYAN,
    PURPLE,
    NO_TEAM
} team_t;

typedef enum unit_type
{
    CITY,
    HUMVEE,
    JET,
    PLANE,
    TANK,
    MAX_UNIT_TYPES
} unit_type;

typedef struct units_t
{
    size_t count;
    size_t capacity;
    vec3 *position;
    bool *active;
    int *formation_id;
    int *type;
    da_int free_ids;
} units_t;

bool units_init(units_t *units);

int units_hit(units_t *units, vec3 position);

int units_add(units_t *units, vec3 positions, int type);

void units_remove(units_t *units, int unit_id);

void units_free(units_t *units);

#endif // UNITS_H
