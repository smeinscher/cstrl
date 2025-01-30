#include "test_mat3.h"
#include "../../test_manager/test_types.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_math.h"

int test_cstrl_mat3_inverse()
{
    mat3 m = (mat3){0.552285f, 0.0f, 0.0f, 0.0f, 0.414214f, 0.0f, 0.0f, 0.0f, -0.998002};
    mat3 expected = (mat3){1.810660f, 0.0f, 0.0f, 0.0f, 2.414211f, 0.0f, 0.0f, 0.0f, -1.002002};
    mat3 result = cstrl_mat3_inverse(m);

    expect_float_to_be(expected.xx, result.xx);
    expect_float_to_be(expected.xy, result.xy);
    expect_float_to_be(expected.xz, result.xz);

    expect_float_to_be(expected.yx, result.yx);
    expect_float_to_be(expected.yy, result.yy);
    expect_float_to_be(expected.yz, result.yz);

    expect_float_to_be(expected.zx, result.zx);
    expect_float_to_be(expected.zy, result.zy);
    expect_float_to_be(expected.zz, result.zz);

    m = (mat3){1.81066f, 0.0f, 0.0f, 0.0f, 2.414213f, 0.0f, 0.0f, 0.0f, -1.002002f};
    expected = (mat3){0.552284f, 0.0f, 0.0f, 0.0f, 0.414214, 0.0f, 0.0f, 0.0f, -0.998002f};
    result = cstrl_mat3_inverse(m);

    expect_float_to_be(expected.xx, result.xx);
    expect_float_to_be(expected.xy, result.xy);
    expect_float_to_be(expected.xz, result.xz);

    expect_float_to_be(expected.yx, result.yx);
    expect_float_to_be(expected.yy, result.yy);
    expect_float_to_be(expected.yz, result.yz);

    expect_float_to_be(expected.zx, result.zx);
    expect_float_to_be(expected.zy, result.zy);
    expect_float_to_be(expected.zz, result.zz);

    return 1;
}

int test_cstrl_mat3_determinant()
{
    mat3 m = (mat3){1.81066f, 0.0f, 0.0f, 0.0f, 2.414213f, 0.0f, 0.0f, 0.0f, -1.002002f};
    float expected = -4.38007;
    float result = cstrl_mat3_determinant(m);

    expect_float_to_be(expected, result);

    return 1;
}

int test_cstrl_mat3_adjugate()
{
    mat3 m = (mat3){1.81066f, 0.0f, 0.0f, 0.0f, 2.414213f, 0.0f, 0.0f, 0.0f, -1.002002f};
    mat3 expected = (mat3){-2.419046252226, 0.0f, 0.0f, 0.0f, -1.81428494132, 0.0f, 0.0f, 0.0f, 4.37131891058};
    mat3 result = cstrl_mat3_adjugate(m);

    expect_float_to_be(expected.xx, result.xx);
    expect_float_to_be(expected.xy, result.xy);
    expect_float_to_be(expected.xz, result.xz);

    expect_float_to_be(expected.yx, result.yx);
    expect_float_to_be(expected.yy, result.yy);
    expect_float_to_be(expected.yz, result.yz);

    expect_float_to_be(expected.zx, result.zx);
    expect_float_to_be(expected.zy, result.zy);
    expect_float_to_be(expected.zz, result.zz);

    return 1;
}

int test_cstrl_mat3_orthogonal_to_quat()
{
    cstrl_camera *camera = cstrl_camera_create(800, 600, false);

    camera->forward = (vec3){0.0f, 0.0f, -3.0f};
    camera->right = cstrl_vec3_cross(camera->forward, camera->up);
    cstrl_camera_update(camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    quat result = cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(camera->view));
    printf("result: %f, %f, %f, %f\n", result.w, result.x, result.y, result.z);

    return 1;
}
