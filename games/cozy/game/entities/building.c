#include "building.h"
#include <stdlib.h>

bool buildings_init(buildings_t *buildings)
{
    cstrl_da_int_init(buildings->freed_ids);
    return true;
}

int buildings_add(buildings_t *buildings, vec2 position)
{
}

void buildings_clean(buildings_t *buildings)
{
    cstrl_da_int_free(&buildings->freed_ids);
    free(buildings->position);
    buildings->position = NULL;
    free(buildings->type);
    buildings->type = NULL;
    free(buildings->collision_index);
    buildings->collision_index = NULL;
    buildings->count = 0;
    buildings->capacity = 0;
}
