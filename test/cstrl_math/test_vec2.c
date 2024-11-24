//
// Created by 12105 on 11/22/2024.
//

#include "test_vec2.h"

#include "../test_types.h"
#include <cstrl/cstrl_math.h>

int test_cstrl_vec2_length()
{
    vec2 test_vec2 = {0.0f, 1.0f};
    float expected = 1.0f;
    float result = cstrl_vec2_length(test_vec2);

    expect_float_to_be(expected, result);

    return 1;
}

int test_cstrl_vec2_normalize()
{
    vec2 test_vec2 = {0.0f, 1.0f};
    vec2 expected = {0.0f, 1.0f};
    vec2 result = cstrl_vec2_normalize(test_vec2);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);

    return 1;
}

int test_cstrl_vec2_add()
{
    vec2 test_vec2_a = {1.0f, 0.0f};
    vec2 test_vec2_b = {0.0f, 1.0f};
    vec2 expected = {1.0f, 1.0f};
    vec2 result = cstrl_vec2_add(test_vec2_a, test_vec2_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);

    return 1;
}

int test_cstrl_vec2_sub()
{
    vec2 test_vec2_a = {1.0f, 0.0f};
    vec2 test_vec2_b = {1.0f, 0.0f};
    vec2 expected = {0.0f, 0.0f};
    vec2 result = cstrl_vec2_sub(test_vec2_a, test_vec2_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);

    return 1;
}

int test_cstrl_vec2_mult_scalar()
{
    vec2 test_vec2 = {1.0f, 1.0f};
    float scalar = 2.0f;
    vec2 expected = {2.0f, 2.0f};
    vec2 result = cstrl_vec2_mult_scalar(test_vec2, scalar);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);

    return 1;
}

int test_cstrl_vec2_mult()
{
    vec2 test_vec2_a = {1.0f, 1.0f};
    vec2 test_vec2_b = {0.0f, 0.0f};
    vec2 expected = {0.0f, 0.0f};
    vec2 result = cstrl_vec2_mult(test_vec2_a, test_vec2_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);

    return 1;
}

int test_cstrl_vec2_div_scalar()
{
    vec2 test_vec2 = {2.0f, 4.0f};
    float scalar = 2.0f;
    vec2 expected = {1.0f, 2.0f};
    vec2 result = cstrl_vec2_div_scalar(test_vec2, scalar);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);

    return 1;
}

int test_cstrl_vec2_div()
{
    vec2 test_vec2_a = {2.0f, 4.0f};
    vec2 test_vec2_b = {2.0f, 4.0f};
    vec2 expected = {1.0f, 1.0f};
    vec2 result = cstrl_vec2_div(test_vec2_a, test_vec2_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);

    return 1;
}

int test_cstrl_vec2_dot()
{
    vec2 test_vec2_a = {1.0f, 1.0f};
    vec2 test_vec2_b = {2.0f, 0.0f};
    float expected = 2.0f;
    float result = cstrl_vec2_dot(test_vec2_a, test_vec2_b);

    expect_float_to_be(expected, result);

    return 1;
}
