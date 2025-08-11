//
// Created by sterling on 7/5/24.
//

#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_math.h"

#include <stdlib.h>

CSTRL_API cstrl_camera *cstrl_camera_create(float viewport_width, float viewport_height, bool is_orthographic)
{
    cstrl_camera *new_camera = malloc(sizeof(cstrl_camera));

    new_camera->is_orthographic = is_orthographic;
    new_camera->fov = 45.0f * cstrl_pi_180;
    new_camera->near = 0.1f;
    new_camera->far = 100.0f;
    new_camera->speed = 1.0f;
    new_camera->viewport.x = viewport_width;
    new_camera->viewport.y = viewport_height;
    new_camera->offset.x = 0.0f;
    new_camera->offset.y = 0.0f;
    new_camera->forward = (vec3){0.0f, 0.0f, -1.0f};
    new_camera->up = (vec3){0.0f, 1.0f, 0.0f};
    new_camera->right = (vec3){1.0f, 0.0f, 0.0f};
    new_camera->view = cstrl_mat4_identity();
    new_camera->projection = cstrl_mat4_identity();
    new_camera->position = (vec3){0.0f, 0.0f, 1.0f};

    cstrl_camera_update(new_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);

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
        if (!camera->is_orthographic)
        {
            velocity = cstrl_vec3_add(velocity, camera->forward);
        }
        else
        {
            velocity = cstrl_vec3_sub(velocity, camera->up);
        }
    }
    if (movement & CSTRL_CAMERA_DIRECTION_DOWN)
    {
        if (!camera->is_orthographic)
        {
            velocity = cstrl_vec3_sub(velocity, camera->forward);
        }
        else
        {
            velocity = cstrl_vec3_add(velocity, camera->up);
        }
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
    velocity = cstrl_vec3_mult_scalar(velocity, camera->speed);
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
            cstrl_mat4_perspective(camera->fov, camera->viewport.x / camera->viewport.y, camera->near, camera->far);
    }
    else
    {
        camera->projection = cstrl_mat4_ortho(camera->offset.x, camera->viewport.x, camera->viewport.y,
                                              camera->offset.y, camera->near, camera->far);
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

CSTRL_API void cstrl_camera_rotate_around_point(cstrl_camera *camera, vec3 point, float vertical_angle_change,
                                                float horizontal_angle_change)
{
    vec3 camera_focus = cstrl_vec3_sub(camera->position, point);
    mat4 up_yaw_matrix = cstrl_mat4_identity();
    up_yaw_matrix = cstrl_mat4_rotate(up_yaw_matrix, -vertical_angle_change, camera->up);
    mat4 right_pitch_matrix = cstrl_mat4_identity();
    right_pitch_matrix = cstrl_mat4_rotate(right_pitch_matrix, -horizontal_angle_change, camera->right);
    vec4 position_vec4 = (vec4){camera->position.x, camera->position.y, camera->position.z, 1.0f};
    camera_focus =
        cstrl_vec4_to_vec3(cstrl_vec4_mult_mat4(position_vec4, cstrl_mat4_mult(up_yaw_matrix, right_pitch_matrix)));
    camera->position = cstrl_vec3_add(point, camera_focus);
    camera->forward = cstrl_vec3_normalize(cstrl_vec3_negate(camera->position));
    camera->right = cstrl_vec3_normalize(cstrl_vec3_cross(camera->forward, camera->up));
    cstrl_camera_update(camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
}

CSTRL_API quat cstrl_camera_get_rotation(cstrl_camera *camera)
{
    return cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(camera->view));
}
