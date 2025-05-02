#include "player.h"
#include "cstrl/cstrl_util.h"
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

    cstrl_da_int_init(&players->metrics[0].shot_history, 10);
    cstrl_da_int_init(&players->metrics[1].shot_history, 10);
    cstrl_da_int_init(&players->metrics[2].shot_history, 10);
    cstrl_da_int_init(&players->metrics[3].shot_history, 10);

    // TODO: do this a different way
    players->team1_cups_remaining = 10;
    players->team2_cups_remaining = 10;
}

void players_advance_turn_state(players_t *players)
{
    if (players->current_turn_state == TURN_END)
    {
        switch (players->base_game_state)
        {
        case EYE_TO_EYE_STAGE:
            players->current_player_turn = (players->current_player_turn + 1) % 2;
            players->current_turn_state = players->human[players->current_player_turn] ? AIM_TARGET : STARTED_SHOT;
            break;
        case MAIN_GAME_STAGE:
        case OVERTIME_STAGE: {
            int index0 = players->metrics[players->current_player_turn].shot_history.size - 1;
            int index1 = players->metrics[players->current_player_turn - 1].shot_history.size - 1;
            if (players->team1_cups_remaining <= 0 || players->team2_cups_remaining <= 0)
            {
                printf("Rebuttal...\n");
                players->current_player_turn =
                    players->current_player_turn < PLAYER3_TURN ? PLAYER3_TURN : PLAYER1_TURN;
                players->base_game_state = REBUTTAL_ATTEMPT1_STAGE;
            }
            else if (players->current_player_turn % 2 == 0 ||
                     players->metrics[players->current_player_turn].shot_history.array[index0] == MISS_SHOT ||
                     index1 < 0 ||
                     players->metrics[players->current_player_turn - 1].shot_history.array[index1] == MISS_SHOT)
            {
                players->current_player_turn = (players->current_player_turn + 1) % MAX_PLAYER_TURN;
            }
            else
            {
                printf("Balls back\n");
                players->current_player_turn--;
            }
            players->current_turn_state = players->human[players->current_player_turn] ? AIM_TARGET : STARTED_SHOT;
            break;
        }
        case REBUTTAL_ATTEMPT1_STAGE:
            printf("Rebuttal Attempt 1\n");
        case REBUTTAL_ATTEMPT2_STAGE: {
            if (players->team1_cups_remaining == 0 && players->team2_cups_remaining == 0)
            {
                printf("Overtime...\n");
                players->base_game_state = OVERTIME_TRANSITION_STAGE;
                players->current_player_turn =
                    players->current_player_turn < PLAYER3_TURN ? PLAYER3_TURN : PLAYER1_TURN;
                players->current_turn_state = players->human[players->current_player_turn] ? AIM_TARGET : STARTED_SHOT;
                players->team1_cups_remaining = 6;
                players->team2_cups_remaining = 6;
                break;
            }
            int index = players->metrics[players->current_player_turn].shot_history.size - 1;
            if (players->metrics[players->current_player_turn].shot_history.array[index] == MISS_SHOT)
            {
                if (players->base_game_state != REBUTTAL_ATTEMPT1_STAGE)
                {
                    players->base_game_state = GAME_OVER_STAGE;
                    break;
                }
            }
            else
            {
                if (players->current_player_turn == PLAYER1_TURN || players->current_player_turn == PLAYER3_TURN)
                {
                    players->current_player_turn++;
                }
                else if (players->current_player_turn == PLAYER2_TURN)
                {
                    players->current_player_turn = PLAYER1_TURN;
                }
                else
                {
                    players->current_player_turn = PLAYER3_TURN;
                }
            }
            players->base_game_state = REBUTTAL_ATTEMPT2_STAGE;
            players->current_turn_state = players->human[players->current_player_turn] ? AIM_TARGET : STARTED_SHOT;
            break;
        }
        default:
            break;
        }
    }
    else
    {
        players->current_turn_state++;
    }
}

void players_complete_shot(players_t *players, player_shot_state_t shot_state, int cups_made)
{
    cstrl_da_int_push_back(&players->metrics[players->current_player_turn].shot_history, shot_state);
    players->metrics[players->current_player_turn].attempted_shots++;
    players->metrics[players->current_player_turn].cups_made += cups_made;
    if (cups_made > 0)
    {
        players->metrics[players->current_player_turn].successful_shots++;
        if (players->current_player_turn < PLAYER3_TURN)
        {
            players->team1_cups_remaining -= cups_made;
        }
        else
        {
            players->team2_cups_remaining -= cups_made;
        }
    }
}

void players_reset(players_t *players)
{
    players->current_player_turn = PLAYER1_TURN;
    players->current_turn_state = players->human[PLAYER1_TURN] ? AIM_TARGET : STARTED_SHOT;
    players->team1_cups_remaining = 10;
    players->team2_cups_remaining = 10;
    players->base_game_state = EYE_TO_EYE_STAGE;
}
