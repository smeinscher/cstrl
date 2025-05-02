#include "cup.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_util.h"

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
}

void cups_free(cups_t *cups)
{
    cstrl_da_int_free(&cups->freed);
}
