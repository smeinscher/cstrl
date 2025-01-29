//
// Created by sterling on 7/5/24.
//

#include "cstrl/cstrl_camera.h"

#include <stdlib.h>

CSTRL_API cstrl_camera *cstrl_camera_create(int viewport_width, int viewport_height, bool is_orthographic)
{
    cstrl_camera *new_camera = malloc(sizeof(cstrl_camera));

    new_camera->is_orthographic = is_orthographic;
    new_camera->fov = 45.0f * cstrl_pi_180;
    new_camera->viewport.x = viewport_width;
    new_camera->viewport.y = viewport_height;
    new_camera->forward = (vec3){0.0f, 0.0f, -1.0f};
    new_camera->up = (vec3){0.0f, 1.0f, 0.0f};
    new_camera->right = (vec3){1.0f, 0.0f, 0.0f};
    new_camera->view = cstrl_mat4_identity();
    new_camera->projection = cstrl_mat4_identity();
    new_camera->position = (vec3){0.0f, 0.0f, 0.0f};

    return new_camera;
}

CSTRL_API void cstrl_camera_free(cstrl_camera *camera)
{
    free(camera);
}

CSTRL_API void cstrl_camera_update(cstrl_camera *camera, cstrl_camera_direction_mask movement,
                                   cstrl_camera_direction_mask rotation)
{
    vec3 velocity = {0.0f, 0.0f, 0.0f};
    if (movement & CSTRL_CAMERA_DIRECTION_UP)
    {
        velocity = cstrl_vec3_add(velocity, camera->forward);
    }
    if (movement & CSTRL_CAMERA_DIRECTION_DOWN)
    {
        velocity = cstrl_vec3_sub(velocity, camera->forward);
    }
    if (movement & CSTRL_CAMERA_DIRECTION_LEFT)
    {
        velocity =
            cstrl_vec3_sub(velocity, cstrl_vec3_normalize(cstrl_vec3_cross(camera->forward, (vec3){0.0f, 1.0f, 0.0f})));
    }
    if (movement & CSTRL_CAMERA_DIRECTION_RIGHT)
    {
        velocity =
            cstrl_vec3_add(velocity, cstrl_vec3_normalize(cstrl_vec3_cross(camera->forward, (vec3){0.0f, 1.0f, 0.0f})));
    }
    velocity = cstrl_vec3_normalize(velocity);
    velocity = cstrl_vec3_mult_scalar(velocity, 0.1f);
    camera->position = cstrl_vec3_add(camera->position, velocity);

    vec3 rotation_velocity = {0.0f, 0.0f, 0.0f};
    if (rotation & CSTRL_CAMERA_DIRECTION_UP)
    {
        rotation_velocity.y -= 1.0f;
    }
    if (rotation & CSTRL_CAMERA_DIRECTION_DOWN)
    {
        rotation_velocity.y += 1.0f;
    }
    if (rotation & CSTRL_CAMERA_DIRECTION_LEFT)
    {
        rotation_velocity.x -= 1.0f;
    }
    if (rotation & CSTRL_CAMERA_DIRECTION_RIGHT)
    {
        rotation_velocity.x += 1.0f;
    }
    cstrl_camera_first_person_rotate(camera, rotation_velocity.x * 0.01f, rotation_velocity.y * 0.01f);

    camera->view = cstrl_mat4_look_at(camera->position, cstrl_vec3_add(camera->position, camera->forward), camera->up);

    if (!camera->is_orthographic)
    {
        camera->projection =
            cstrl_mat4_perspective(camera->fov, (float)camera->viewport.x / (float)camera->viewport.y, 0.1f, 100.0f);
    }
    else
    {
        camera->projection =
            cstrl_mat4_ortho(0.0f, (float)camera->viewport.x, (float)camera->viewport.y, 0.0f, 0.1f, 100.0f);
    }
}

CSTRL_API void cstrl_camera_first_person_rotate(cstrl_camera *camera, float change_y_axis, float change_x_axis)
{
    if (change_y_axis == 0.0f && change_x_axis == 0.0f)
    {
        return;
    }
    quat h = cstrl_quat_from_euler_angles((vec3){0.0f, -change_y_axis, 0.0f});
    camera->forward = cstrl_vec3_rotate_by_quat(camera->forward, h);
    camera->forward = cstrl_vec3_normalize(camera->forward);
    quat v = cstrl_quat_angle_axis(-change_x_axis, cstrl_vec3_cross(camera->forward, (vec3){0.0f, 1.0f, 0.0f}));
    camera->forward = cstrl_vec3_rotate_by_quat(camera->forward, v);
    camera->forward = cstrl_vec3_normalize(camera->forward);
    quat forward_up_rotation = cstrl_quat_from_to(camera->forward, (vec3){0.0f, 1.0f, 0.0f});
    float angle = cstrl_quat_get_angle(forward_up_rotation);
    if (angle > cstrl_pi - cstrl_pi_4 || angle < cstrl_pi_4)
    {
        camera->forward = cstrl_vec3_rotate_by_quat(camera->forward, cstrl_quat_conjugate(v));
    }
}

CSTRL_API void cstrl_camera_set_rotation(cstrl_camera *camera, quat rotation)
{
    camera->forward = cstrl_vec3_rotate_by_quat(camera->forward, rotation);
    camera->forward = cstrl_vec3_normalize(camera->forward);
}
