//
// Created by sterling on 7/5/24.
//

#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "log.c/log.h"

#include <stdlib.h>

cstrl_camera *cstrl_camera_create(int viewport_width, int viewport_height, bool is_orthographic)
{
    cstrl_camera *new_camera = malloc(sizeof(cstrl_camera));

    new_camera->is_orthographic = is_orthographic;
    new_camera->fov = 45.0f * cstrl_pi_180;
    new_camera->viewport.x = viewport_width;
    new_camera->viewport.y = viewport_height;
    new_camera->forward = (vec3){0.0f, 0.0f, -1.0f};
    new_camera->view = cstrl_mat4_identity();
    new_camera->projection = cstrl_mat4_identity();
    new_camera->transform.position = (vec3){0.0f, 0.0f, 0.0f};
    new_camera->transform.rotation = cstrl_quat_identity();
    new_camera->transform.scale = (vec3){1.0f, 1.0f, 1.0f};

    return new_camera;
}

void cstrl_camera_free(cstrl_camera *camera)
{
    free(camera);
}

void cstrl_camera_update(cstrl_camera *camera, bool moving_up, bool moving_down, bool moving_left, bool moving_right,
                         bool turning_up, bool turning_down, bool turning_left, bool turning_right)
{
    vec3 forward = cstrl_vec3_rotate_by_quat((vec3){0.0f, 0.0f, -1.0f}, camera->transform.rotation);
    forward = cstrl_vec3_normalize(forward);
    forward = camera->forward;
    vec3 velocity = {0.0f, 0.0f, 0.0f};
    if (moving_up)
    {
        velocity = cstrl_vec3_add(velocity, forward);
        // velocity.y -= 1.0f;
    }
    if (moving_down)
    {
        velocity = cstrl_vec3_sub(velocity, forward);
        // velocity.y += 1.0f;
    }
    if (moving_left)
    {
        velocity = cstrl_vec3_sub(velocity, cstrl_vec3_normalize(cstrl_vec3_cross(forward, (vec3){0.0f, 1.0f, 0.0f})));
        // velocity.x -= 1.0f;
    }
    if (moving_right)
    {
        velocity = cstrl_vec3_add(velocity, cstrl_vec3_normalize(cstrl_vec3_cross(forward, (vec3){0.0f, 1.0f, 0.0f})));
        // velocity.x += 1.0f;
    }
    velocity = cstrl_vec3_normalize(velocity);
    velocity = cstrl_vec3_mult_scalar(velocity, 0.1f);
    camera->transform.position = cstrl_vec3_add(camera->transform.position, velocity);

    vec3 rotation_velocity = {0.0f, 0.0f, 0.0f};
    if (turning_up)
    {
        rotation_velocity.y -= 1.0f;
    }
    if (turning_down)
    {
        rotation_velocity.y += 1.0f;
    }
    if (turning_left)
    {
        rotation_velocity.x -= 1.0f;
    }
    if (turning_right)
    {
        rotation_velocity.x += 1.0f;
    }
    cstrl_camera_rotate(camera, rotation_velocity.x * 0.01f, rotation_velocity.y * 0.01f);

    vec3 up = {0.0f, 1.0f, 0.0f};
    camera->view =
        cstrl_mat4_look_at(camera->transform.position, cstrl_vec3_add(camera->transform.position, forward), up);

    // float left = (float)g_viewport_width - (float)g_viewport_width / (g_zoom_factor / g_zoom);
    // float right = (float)g_viewport_width / (g_zoom_factor / g_zoom);
    // float bottom = (float)g_viewport_height / (g_zoom_factor / g_zoom);
    // float top = (float)g_viewport_height - (float)g_viewport_height / (g_zoom_factor / g_zoom);
    // g_projection = cstrl_ortho(left, right, bottom, top, 0.1f, 1000.0f);
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
    // g_projection = cstrl_ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
}

void cstrl_camera_rotate(cstrl_camera *camera, float change_y_axis, float change_x_axis)
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

void cstrl_camera_set_rotation(cstrl_camera *camera, quat rotation)
{
    camera->forward = cstrl_vec3_rotate_by_quat(camera->forward, rotation);
    camera->forward = cstrl_vec3_normalize(camera->forward);
}
