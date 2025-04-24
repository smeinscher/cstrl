#include "cup.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_util.h"

bool cups_init(cups_t *cups)
{
    float cup_start_x0 = 285.0f;
    float cup_start_x1 = 995.0f;
    float cup_start_y = 302.0f;
    float cup_x_offset = 25.0f;

    cups->position[0] = (vec2){cup_start_x0 + CUP_SIZE / 2.0f, cup_start_y + CUP_SIZE / 2.0f};
    cups->position[1] = (vec2){cup_start_x0 + CUP_SIZE / 2.0f, cup_start_y + CUP_SIZE / 2.0f * 3.0f};
    cups->position[2] = (vec2){cup_start_x0 + CUP_SIZE / 2.0f, cup_start_y + CUP_SIZE / 2.0f * 5.0f};
    cups->position[3] = (vec2){cup_start_x0 + CUP_SIZE / 2.0f, cup_start_y + CUP_SIZE / 2.0f * 7.0f};
    cups->position[4] = (vec2){cup_start_x0 + CUP_SIZE / 2.0f + cup_x_offset, cup_start_y + CUP_SIZE};
    cups->position[5] = (vec2){cup_start_x0 + CUP_SIZE / 2.0f + cup_x_offset, cup_start_y + CUP_SIZE * 2.0f};
    cups->position[6] = (vec2){cup_start_x0 + CUP_SIZE / 2.0f + cup_x_offset, cup_start_y + CUP_SIZE * 3.0f};
    cups->position[7] =
        (vec2){cup_start_x0 + CUP_SIZE / 2.0f + cup_x_offset * 2.0f, cup_start_y + CUP_SIZE / 2.0f * 3.0f};
    cups->position[8] =
        (vec2){cup_start_x0 + CUP_SIZE / 2.0f + cup_x_offset * 2.0f, cup_start_y + CUP_SIZE / 2.0f * 5.0f};
    cups->position[9] = (vec2){cup_start_x0 + CUP_SIZE / 2.0f + cup_x_offset * 3.0f, cup_start_y + CUP_SIZE * 2.0f};

    cups->position[10] = (vec2){cup_start_x1 - CUP_SIZE / 2.0f, cup_start_y + CUP_SIZE / 2.0f};
    cups->position[11] = (vec2){cup_start_x1 - CUP_SIZE / 2.0f, cup_start_y + CUP_SIZE / 2.0f * 3.0f};
    cups->position[12] = (vec2){cup_start_x1 - CUP_SIZE / 2.0f, cup_start_y + CUP_SIZE / 2.0f * 5.0f};
    cups->position[13] = (vec2){cup_start_x1 - CUP_SIZE / 2.0f, cup_start_y + CUP_SIZE / 2.0f * 7.0f};
    cups->position[14] = (vec2){cup_start_x1 - CUP_SIZE / 2.0f - cup_x_offset, cup_start_y + CUP_SIZE};
    cups->position[15] = (vec2){cup_start_x1 - CUP_SIZE / 2.0f - cup_x_offset, cup_start_y + CUP_SIZE * 2.0f};
    cups->position[16] = (vec2){cup_start_x1 - CUP_SIZE / 2.0f - cup_x_offset, cup_start_y + CUP_SIZE * 3.0f};
    cups->position[17] =
        (vec2){cup_start_x1 - CUP_SIZE / 2.0f - cup_x_offset * 2.0f, cup_start_y + CUP_SIZE / 2.0f * 3.0f};
    cups->position[18] =
        (vec2){cup_start_x1 - CUP_SIZE / 2.0f - cup_x_offset * 2.0f, cup_start_y + CUP_SIZE / 2.0f * 5.0f};
    cups->position[19] = (vec2){cup_start_x1 - CUP_SIZE / 2.0f - cup_x_offset * 3.0f, cup_start_y + CUP_SIZE * 2.0f};

    cstrl_da_int_init(&cups->freed, 10);

    return true;
}

int cups_shot_test(cups_t *cups, vec2 hit_position)
{
    for (int i = 0; i < TOTAL_CUP_COUNT; i++)
    {
        if (cstrl_vec2_length(cstrl_vec2_sub(cups->position[i], hit_position)) < CUP_SIZE / 3.0f)
        {
            if (cstrl_da_int_find_first(&cups->freed, i) != CSTRL_DA_INT_ITEM_NOT_FOUND)
            {
                return -1;
            }
            cstrl_da_int_push_back(&cups->freed, i);
            return i;
        }
    }
    return -1;
}

void cups_free(cups_t *cups)
{
    cstrl_da_int_free(&cups->freed);
}
