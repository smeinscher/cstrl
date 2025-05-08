#include "../../test_manager/test_types.h"
#include "cstrl/cstrl_util.h"

int test_cstrl_rand_uint32()
{
    cstrl_mt_rand_state_t rand_state;
    cstrl_rand_init(&rand_state, 42);

    uint32_t expected = 1853429358;
    uint32_t result = cstrl_rand_uint32(&rand_state);

    expect_int_to_be(expected, result);

    return 1;
}

int test_cstrl_rand_uint32_range()
{
    cstrl_mt_rand_state_t rand_state;
    cstrl_rand_init(&rand_state, 42);

    uint32_t expected = 3;
    uint32_t result = cstrl_rand_uint32_range(&rand_state, 0, 4);

    expect_int_to_be(expected, result);

    return 1;
}

int test_cstrl_rand_float()
{
    cstrl_mt_rand_state_t rand_state;
    cstrl_rand_init(&rand_state, 42);

    float expected = 0.431535f;
    float result = cstrl_rand_float(&rand_state);

    expect_float_to_be(expected, result);

    return 1;
}

int test_cstrl_rand_float_range()
{
    cstrl_mt_rand_state_t rand_state;
    cstrl_rand_init(&rand_state, 42);

    float expected = 1.726141f;
    float result = cstrl_rand_float_range(&rand_state, 0.0f, 4.0f);

    expect_float_to_be(expected, result);

    return 1;
}
