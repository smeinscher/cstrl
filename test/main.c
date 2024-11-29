//
// Created by 12105 on 11/22/2024.
//

#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include "cstrl_math/test_quat.h"
#include "cstrl_math/test_vec2.h"
#include "cstrl_math/test_vec3.h"
#include "cstrl_math/test_vec4.h"
#include "log.c/log.h"
#include "renderer/camera.h"
#include "test_manager.h"
#include "util/test_dynamic_array.h"

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

void key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case CSTRL_KEY_ESCAPE:
        if (action == CSTRL_RELEASE_KEY)
        {
            cstrl_platform_set_should_exit(true);
        }
        break;
    case CSTRL_KEY_W:
        if (action == CSTRL_PRESS_KEY)
        {
            camera_set_moving_up(true);
        }
        else if (action == CSTRL_RELEASE_KEY)
        {
            camera_set_moving_up(false);
        }
        break;
    case CSTRL_KEY_S:
        if (action == CSTRL_PRESS_KEY)
        {
            camera_set_moving_down(true);
        }
        else if (action == CSTRL_RELEASE_KEY)
        {
            camera_set_moving_down(false);
        }
        break;
    case CSTRL_KEY_A:
        if (action == CSTRL_PRESS_KEY)
        {
            camera_set_moving_left(true);
        }
        else if (action == CSTRL_RELEASE_KEY)
        {
            camera_set_moving_left(false);
        }
        break;
    case CSTRL_KEY_D:
        if (action == CSTRL_PRESS_KEY)
        {
            camera_set_moving_right(true);
        }
        else if (action == CSTRL_RELEASE_KEY)
        {
            camera_set_moving_right(false);
        }
        break;
    case CSTRL_KEY_R:
        if (action == CSTRL_PRESS_KEY)
        {
            camera_set_position((vec3){0.0f, 0.0f, 5.0f});
            camera_set_rotation(cstrl_quat_from_euler_angles((vec3){-4.5f, 7.6f, 0}));
        }
        else if (action == CSTRL_RELEASE_KEY)
        {
        }
        break;
    default:
        break;
    }
}

int last_x = 400;
int last_y = 300;

void mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos)
{
    if (last_x == -1 || last_y == -1)
    {
        last_x = xpos;
        last_y = ypos;
        return;
    }
    int offset_x = xpos - last_x;
    int offset_y = last_y - ypos;

    if (false)
    {
        last_x = xpos;
        last_y = ypos;
    }

    camera_process_mouse_movement(offset_x, offset_y);
}

int main()
{
    // vec2_tests();
    // vec3_tests();
    // vec4_tests();

    // dynamic_int_array_tests();

    // quat_tests();

    // test_manager_log_total_failed_tests();

    // return 0;
    cstrl_platform_state state;
    if (!cstrl_platform_init(&state, "cstrl window test", 560, 240, 800, 600))
    {
        cstrl_platform_destroy(&state);
        return 1;
    }

    cstrl_platform_set_key_callback(&state, key_callback);
    cstrl_platform_set_mouse_position_callback(&state, mouse_position_callback);

    cstrl_renderer_init(&state);
    render_data *render_data = cstrl_renderer_create_render_data();
    float vertices_old[] = {
        -0.5f, -0.5f, 0.0f, // left
        0.5f,  -0.5f, 0.0f, // right
        0.0f,  0.5f,  0.0f  // top
    };
    float vertices[] = {-0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f,
                        0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,

                        -0.5f, -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,
                        0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, -0.5f, 0.5f,

                        -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f, -0.5f,
                        -0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,

                        0.5f,  0.5f,  0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f,
                        0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,  0.5f,

                        -0.5f, -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, 0.5f,
                        0.5f,  -0.5f, 0.5f,  -0.5f, -0.5f, 0.5f,  -0.5f, -0.5f, -0.5f,

                        -0.5f, 0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  0.5f,
                        0.5f,  0.5f,  0.5f,  -0.5f, 0.5f,  0.5f,  -0.5f, 0.5f,  -0.5f};
    float uvs[] = {0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

                   0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,

                   1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

                   1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

                   0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,

                   0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    cstrl_renderer_add_positions(render_data, vertices, 3, 36);
    cstrl_renderer_add_uvs(render_data, uvs);

    camera_set_position((vec3){0.0f, 0.0f, 5.0f});

    camera_set_rotation(cstrl_quat_from_euler_angles((vec3){0.0f, 0.0f, 0.0f}));
    while (!cstrl_platform_should_exit(&state))
    {
        cstrl_platform_pump_messages(&state);
        cstrl_renderer_clear(0.1f, 0.2f, 0.4f, 1.0f);
        cstrl_renderer_draw(render_data);
        cstrl_renderer_swap_buffers(&state);
    }

    cstrl_renderer_destroy(&state);
    cstrl_platform_destroy(&state);

    return 0;
}