//
// Created by 12105 on 12/1/2024.
//

#include "unit_tests.h"

#include "../test_manager/test_manager.h"
#include "cstrl_math/test_mat3.h"
#include "cstrl_math/test_mat4.h"
#include "cstrl_math/test_quat.h"
#include "cstrl_math/test_vec2.h"
#include "cstrl_math/test_vec3.h"
#include "cstrl_math/test_vec4.h"
#include "cstrl_physics/test_collision.h"
#include "cstrl_renderer/test_camera.h"
#include "cstrl_util/test_dynamic_array.h"
#include "cstrl_util/test_random.h"

void vec2_tests()
{
    int test_suite = test_manager_add_suite("vec2 Tests", "Suite for testing vec2 functions");
    test_manager_add_test(test_suite, test_cstrl_vec2_length, "vec2 Length Test", "Test if correct length is returned");
    test_manager_add_test(test_suite, test_cstrl_vec2_normalize, "vec2 Normalize Test",
                          "Test if correctly normalized vector is returned");
    test_manager_add_test(test_suite, test_cstrl_vec2_add, "vec2 Add Test", "Test if correctly adds vector to vector");
    test_manager_add_test(test_suite, test_cstrl_vec2_sub, "vec2 Subtract Test", "Test if correctly subtracts vectors");
    test_manager_add_test(test_suite, test_cstrl_vec2_mult_scalar, "vec2 Multiply (Scalar) Test",
                          "Test if correctly multiplies vector by scalar");
    test_manager_add_test(test_suite, test_cstrl_vec2_mult, "vec2 Multiply Test",
                          "Test if correctly multiplies vectors");
    test_manager_add_test(test_suite, test_cstrl_vec2_div_scalar, "vec2 Divide (Scalar) Test",
                          "Test if correctly divides vector by scalar");
    test_manager_add_test(test_suite, test_cstrl_vec2_div, "vec2 Divide Test", "Test if correctly multiplies vectors");
    test_manager_add_test(test_suite, test_cstrl_vec2_dot, "vec2 Dot Product Test",
                          "Test if correctly gets the dot product of two vectors");
    test_manager_run_tests(test_suite);
    test_manager_log_results(test_suite);
}

void vec3_tests()
{
    int test_suite = test_manager_add_suite("vec3 Tests", "Suite for testing vec3 functions");
    test_manager_add_test(test_suite, test_cstrl_vec3_length, "vec3 Length Test", "Test if correct length is returned");
    test_manager_add_test(test_suite, test_cstrl_vec3_normalize, "vec3 Normalize Test",
                          "Test if correctly normalized vector is returned");
    test_manager_add_test(test_suite, test_cstrl_vec3_add, "vec3 Add Test", "Test if correctly adds vector to vector");
    test_manager_add_test(test_suite, test_cstrl_vec3_sub, "vec3 Subtract Test", "Test if correctly subtracts vectors");
    test_manager_add_test(test_suite, test_cstrl_vec3_mul_scalar, "vec3 Multiply (Scalar) Test",
                          "Test if correctly multiplies vector by scalar");
    test_manager_add_test(test_suite, test_cstrl_vec3_mul, "vec3 Multiply Test",
                          "Test if correctly multiplies vectors");
    test_manager_add_test(test_suite, test_cstrl_vec3_div_scalar, "vec3 Divide (Scalar) Test",
                          "Test if correctly divides vector by scalar");
    test_manager_add_test(test_suite, test_cstrl_vec3_div, "vec3 Divide Test", "Test if correctly divides vectors");
    test_manager_add_test(test_suite, test_cstrl_vec3_dot, "vec3 Dot Product Test",
                          "Test if correctly gets the dot product of two vectors");
    test_manager_add_test(test_suite, test_cstrl_vec3_cross, "vec3 Cross Test",
                          "Test if correctly gets the cross vector");
    test_manager_add_test(test_suite, test_cstrl_vec3_rotate_by_quat, "vec3 quat Rotation Test",
                          "Test if correctly rotates vector by quaternion");
    test_manager_add_test(test_suite, test_cstrl_quat_to_euler_angles, "Euler Angles From quat Test",
                          "Test if correctly gets euler angles from quaternion");
    test_manager_run_tests(test_suite);
    test_manager_log_results(test_suite);
}

void vec4_tests()
{
    int test_suite = test_manager_add_suite("vec4 Tests", "Suite for testing vec4 functions");
    test_manager_add_test(test_suite, test_cstrl_vec4_length, "vec4 Length Test", "Test if correct length is returned");
    test_manager_add_test(test_suite, test_cstrl_vec4_normalize, "vec4 Normalize Test",
                          "Test if correct normalized vector is returned");
    test_manager_add_test(test_suite, test_cstrl_vec4_add, "vec4 Add Test", "Test if correctly adds vector to vector");
    test_manager_add_test(test_suite, test_cstrl_vec4_sub, "vec4 Subtract Test", "Test if correctly subtracts vectors");
    test_manager_add_test(test_suite, test_cstrl_vec4_mult_scalar, "vec4 Multiply (Scalar) Test",
                          "Tests if correctly multiplies vector by scalar");
    test_manager_add_test(test_suite, test_cstrl_vec4_mult, "vec4 Multiply Test",
                          "Test if correctly multiplies vectors");
    test_manager_add_test(test_suite, test_cstrl_vec4_div_scalar, "vec4 Divide (Scalar) Test",
                          "Test if correctly divides vector by scalar");
    test_manager_add_test(test_suite, test_cstrl_vec4_div, "vec4 Divide Test", "Test if correctly divides vectors");
    test_manager_add_test(test_suite, test_cstrl_vec4_dot, "vec4 Dot Product Test",
                          "Test if correctly gets the dot product of two vectors");
    test_manager_add_test(test_suite, test_cstrl_vec4_mult_mat4, "vec4 Multiply mat4",
                          "Test if correctly multiplies mat4x4 by column vec4");
    test_manager_run_tests(test_suite);
    test_manager_log_results(test_suite);
}

void quat_tests()
{
    int test_suite = test_manager_add_suite("quat Tests", "Suite for testing quat functions");
    test_manager_add_test(test_suite, test_cstrl_quat_get_axis, "quat Get Axis Test",
                          "Test if correctly gets axis of quaternion");
    test_manager_add_test(test_suite, test_cstrl_quat_add, "quat Get Angle Test",
                          "Test if correctly gets angle of quaternion");
    test_manager_add_test(test_suite, test_cstrl_quat_add, "quat Add Test", "Test if correctly adds quaternions");
    test_manager_add_test(test_suite, test_cstrl_quat_mult, "quat Multiply Test",
                          "Test if correctly multiplies quaternions");
    test_manager_add_test(test_suite, test_cstrl_quat_to_mat4, "quat to mat4 Test",
                          "Test if correctly convertes quaternion to 4 by 4 matrix");
    test_manager_add_test(test_suite, test_cstrl_quat_from_euler_angles, "quat From Euler Angles",
                          "Test if correctly converts euler angles to quaternion");
    test_manager_add_test(test_suite, test_cstrl_mat3_orthogonal_to_quat, "quat from mat3",
                          "Test if correctly converts matrix 3x3 to quaternion");
    test_manager_run_tests(test_suite);
    test_manager_log_results(test_suite);
}

void mat3_tests()
{
    int test_suite = test_manager_add_suite("mat3 Tests", "Suite for testing mat3 functions");
    test_manager_add_test(test_suite, test_cstrl_mat3_inverse, "mat3 Inverse Test",
                          "Test if correctly applies inverse to matrix 3x3");
    test_manager_add_test(test_suite, test_cstrl_mat3_determinant, "mat3 Determinant Test",
                          "Test if correctly calculates determinant of matrix 3x3");
    test_manager_add_test(test_suite, test_cstrl_mat3_adjugate, "mat3 Adjugate Test",
                          "Test if correctly calculates adjugate of matrix 3x3");
    test_manager_run_tests(test_suite);
    test_manager_log_results(test_suite);
}

void mat4_tests()
{
    int test_suite = test_manager_add_suite("mat4 Tests", "Suite for testing mat4 functions");
    test_manager_add_test(test_suite, test_cstrl_mat4_inverse, "mat4 Inverse Test",
                          "Test if correctly applies inverse to matrix 4x4");
    // test_manager_add_test(test_suite, test_cstrl_mat4_affine_inverse, "mat4 Affine Inverse Test",
    //"Test if correctly applies inverse to affine matrix 4x4");
    test_manager_add_test(test_suite, test_cstrl_mat4_perspective, "mat4 Perspective Test",
                          "Test if correctly applies perspective projection on matrix 4x4");
    test_manager_add_test(test_suite, test_cstrl_mat4_look_at, "mat4 Look At Test",
                          "Test if correctly applies look at calculation on matrix 4x4");
    test_manager_add_test(test_suite, test_cstrl_mat4_shear, "mat4 Shear Test",
                          "Test if correctly applies shear on matrix 4x4");
    test_manager_run_tests(test_suite);
    test_manager_log_results(test_suite);
}

void dynamic_int_array_tests()
{
    int test_suite = test_manager_add_suite("Dynamic Int Array Tests", "Suite for testing functions for da_int");
    test_manager_add_test(test_suite, test_cstrl_da_int_init_happy_path, "da_int Init Test",
                          "Test if correctly initializes da_int (happy path)");
    test_manager_add_test(test_suite, test_cstrl_da_int_init_zero_size, "da_int Init initial_size=0 Test",
                          "Test if initialization with 0 initial size results in expected behavior");
    test_manager_add_test(test_suite, test_cstrl_da_int_reserve_happy_path, "da_int Reserve Test",
                          "Test if da_int increase in capacity works as expected");
    test_manager_add_test(test_suite, test_cstrl_da_int_reserve_overflow_capacity, "da_int Reserve Overflow Test",
                          "Test that setting new capacity too high results in expected behavior");
    test_manager_add_test(test_suite, test_cstrl_da_int_push_back_happy_path, "da_int Push Back Test",
                          "Test if da_int push back works as expected");
    test_manager_add_test(test_suite, test_cstrl_da_int_push_back_breach_capacity,
                          "da_int Push Back Breach Capacity Test",
                          "Test that when push back triggers array resize the behavior is expected");
    test_manager_run_tests(test_suite);
    test_manager_log_results(test_suite);
}

void camera_tests()
{
    int test_suite = test_manager_add_suite("Camera Tests", "Suite for testing camera functions");
    test_manager_add_test(test_suite, test_camera_process_mouse_movement, "Camera Process Mouse Movement Test",
                          "Test if mouse movement results in expected camera behavior");
    test_manager_run_tests(test_suite);
    test_manager_log_results(test_suite);
}

void collision_tests()
{
    int test_suite = test_manager_add_suite("Collision Tests", "Suite for testing physics collision functions");
    test_manager_add_test(test_suite, test_cstrl_collision_aabb_tree_insert_leaf, "Collision Insert Leaf Node",
                          "Test if can correctly insert node into aabb collision tree");
    test_manager_run_tests(test_suite);
    test_manager_log_results(test_suite);
}

void random_tests()
{
    int test_suite = test_manager_add_suite("Random Test", "Suite for testing random functions");
    test_manager_add_test(test_suite, test_cstrl_rand_uint32, "Random uint32",
                          "Test if correctly produces random numbers based on seed");
    test_manager_add_test(test_suite, test_cstrl_rand_uint32_range, "Random uint32 in Range",
                          "Test if correctly produces random numbers in r range");
    test_manager_add_test(test_suite, test_cstrl_rand_float, "Random float",
                          "Test if correctly produces random floats");
    test_manager_add_test(test_suite, test_cstrl_rand_float_range, "Random float in Range",
                          "Test if correctly produces random floats in range");
    test_manager_run_tests(test_suite);
    test_manager_log_results(test_suite);
}

int run_unit_tests()
{
    // vec2_tests();
    // vec3_tests();
    // vec4_tests();
    // quat_tests();
    // mat3_tests();
    mat4_tests();
    // dynamic_int_array_tests();
    //
    // camera_tests();

    // collision_tests();

    // random_tests();

    test_manager_log_total_failed_tests();

    return test_manager_total_passed_tests();
}
