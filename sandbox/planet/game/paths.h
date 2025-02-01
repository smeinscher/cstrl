#ifndef PATHS_H
#define PATHS_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"

typedef struct paths_t
{
    size_t count;
    size_t capacity;
    int *prev;
    int *next;
    vec3 *start_positions;
    vec3 *end_positions;
    float *progress;
    bool *completed;
    bool *render;
    bool *in_queue;
    bool *active;
    da_int free_ids;
} paths_t;

bool paths_init(paths_t *paths);

int paths_add(paths_t *paths, vec3 start_position, vec3 end_position, int prev);

void paths_remove(paths_t *paths, int path_id);

void paths_free(paths_t *paths);

void paths_update(paths_t *paths);

#endif // PATHS_H
