//
// Created by sterling on 11/30/24.
//

#include "test_camera.h"

#include "../../test_manager/test_types.h"
#include "renderer/camera.h"

int test_camera_process_mouse_movement()
{
    camera *camera = cstrl_camera_create(800, 600);
    cstrl_camera_rotate(camera, 0, 0);
    cstrl_camera_update(camera, false, false, false, false, false, false, false, false);
    quat rotation_expected = {1.0f, 0.0f, 0.0f, 0.0f};
    quat rotation_result = camera->transform.rotation;

    expect_float_to_be(rotation_expected.w, rotation_result.w);
    expect_float_to_be(rotation_expected.x, rotation_result.x);
    expect_float_to_be(rotation_expected.y, rotation_result.y);
    expect_float_to_be(rotation_expected.z, rotation_result.z);

    cstrl_camera_rotate(camera, 0, 2000);
    cstrl_camera_update(camera, false, false, false, false, false, false, false, false);
    rotation_expected = cstrl_quat_from_euler_angles((vec3){-69.0f * (cstrl_pi / 180.0f), 0.0f, 0.0f});
    rotation_result = camera->transform.rotation;

    expect_float_to_be(rotation_expected.w, rotation_result.w);
    expect_float_to_be(rotation_expected.x, rotation_result.x);
    expect_float_to_be(rotation_expected.y, rotation_result.y);
    expect_float_to_be(rotation_expected.z, rotation_result.z);

    // log_trace("%f, %f, %f, %f", rotation_result.w, rotation_result.x, rotation_result.y, rotation_result.z);
    camera->transform.rotation = (quat){1.0f, 0.0f, 0.0f, 0.0f};

    cstrl_camera_rotate(camera, 0, -2000);
    cstrl_camera_update(camera, false, false, false, false, false, false, false, false);
    rotation_expected = cstrl_quat_from_euler_angles((vec3){69.0f * (cstrl_pi / 180.0f), 0.0f, 0.0f});
    rotation_result = camera->transform.rotation;

    expect_float_to_be(rotation_expected.w, rotation_result.w);
    expect_float_to_be(rotation_expected.x, rotation_result.x);
    expect_float_to_be(rotation_expected.y, rotation_result.y);
    expect_float_to_be(rotation_expected.z, rotation_result.z);

    camera->transform.rotation = (quat){1.0f, 0.0f, 0.0f, 0.0f};

    cstrl_camera_rotate(camera, 0.2f, 0.0f);
    cstrl_camera_update(camera, false, false, false, false, false, false, false, false);
    rotation_expected = cstrl_quat_from_euler_angles((vec3){0.0f, -0.2f, 0.0f});
    rotation_result = camera->transform.rotation;

    expect_float_to_be(rotation_expected.w, rotation_result.w);
    expect_float_to_be(rotation_expected.x, rotation_result.x);
    expect_float_to_be(rotation_expected.y, rotation_result.y);
    expect_float_to_be(rotation_expected.z, rotation_result.z);

    mat4 view_expected = (mat4){0.980067f, 0.0f, -0.198669f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.198669f, 0,    0.980067,   0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    mat4 view_result = camera->view;

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

    cstrl_camera_free(camera);

    return 1;
}
