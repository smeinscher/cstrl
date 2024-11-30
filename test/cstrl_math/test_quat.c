//
// Created by 12105 on 11/27/2024.
//

#include "test_quat.h"

#include "../test_types.h"
#include "cstrl/cstrl_math.h"

int test_cstrl_quat_get_axis()
{
    quat q = {1.0f, 1.0f, 1.0f, 1.0f};
    float val = 1.0f / sqrtf(3.0f);
    vec3 expected = {val, val, val};
    vec3 result = cstrl_quat_get_axis(q);

    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_quat_get_angle()
{
    quat q = {1.0f, 1.0f, 1.0f, 1.0f};
    float expected = 0.0f;
    float result = cstrl_quat_get_angle(q);

    expect_float_to_be(expected, result);

    return 1;
}

int test_cstrl_quat_add()
{
    quat a = {1.0f, 1.0f, 1.0f, 1.0f};
    quat b = {1.0f, 1.0f, 1.0f, 1.0f};
    quat expected = {2.0f, 2.0f, 2.0f, 2.0f};
    quat result = cstrl_quat_add(a, b);

    expect_float_to_be(expected.w, result.w);
    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_quat_mult()
{
    quat a = {1.0f, 1.0f, 1.0f, 1.0f};
    quat b = {1.0f, 1.0f, 1.0f, 1.0f};
    quat expected = {-2.0f, 2.0f, 2.0f, 2.0f};
    quat result = cstrl_quat_mult(a, b);

    expect_float_to_be(expected.w, result.w);
    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    return 1;
}

int test_cstrl_quat_to_mat4()
{
    quat q = {1.0f, 1.0f, 1.0f, 1.0f};
    mat4 expected = {1, -1, -1, -1, 1, 1, -1, 1, 1, 1, 1, -1, 1, -1, 1, 1};
    mat4 result = cstrl_quat_to_mat4(q);

    expect_float_to_be(expected.xx, result.xx);
    expect_float_to_be(expected.xy, result.xy);
    expect_float_to_be(expected.xz, result.xz);
    expect_float_to_be(expected.xw, result.xw);

    expect_float_to_be(expected.yx, result.yx);
    expect_float_to_be(expected.yy, result.yy);
    expect_float_to_be(expected.yz, result.yz);
    expect_float_to_be(expected.yw, result.yw);

    expect_float_to_be(expected.zx, result.zx);
    expect_float_to_be(expected.zy, result.zy);
    expect_float_to_be(expected.zz, result.zz);
    expect_float_to_be(expected.zw, result.zw);

    expect_float_to_be(expected.wx, result.wx);
    expect_float_to_be(expected.wy, result.wy);
    expect_float_to_be(expected.wz, result.wz);
    expect_float_to_be(expected.ww, result.ww);

    return 1;
}

int test_cstrl_quat_from_euler_angles()
{
    vec3 euler = {cstrl_pi_2, 0.0f, 0.0f};
    quat expected = {1.0f, 1.0f, 0.0f, 0.0f};
    expected = cstrl_quat_normalize(expected);
    quat result = cstrl_quat_from_euler_angles(euler);

    expect_float_to_be(expected.w, result.w);
    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    euler = (vec3){0.0f, cstrl_pi_2, 0.0f};
    expected = (quat){1.0f, 0.0f, 1.0f, 0.0f};
    expected = cstrl_quat_normalize(expected);
    result = cstrl_quat_from_euler_angles(euler);

    expect_float_to_be(expected.w, result.w);
    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    euler = (vec3){0.0f, 0.0f, cstrl_pi_2};
    expected = (quat){1.0f, 0.0f, 0.0f, 1.0f};
    expected = cstrl_quat_normalize(expected);
    result = cstrl_quat_from_euler_angles(euler);

    expect_float_to_be(expected.w, result.w);
    expect_float_to_be(expected.x, result.x);
    expect_float_to_be(expected.y, result.y);
    expect_float_to_be(expected.z, result.z);

    euler = (vec3){0.0f, 0.0f, cstrl_pi_2};
    result = cstrl_quat_from_euler_angles(euler);
    vec3 euler_result = cstrl_euler_angles_from_quat(result);

    expect_float_to_be(euler.x, euler_result.x);
    expect_float_to_be(euler.y, euler_result.y);
    expect_float_to_be(euler.z, euler_result.z);

    return 1;
}
