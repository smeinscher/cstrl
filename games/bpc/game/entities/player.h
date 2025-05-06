#ifndef PLAYER_H
#define PLAYER_H

#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_types.h"
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

typedef struct player_metrics_t
{
    da_int shot_history;
    int cups_made;
    int successful_shots;
    int attempted_shots;
    int drink_count;
} player_metrics_t;

typedef CSTRL_PACKED_ENUM{PLAYER1_TURN, PLAYER2_TURN, PLAYER3_TURN, PLAYER4_TURN,
                          MAX_PLAYER_TURN} player_active_state_t;

typedef CSTRL_PACKED_ENUM{AIM_TARGET, AIM_METER, STARTED_SHOT, SHOOTING, TURN_END} player_turn_state_t;

enum possible_players
{
    PLAYER1 = 0x1,
    PLAYER2 = 0x2,
    PLAYER3 = 0x4,
    PLAYER4 = 0x8
};

typedef CSTRL_PACKED_ENUM{TRANSITION_STAGE, EYE_TO_EYE_STAGE, MAIN_GAME_STAGE, REBUTTAL_ATTEMPT1_STAGE, REBUTTAL_ATTEMPT2_STAGE, OVERTIME_TRANSITION_STAGE, OVERTIME_STAGE,
                          GAME_OVER_STAGE} player_base_game_state_t;

typedef CSTRL_PACKED_ENUM{MISS_SHOT, MAKE_SHOT, BOUNCE_SHOT} player_shot_state_t;

typedef struct players_t
{
    player_stats_t stats[MAX_PLAYER_COUNT];
    player_metrics_t metrics[MAX_PLAYER_COUNT];
    bool human[MAX_PLAYER_COUNT];
    bool active[MAX_PLAYER_COUNT];
    // TODO: do this a different way
    int team1_cups_remaining;
    int team2_cups_remaining;
    int team1_reracks_remaining;
    int team2_reracks_remaining;
    bool first_turn;
    player_active_state_t current_player_turn;
    player_turn_state_t current_turn_state;
    player_base_game_state_t base_game_state;
} players_t;

void players_init(players_t *players, bool playing_doubles, int human_players);

void players_advance_turn_state(players_t *players);

void players_complete_shot(players_t *players, player_shot_state_t shot_state, int cups_made);

void players_reset(players_t *players);

void players_free(players_t *players);

#endif
