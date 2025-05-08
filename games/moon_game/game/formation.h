#ifndef FORMATION_H
#define FORMATION_H

#include "cstrl/cstrl_util.h"
#include <stdbool.h>

typedef struct formations_t
{
    size_t count;
    size_t capacity;
    bool *active;
    bool *moving;
    bool *following_enemy;
    da_int *path_heads;
    da_int *unit_ids;
    da_int free_ids;
} formations_t;

bool formations_init(formations_t *formations);

int formations_add(formations_t *formations);

void formations_remove(formations_t *formations, int formation_id);

void formations_free(formations_t *formations);

void formations_add_unit(formations_t *formations, int formation_id, int unit_id);

void formations_remove_unit(formations_t *formations, int formation_id, int unit_id);

#endif // FORMATION_H
