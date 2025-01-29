#include "test_mat4.h"
#include "../../test_manager/test_types.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_math.h"

int test_cstrl_mat4_inverse()
{
    mat4 m = cstrl_mat4_identity();
    mat4 expected = cstrl_mat4_identity();
    mat4 result = cstrl_mat4_inverse(m);

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

    m.yx = 2;
    m.zy = 1;
    m.wz = 2;
    expected =
        (mat4){1.0f, -2.0f, 2.0f, -4.0f, 0.0f, 1.0f, -1.0f, 2.0f, 0.0f, 0.0f, 1.0f, -2.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    result = cstrl_mat4_inverse(m);

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

    m = (mat4){1.810660f, 0.0f, 0.0f,       0.0f,  0.0f, 2.414213f, 0.0f,     0.0f,
               0.0f,      0.0f, -1.002002f, -1.0f, 0.0f, 0.0f,      -0.2002f, 0.0f};
    expected = (mat4){0.552285f, 0.0f, 0.0f, 0.0f,       0.0f, 0.414214f, 0.0f,  0.0f,
                      0.0f,      0.0f, 0.0f, -4.995005f, 0.0f, 0.0f,      -1.0f, 5.005005f};
    result = cstrl_mat4_inverse(m);

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

    m = cstrl_mat4_identity();
    m.zw = -1.0f;
    expected = cstrl_mat4_identity();
    expected.zw = 1.0f;
    result = cstrl_mat4_inverse(m);

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

    cstrl_camera *camera = cstrl_camera_create(800, 600, false);
    camera->forward = (vec3){1.0f, 0.0f, 0.0f};
    camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(camera->forward, camera->up));
    camera->position = (vec3){-3.0f, 0.0f, 0.0f};
    cstrl_camera_update(camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    expected = cstrl_mat4_identity();
    expected = (mat4){0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, -3.0f, 0.0f, 0.0f, 1.0f};
    result = cstrl_mat4_inverse(camera->view);
    cstrl_camera_free(camera);

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

int test_cstrl_mat4_affine_inverse()
{
    mat4 m = cstrl_mat4_identity();
    mat4 expected = cstrl_mat4_identity();
    mat4 result = cstrl_mat4_affine_inverse(m);

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

    cstrl_camera *camera = cstrl_camera_create(800, 600, true);
    camera->position = (vec3){0.0f, 5.0f, 5.0f};
    camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(camera->position));
    camera->right = cstrl_vec3_cross(camera->forward, camera->up);
    cstrl_camera_update(camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    expected = (mat4){1.0f, 0.0f,     0.0f,     0.0f, 0.0f, 0.707107, -0.707107, 0.0f,
                      0.0f, 0.707107, 0.707107, 0.0f, 0.0f, 5.0f,     5.0f,      1.0f};
    result = cstrl_mat4_affine_inverse(camera->view);
    cstrl_camera_free(camera);

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

    m = (mat4){1.81066f, 0.0f, 0.0f,       0.0f,  0.0f, 2.414213f, 0.0f,    0.0f,
               0.0f,     0.0f, -1.002002f, -1.0f, 0.0f, 0.0f,      -0.2002, 0.0f};
    expected = (mat4){0.552284f, 0.0f, 0.0f, 0.0f,  0.0f, 0.414214, 0.0f,  0.0f,
                      0.0f,      0.0f, 0.0f, -5.0f, 0.0f, 0.0f,     -1.0f, 5.0f};
    result = cstrl_mat4_affine_inverse(m);

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

int test_cstrl_mat4_perspective()
{
    mat4 expected = {1.810660, 0.0f, 0.0f,          0.0f,  0.0f, 2.414213f, 0.0f,     0.0f,
                     0.0f,     0.0f, -1.002002002f, -1.0f, 0.0f, 0.0f,      -0.2002f, 0.0f};
    mat4 result = cstrl_mat4_perspective(cstrl_pi_4, 800.0f / 600.0f, 0.1f, 100.0f);

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

int test_cstrl_mat4_look_at()
{
    vec3 eye = {-3.0f, 0.0f, 0.0f};
    vec3 forward = {1.0f, 0.0f, 0.0f};
    vec3 up = {0.0f, 1.0f, 0.0f};
    mat4 result = cstrl_mat4_look_at(eye, cstrl_vec3_add(eye, forward), up);

    return 1;
}
