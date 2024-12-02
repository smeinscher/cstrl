//
// Created by 12105 on 11/22/2024.
//

#include "test_vec3.h"

#include "../../test_manager/test_types.h"
#include "cstrl/cstrl_math.h"

int test_cstrl_vec3_length()
{
    vec3 test_vec3 = {0.0f, 0.0f, 1.0f};
    float expected = 1.0f;
    float result = cstrl_vec3_length(test_vec3);

    expect_float_to_be(expected, result);

    return 1;
}

int test_cstrl_vec3_normalize()
{
    vec3 test_vec3 = {0.0f, 0.0f, 1.0f};
    vec3 expected = {0.0f, 0.0f, 1.0f};
    vec3 result = cstrl_vec3_normalize(test_vec3);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_vec3_add()
{
    vec3 test_vec3_a = {1.0f, 0.0f, 0.0f};
    vec3 test_vec3_b = {0.0f, 1.0f, 0.0f};
    vec3 expected = {1.0f, 1.0f, 0.0f};
    vec3 result = cstrl_vec3_add(test_vec3_a, test_vec3_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_vec3_sub()
{
    vec3 test_vec3_a = {1.0f, 0.0f, 0.0f};
    vec3 test_vec3_b = {1.0f, 0.0f, 0.0f};
    vec3 expected = {0.0f, 0.0f, 0.0f};
    vec3 result = cstrl_vec3_sub(test_vec3_a, test_vec3_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_vec3_mul_scalar()
{
    vec3 test_vec3 = {1.0f, 1.0f, 1.0f};
    float scalar = 2.0f;
    vec3 expected = {2.0f, 2.0f, 2.0f};
    vec3 result = cstrl_vec3_mult_scalar(test_vec3, scalar);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_vec3_mul()
{
    vec3 test_vec3_a = {1.0f, 2.0f, 3.0f};
    vec3 test_vec3_b = {0.0f, 0.0f, 0.0f};
    vec3 expected = {0.0f, 0.0f, 0.0f};
    vec3 result = cstrl_vec3_mult(test_vec3_a, test_vec3_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_vec3_div_scalar()
{
    vec3 test_vec3 = {2.0f, 4.0f, 8.0f};
    float scalar = 2.0f;
    vec3 expected = {1.0f, 2.0f, 4.0f};
    vec3 result = cstrl_vec3_div_scalar(test_vec3, scalar);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_vec3_div()
{
    vec3 test_vec3_a = {2.0f, 4.0f, 8.0f};
    vec3 test_vec3_b = {2.0f, 2.0f, 2.0f};
    vec3 expected = {1.0f, 2.0f, 4.0f};
    vec3 result = cstrl_vec3_div(test_vec3_a, test_vec3_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_vec3_dot()
{
    vec3 test_vec3_a = {1.0f, 2.0f, 3.0f};
    vec3 test_vec3_b = {1.0f, 0.0f, 0.0f};
    float expected = 1.0f;
    float result = cstrl_vec3_dot(test_vec3_a, test_vec3_b);

    expect_float_to_be(expected, result);

    return 1;
}

int test_cstrl_vec3_cross()
{
    vec3 test_vec3_a = {1.0f, 2.0f, 3.0f};
    vec3 test_vec3_b = {4.0f, 5.0f, 6.0f};
    vec3 expected = {-3.0f, 6.0f, -3.0f};
    vec3 result = cstrl_vec3_cross(test_vec3_a, test_vec3_b);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_vec3_rotate_by_quat()
{
    vec3 test_vec3 = {0.0f, 0.0f, -1.0f};
    quat test_quat = {1.0f, 0.0f, 0.0f, 0.0f};
    vec3 expected = {0.0f, 0.0f, -1.0f};
    vec3 result = cstrl_vec3_rotate_by_quat(test_vec3, test_quat);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_euler_angles_from_quat()
{
    quat test_quat = {1.0f, 1.0f, 0.0f, 0.0f};
    vec3 expected = {cstrl_pi_2, 0.0f, 0.0f};
    vec3 result = cstrl_euler_angles_from_quat(test_quat);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    test_quat = cstrl_quat_normalize(test_quat);
    result = cstrl_euler_angles_from_quat(test_quat);
    quat quat_result = cstrl_quat_from_euler_angles(result);

    expect_float_to_be(test_quat.w, quat_result.w);
    expect_float_to_be(test_quat.x, quat_result.x);
    expect_float_to_be(test_quat.y, quat_result.y);
    expect_float_to_be(test_quat.z, quat_result.z);

    return 1;
}
