//
// Created by sterling on 11/30/24.
//

#include "test_camera.h"

#include "../test_types.h"
#include "renderer/camera.h"

int test_camera_process_mouse_movement()
{
    camera_process_mouse_movement(0, 0);
    camera_update();
    quat rotation_expected = {1.0f, 0.0f, 0.0f, 0.0f};
    quat rotation_result = camera_get_rotation();

    expect_float_to_be(rotation_expected.w, rotation_result.w);
    expect_float_to_be(rotation_expected.x, rotation_result.x);
    expect_float_to_be(rotation_expected.y, rotation_result.y);
    expect_float_to_be(rotation_expected.z, rotation_result.z);

    camera_process_mouse_movement(0, 2000);
    camera_update();
    rotation_expected = cstrl_quat_from_euler_angles((vec3){69.0f * (cstrl_pi / 180.0f), 0.0f, 0.0f});
    rotation_result = camera_get_rotation();

    expect_float_to_be(rotation_expected.w, rotation_result.w);
    expect_float_to_be(rotation_expected.x, rotation_result.x);
    expect_float_to_be(rotation_expected.y, rotation_result.y);
    expect_float_to_be(rotation_expected.z, rotation_result.z);

    // log_trace("%f, %f, %f, %f", rotation_result.w, rotation_result.x, rotation_result.y, rotation_result.z);
    camera_set_rotation((quat){1.0f, 0.0f, 0.0f, 0.0f});

    camera_process_mouse_movement(0, -2000);
    camera_update();
    rotation_expected = cstrl_quat_from_euler_angles((vec3){-69.0f * (cstrl_pi / 180.0f), 0.0f, 0.0f});
    rotation_result = camera_get_rotation();

    expect_float_to_be(rotation_expected.w, rotation_result.w);
    expect_float_to_be(rotation_expected.x, rotation_result.x);
    expect_float_to_be(rotation_expected.y, rotation_result.y);
    expect_float_to_be(rotation_expected.z, rotation_result.z);

    camera_set_rotation((quat){1.0f, 0.0f, 0.0f, 0.0f});

    camera_process_mouse_movement(200, 0);
    camera_update();
    rotation_expected = cstrl_quat_from_euler_angles((vec3){0.0f, -0.2f, 0.0f});
    rotation_result = camera_get_rotation();

    expect_float_to_be(rotation_expected.w, rotation_result.w);
    expect_float_to_be(rotation_expected.x, rotation_result.x);
    expect_float_to_be(rotation_expected.y, rotation_result.y);
    expect_float_to_be(rotation_expected.z, rotation_result.z);

    mat4 view_expected = (mat4){0.980067f, 0.0f, -0.198669f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.198669f, 0,    0.980067,   0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    mat4 view_result = camera_get_view();

    log_trace("%f, %f, %f, %f", view_result.xx, view_result.yx, view_result.zx, view_result.wx);
    log_trace("%f, %f, %f, %f", view_result.xy, view_result.yy, view_result.zy, view_result.wy);
    log_trace("%f, %f, %f, %f", view_result.xz, view_result.yz, view_result.zz, view_result.wz);
    log_trace("%f, %f, %f, %f", view_result.xw, view_result.yw, view_result.zw, view_result.ww);

    expect_float_to_be(view_expected.xx, view_result.xx);
    expect_float_to_be(view_expected.xy, view_result.xy);
    expect_float_to_be(view_expected.xz, view_result.xz);
    expect_float_to_be(view_expected.xw, view_result.xw);

    expect_float_to_be(view_expected.yx, view_result.yx);
    expect_float_to_be(view_expected.yy, view_result.yy);
    expect_float_to_be(view_expected.yz, view_result.yz);
    expect_float_to_be(view_expected.yw, view_result.yw);

    expect_float_to_be(view_expected.zx, view_result.zx);
    expect_float_to_be(view_expected.zy, view_result.zy);
    expect_float_to_be(view_expected.zz, view_result.zz);
    expect_float_to_be(view_expected.zw, view_result.zw);

    expect_float_to_be(view_expected.wx, view_result.wx);
    expect_float_to_be(view_expected.wy, view_result.wy);
    expect_float_to_be(view_expected.wz, view_result.wz);
    expect_float_to_be(view_expected.ww, view_result.ww);

    return 1;
}
