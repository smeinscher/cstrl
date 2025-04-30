#include "player.h"
#include <stdio.h>
#include <string.h>

void players_init(players_t *players, bool playing_doubles, int human_players)
{
    if (!playing_doubles)
    {
        players->active[2] = false;
        players->active[3] = false;
    }
    memset(players->human, 0, 4 * sizeof(bool));
    if (human_players & PLAYER1)
    {
        players->human[0] = true;
    }
    if (human_players & PLAYER2)
    {
        players->human[1] = true;
    }
    if (human_players & PLAYER3)
    {
        players->human[2] = true;
    }
    if (human_players & PLAYER4)
    {
        players->human[3] = true;
    }

    players->current_player_turn = PLAYER1_TURN;
    if (human_players != 0)
    {
        players->current_turn_state = AIM_TARGET;
    }
    else
    {
        players->current_turn_state = STARTED_SHOT;
    }
    players->base_game_state = EYE_TO_EYE_STAGE;
}

void players_rerun_turn(players_t *players)
{
    players->current_player_turn = players->current_player_turn != PLAYER4_TURN ? PLAYER1_TURN : PLAYER3_TURN;
    players->current_turn_state = players->human[players->current_player_turn] ? AIM_TARGET : STARTED_SHOT;
}

void players_advance_turn_state(players_t *players)
{
    if (players->current_turn_state == TURN_END)
    {
        if (players->base_game_state != EYE_TO_EYE_STAGE)
        {
            players->current_player_turn++;
            if (players->current_player_turn == MAX_PLAYER_TURN)
            {
                players->current_player_turn = PLAYER1_TURN;
            }
            players->current_turn_state = players->human[players->current_player_turn] ? AIM_TARGET : STARTED_SHOT;
        }
        else
        {
            players->current_player_turn = (players->current_player_turn + 1) % 2;
            players->current_turn_state = players->human[players->current_player_turn] ? AIM_TARGET : STARTED_SHOT;
        }
    }
    else
    {
        players->current_turn_state++;
    }
}
