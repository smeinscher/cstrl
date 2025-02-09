#ifndef AI_H
#define AI_H

#include "players.h"
#include <stdbool.h>

#define MAX_AI_COUNT 8

typedef enum ai_objective
{
    MOVE_RANDOMLY
} ai_objective;

typedef struct ai_t
{
    ai_objective objective[MAX_AI_COUNT];
    bool active[MAX_AI_COUNT];
    int player_id[MAX_AI_COUNT];
    int cached_unit_count[MAX_AI_COUNT];
} ai_t;

void ai_init(ai_t *ai, int player_count, int human_player_id);

void ai_update(ai_t *ai, players_t *players);

#endif
