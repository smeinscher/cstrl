#ifndef CUP_H
#define CUP_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_util.h"
#include <stdbool.h>

#define TOTAL_CUP_COUNT 20
#define CUP_SIZE 9.0f

typedef struct cups_t
{
    vec2 position[TOTAL_CUP_COUNT];
    bool active[TOTAL_CUP_COUNT];
    da_int freed;
} cups_t;

bool cups_init(cups_t *cups, bool overtime);

int cups_shot_test(cups_t *cups, float ball_size, vec2 hit_position, float *distance_from_center);

void cups_make(cups_t *cups, int cup_id);

float cups_get_priority(cups_t *cups, int cup_id, int team);

void cups_get_active_cups_by_team(cups_t *cups, da_int *active_cups, int team);

int cups_count_active_by_team(cups_t *cups, int team);

bool cups_can_rerack(cups_t *cups, int team);

bool cups_rerack(cups_t *cups, int team);

void cups_free(cups_t *cups);

#endif // CUP_H
