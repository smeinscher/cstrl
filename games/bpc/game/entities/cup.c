#include "cup.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>

bool cups_init(cups_t *cups, bool overtime)
{
    float cup_start_x0 = 48.0f;
    float cup_start_x1 = 272.0f;
    float cup_start_y = 74.0f;
    float cup_x_offset = 7.0f;
    float cup_y_offset = 1.0f;

    cups->position[0] = (vec2){48.0f + CUP_SIZE / 2.0f, 74.0f + CUP_SIZE / 2.0f};
    cups->position[1] = (vec2){48.0f + CUP_SIZE / 2.0f, 82.0f + CUP_SIZE / 2.0f};
    cups->position[2] = (vec2){48.0f + CUP_SIZE / 2.0f, 90.0f + CUP_SIZE / 2.0f};
    cups->position[3] = (vec2){48.0f + CUP_SIZE / 2.0f, 98.0f + CUP_SIZE / 2.0f};
    cups->position[4] = (vec2){55.0f + CUP_SIZE / 2.0f, 78.0f + CUP_SIZE / 2.0f};
    cups->position[5] = (vec2){55.0f + CUP_SIZE / 2.0f, 86.0f + CUP_SIZE / 2.0f};
    cups->position[6] = (vec2){55.0f + CUP_SIZE / 2.0f, 94.0f + CUP_SIZE / 2.0f};
    cups->position[7] = (vec2){62.0f + CUP_SIZE / 2.0f, 82.0f + CUP_SIZE / 2.0f};
    cups->position[8] = (vec2){62.0f + CUP_SIZE / 2.0f, 90.0f + CUP_SIZE / 2.0f};
    cups->position[9] = (vec2){69.0f + CUP_SIZE / 2.0f, 86.0f + CUP_SIZE / 2.0f};

    cups->position[10] = (vec2){273.0f - CUP_SIZE / 2.0f, 74.0f + CUP_SIZE / 2.0f};
    cups->position[11] = (vec2){273.0f - CUP_SIZE / 2.0f, 82.0f + CUP_SIZE / 2.0f};
    cups->position[12] = (vec2){273.0f - CUP_SIZE / 2.0f, 90.0f + CUP_SIZE / 2.0f};
    cups->position[13] = (vec2){273.0f - CUP_SIZE / 2.0f, 98.0f + CUP_SIZE / 2.0f};
    cups->position[14] = (vec2){266.0f - CUP_SIZE / 2.0f, 78.0f + CUP_SIZE / 2.0f};
    cups->position[15] = (vec2){266.0f - CUP_SIZE / 2.0f, 86.0f + CUP_SIZE / 2.0f};
    cups->position[16] = (vec2){266.0f - CUP_SIZE / 2.0f, 94.0f + CUP_SIZE / 2.0f};
    cups->position[17] = (vec2){259.0f - CUP_SIZE / 2.0f, 82.0f + CUP_SIZE / 2.0f};
    cups->position[18] = (vec2){259.0f - CUP_SIZE / 2.0f, 90.0f + CUP_SIZE / 2.0f};
    cups->position[19] = (vec2){252.0f - CUP_SIZE / 2.0f, 86.0f + CUP_SIZE / 2.0f};

    cstrl_da_int_init(&cups->freed, 10);

    for (int i = 0; i < TOTAL_CUP_COUNT; i++)
    {
        cups->active[i] = true;
    }
    if (overtime)
    {
        cstrl_da_int_push_back(&cups->freed, 0);
        cstrl_da_int_push_back(&cups->freed, 1);
        cstrl_da_int_push_back(&cups->freed, 2);
        cstrl_da_int_push_back(&cups->freed, 3);

        cstrl_da_int_push_back(&cups->freed, 10);
        cstrl_da_int_push_back(&cups->freed, 11);
        cstrl_da_int_push_back(&cups->freed, 12);
        cstrl_da_int_push_back(&cups->freed, 13);

        cups->active[0] = false;
        cups->active[1] = false;
        cups->active[2] = false;
        cups->active[3] = false;

        cups->active[10] = false;
        cups->active[11] = false;
        cups->active[12] = false;
        cups->active[13] = false;
    }

    return true;
}

int cups_shot_test(cups_t *cups, float ball_size, vec2 hit_position, float *distance_from_center)
{
    float min_length = cstrl_infinity;
    int selected_cup = -1;
    for (int i = 0; i < TOTAL_CUP_COUNT; i++)
    {
        float length = cstrl_vec2_length(cstrl_vec2_sub(cups->position[i], hit_position));
        if (length <= CUP_SIZE / 2.0f + ball_size / 2.0f)
        {
            if (cstrl_da_int_find_first(&cups->freed, i) != CSTRL_DA_INT_ITEM_NOT_FOUND)
            {
                continue;
            }
            if (length < min_length)
            {
                min_length = length;
                selected_cup = i;
            }
        }
    }
    *distance_from_center = min_length;
    return selected_cup;
}

void cups_make(cups_t *cups, int cup_id)
{
    cstrl_da_int_push_back(&cups->freed, cup_id);
    cups->active[cup_id] = false;
}

int cups_count_active_by_team(cups_t *cups, int team)
{
    int count = 0;
    for (int i = (team == 0 ? 0 : 10); i < (team == 0 ? 10 : 20); i++)
    {
        if (cups->active[i])
        {
            count++;
        }
    }
    return count;
}

bool cups_can_rerack(cups_t *cups, int team)
{
    int count = cups_count_active_by_team(cups, team);
    if (count != 1 && count != 3 && count != 4 && count != 6)
    {
        return false;
    }
    int start = team == 0 ? 0 : 10;
    switch (count)
    {
    case 1:
        return !cups->active[start + 9];
    case 3:
        return !(cups->active[start + 9] && cups->active[start + 8] && cups->active[start + 7]);
    case 4:
        return !(cups->active[start + 9] && cups->active[start + 8] && cups->active[start + 7] &&
                 cups->active[start + 5]);
    case 6:
        return !(cups->active[start + 9] && cups->active[start + 8] && cups->active[start + 7] &&
                 cups->active[start + 6] && cups->active[start + 5] && cups->active[start + 4]);
    default:
        return false;
    }
    return true;
}

bool cups_rerack(cups_t *cups, int team)
{
    int count = cups_count_active_by_team(cups, team);
    if (!cups_can_rerack(cups, team))
    {
        printf("Can't rerack, count: %d\n", count);
        return false;
    }
    for (int i = (team == 0 ? 0 : 10); i < (team == 0 ? 10 : 20); i++)
    {
        if (cstrl_da_int_find_first(&cups->freed, i) == CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_push_back(&cups->freed, i);
            cups->active[i] = false;
        }
    }
    switch (count)
    {
    case 1: {
        cups->active[team == 0 ? 9 : 19] = true;
        int index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 9 : 19);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        break;
    }
    case 3: {
        cups->active[team == 0 ? 9 : 19] = true;
        cups->active[team == 0 ? 8 : 18] = true;
        cups->active[team == 0 ? 7 : 17] = true;
        int index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 9 : 19);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 8 : 18);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 7 : 17);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        break;
    }
    case 4: {
        cups->active[team == 0 ? 9 : 19] = true;
        cups->active[team == 0 ? 8 : 18] = true;
        cups->active[team == 0 ? 7 : 17] = true;
        cups->active[team == 0 ? 5 : 15] = true;
        int index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 9 : 19);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 8 : 18);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 7 : 17);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 5 : 15);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        break;
    }
    case 6: {
        cups->active[team == 0 ? 9 : 19] = true;
        cups->active[team == 0 ? 8 : 18] = true;
        cups->active[team == 0 ? 7 : 17] = true;
        cups->active[team == 0 ? 6 : 16] = true;
        cups->active[team == 0 ? 5 : 15] = true;
        cups->active[team == 0 ? 4 : 14] = true;
        int index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 9 : 19);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 8 : 18);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 7 : 17);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 6 : 16);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 5 : 15);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        index = cstrl_da_int_find_first(&cups->freed, team == 0 ? 4 : 14);
        if (index != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            cstrl_da_int_remove(&cups->freed, index);
        }
        break;
    }
    default:
        break;
    }
    return true;
}

void cups_free(cups_t *cups)
{
    cstrl_da_int_free(&cups->freed);
}
