#include "cstrl/cstrl_util.h"
#include "log.c/log.h"

CSTRL_API void cstrl_rand_init(cstrl_mt_rand_state_t *state, uint32_t seed)
{
    state->array[0] = seed;

    for (int i = 0; i < CSTRL_RAND_N; i++)
    {
        seed = CSTRL_RAND_F * (seed ^ (seed >> (CSTRL_RAND_W - 2))) + i;
        state->array[i] = seed;
    }
    state->index = 0;
}

CSTRL_API uint32_t cstrl_rand_uint32(cstrl_mt_rand_state_t *state)
{
    int k = state->index;

    int j = k - (CSTRL_RAND_N - 1);
    if (j < 0)
    {
        j += CSTRL_RAND_N;
    }

    uint32_t x = (state->array[k] & CSTRL_RAND_UMASK) | (state->array[j] & CSTRL_RAND_LMASK);

    uint32_t x_a = x >> 1;
    if (x & 0x00000001UL)
    {
        x_a ^= CSTRL_RAND_A;
    }

    j = k - (CSTRL_RAND_N - CSTRL_RAND_M);
    if (j < 0)
    {
        j += CSTRL_RAND_N;
    }

    x = state->array[j] ^ x_a;
    state->array[k++] = x;

    if (k >= CSTRL_RAND_N)
    {
        k = 0;
    }
    state->index = k;

    uint32_t y = x ^ (x >> CSTRL_RAND_U);
    y = y ^ ((y << CSTRL_RAND_S) & CSTRL_RAND_B);
    y = y ^ ((y << CSTRL_RAND_T) & CSTRL_RAND_C);
    uint32_t z = y ^ (y >> CSTRL_RAND_L);

    return z;
}

CSTRL_API uint32_t cstrl_rand_uint32_range(cstrl_mt_rand_state_t *state, uint32_t min, uint32_t max)
{
    return min + cstrl_rand_uint32(state) % (max - min + 1);
}

CSTRL_API float cstrl_rand_float(cstrl_mt_rand_state_t *state)
{
    return (float)cstrl_rand_uint32(state) / (float)0xffffffff;
}

CSTRL_API float cstrl_rand_float_range(cstrl_mt_rand_state_t *state, float min, float max)
{
    return min + (cstrl_rand_float(state) * (max - min));
}
