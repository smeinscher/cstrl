#include "cozy_random.h"
#include "cstrl/cstrl_util.h"

cstrl_mt_rand_state_t state;

void cozy_random_init(int seed)
{
    cstrl_rand_init(&state, seed);
}

int cozy_random_int(int min, int max)
{
    return cstrl_rand_uint32_range(&state, min, max);
}

float cozy_random_float(float min, float max)
{
    return cstrl_rand_float_range(&state, min, max);
}
