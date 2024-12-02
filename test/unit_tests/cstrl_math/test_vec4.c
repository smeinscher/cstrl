//
// Created by 12105 on 11/22/2024.
//

#include "test_vec4.h"

#include "../../test_manager/test_types.h"
#include "cstrl/cstrl_math.h"

int test_cstrl_vec4_length()
{
    vec4 test_vec4 = {0.0f, 0.0f, 0.0f, 1.0f};
    float expected = 1.0f;
    float result = cstrl_vec4_length(test_vec4);

    expect_float_to_be(expected, result);

    return 1;
}

int test_cstrl_vec4_normalize()
{
    vec4 test_vec4 = {0.0f, 0.0f, 0.0f, 1.0f};
    vec4 expected = {0.0f, 0.0f, 0.0f, 1.0f};
    vec4 result = cstrl_vec4_normalize(test_vec4);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);
    expect_float_to_be(expected.w, result.w);

    return 1;
}

int test_cstrl_vec4_add()
{
    vec4 test_vec4_a = {1.0f, 0.0f, 0.0f, 0.0f};
    vec4 test_vec4_b = {0.0f, 1.0f, 0.0f, 0.0f};
    vec4 expected = {1.0f, 1.0f, 0.0f, 0.0f};
    vec4 result = cstrl_vec4_add(test_vec4_a, test_vec4_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);
    expect_float_to_be(expected.w, result.w);

    return 1;
}

int test_cstrl_vec4_sub()
{
    vec4 test_vec4_a = {0.0f, 0.0f, 0.0f, 1.0f};
    vec4 test_vec4_b = {0.0f, 0.0f, 0.0f, 1.0f};
    vec4 expected = {0.0f, 0.0f, 0.0f, 0.0f};
    vec4 result = cstrl_vec4_sub(test_vec4_a, test_vec4_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_vec4_mult_scalar()
{
    vec4 test_vec4 = {1.0f, 1.0f, 1.0f, 1.0f};
    float scalar = 2.0f;
    vec4 expected = {2.0f, 2.0f, 2.0f, 2.0f};
    vec4 result = cstrl_vec4_mult_scalar(test_vec4, scalar);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);
    expect_float_to_be(expected.w, result.w);

    return 1;
}

int test_cstrl_vec4_mult()
{
    vec4 test_vec4_a = {1.0f, 1.0f, 1.0f, 1.0f};
    vec4 test_vec4_b = {0.0f, 0.0f, 0.0f, 2.0f};
    vec4 expected = {0.0f, 0.0f, 0.0f, 2.0f};
    vec4 result = cstrl_vec4_mult(test_vec4_a, test_vec4_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);
    expect_float_to_be(expected.w, result.w);

    return 1;
}

int test_cstrl_vec4_div_scalar()
{
    vec4 test_vec4_a = {2.0f, 2.0f, 2.0f, 2.0f};
    float scalar = 2.0f;
    vec4 expected = {1.0f, 1.0f, 1.0f, 1.0f};
    vec4 result = cstrl_vec4_div_scalar(test_vec4_a, scalar);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);
    expect_float_to_be(expected.w, result.w);

    return 1;
}

int test_cstrl_vec4_div()
{
    vec4 test_vec4_a = {4.0f, 4.0f, 4.0f, 4.0f};
    vec4 test_vec4_b = {2.0f, 2.0f, 2.0f, 2.0f};
    vec4 expected = {2.0f, 2.0f, 2.0f, 2.0f};
    vec4 result = cstrl_vec4_div(test_vec4_a, test_vec4_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);
    expect_float_to_be(expected.w, result.w);

    return 1;
}

int test_cstrl_vec4_dot()
{
    vec4 test_vec4_a = {1.0f, 1.0f, 1.0f, 1.0f};
    vec4 test_vec4_b = {0.0f, 0.0f, 0.0f, 1.0f};
    float expected = 1.0f;
    float result = cstrl_vec4_dot(test_vec4_a, test_vec4_b);

    expect_float_to_be(expected, result);

    return 1;
}
