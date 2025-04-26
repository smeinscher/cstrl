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
    players->current_turn_state = AIM_TARGET;
}

void players_rerun_turn(players_t *players)
{
    printf("Rerun turn\n");
    if (players->current_player_turn != PLAYER1_TURN)
    {
        printf("Player 1\n");
        players->current_player_turn = PLAYER1_TURN;
    }
    else
    {
        printf("Player 3\n");
        players->current_player_turn = PLAYER3_TURN;
    }
    if (!players->human[players->current_player_turn])
    {
        players->current_turn_state = SHOOTING;
    }
    else
    {
        players->current_turn_state = AIM_TARGET;
    }
}

void players_advance_turn_state(players_t *players)
{
    if (players->current_turn_state == TURN_END)
    {
        players->current_player_turn++;
        if (players->current_player_turn == MAX_PLAYER_TURN)
        {
            players->current_player_turn = PLAYER1_TURN;
        }
        if (!players->human[players->current_player_turn])
        {
            players->current_turn_state = SHOOTING;
        }
        else
        {
            players->current_turn_state = AIM_TARGET;
        }
    }
    else
    {
        players->current_turn_state++;
    }
    printf("Current Turn State: %d\n", players->current_turn_state);
    printf("Current Player Turn: %d\n", players->current_player_turn);
}
