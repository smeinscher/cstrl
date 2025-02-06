#ifndef AI_H
#define AI_H

#include <stdbool.h>

#define MAX_AI_COUNT 8

typedef enum ai_objective
{
    GO_IN_CIRCLES
} ai_objective;

typedef struct ai_t
{
    ai_objective objective[MAX_AI_COUNT];
    bool active[MAX_AI_COUNT];
} ai_t;

void ai_init(ai_t *ai);

void ai_update(ai_t *ai);

void ai_remove_unit(ai_t *ai, int ai_id, int unit_id);

#endif
