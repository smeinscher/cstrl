#include "ai.h"
#include "cstrl/cstrl_util.h"

static void move_in_circle(ai_t *ai, int ai_id)
{
}

void ai_init(ai_t *ai)
{
    for (int i = 0; i < MAX_AI_COUNT; i++)
    {
        if (!ai->active[i])
        {
            continue;
        }
        // TODO: make random
        ai->objective[i] = GO_IN_CIRCLES;
    }
}

void ai_update(ai_t *ai)
{
    for (int i = 0; i < MAX_AI_COUNT; i++)
    {
        if (!ai->active[i])
        {
            continue;
        }

        switch (ai->objective[i])
        {
            case GO_IN_CIRCLES:
                move_in_circle(ai, i);
                break;
            default:
                break;
        }
    }
}

void ai_remove_unit(ai_t *ai, int ai_id, int unit_id)
{

}

