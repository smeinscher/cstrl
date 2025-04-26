#ifndef PLAYER_H
#define PLAYER_H

#include "cstrl/cstrl_defines.h"
#include <stdbool.h>

#define MAX_PLAYER_COUNT 4

typedef struct player_stats_t
{
    int accuracy;
    int focus;
    int tolerance;
    int charisma;
    int defence;
} player_stats_t;

typedef CSTRL_PACKED_ENUM
{
    PLAYER1_TURN,
    PLAYER2_TURN,
    PLAYER3_TURN,
    PLAYER4_TURN,
    MAX_PLAYER_TURN
} player_active_state_t;

typedef CSTRL_PACKED_ENUM
{
    AIM_TARGET,
    AIM_METER,
    SHOOTING,
    TURN_END
} player_turn_state_t;

enum possible_players
{
    PLAYER1 = 0x1,
    PLAYER2 = 0x2,
    PLAYER3 = 0x4,
    PLAYER4 = 0x8
};

typedef struct players_t
{
    player_stats_t stats[MAX_PLAYER_COUNT];
    int drink_count[MAX_PLAYER_COUNT];
    bool human[MAX_PLAYER_COUNT];
    bool active[MAX_PLAYER_COUNT];
    player_active_state_t current_player_turn;
    player_turn_state_t current_turn_state;

} players_t;

void players_init(players_t *players, bool playing_doubles, int human_players);

void players_rerun_turn(players_t *players);

void players_advance_turn_state(players_t *players);

#endif
