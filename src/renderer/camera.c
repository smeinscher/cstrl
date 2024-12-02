//
// Created by sterling on 7/5/24.
//

#include "camera.h"

#include "cstrl/cstrl_platform.h"
#include "log.c/log.h"

#include <stdlib.h>

camera *cstrl_camera_create(int viewport_width, int viewport_height)
{
    camera *new_camera = malloc(sizeof(camera));

    new_camera->fov = 45.0f * cstrl_pi_180;
    new_camera->viewport.x = viewport_width;
    new_camera->viewport.y = viewport_height;
    new_camera->view = cstrl_mat4_identity();
    new_camera->projection = cstrl_mat4_identity();
    new_camera->transform.position = (vec3){0.0f, 0.0f, 0.0f};
    new_camera->transform.rotation = cstrl_quat_identity();
    new_camera->transform.scale = (vec3){1.0f, 1.0f, 1.0f};

    return new_camera;
}

void cstrl_camera_free(camera *camera)
{
    free(camera);
}

void cstrl_camera_update(camera *camera, bool moving_up, bool moving_down, bool moving_left, bool moving_right,
                         bool turning_up, bool turning_down, bool turning_left, bool turning_right)
{
    vec3 forward = cstrl_vec3_rotate_by_quat((vec3){0.0f, 0.0f, -1.0f}, camera->transform.rotation);
    forward = cstrl_vec3_normalize(forward);
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
    camera->projection =
        cstrl_mat4_perspective(camera->fov, (float)camera->viewport.x / (float)camera->viewport.y, 0.1f, 100.0f);
    // g_projection = cstrl_ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
}

void cstrl_camera_rotate(camera *camera, float change_x, float change_y)
{
    if (change_x == 0.0f && change_y == 0.0f)
    {
        return;
    }
    vec3 euler = (vec3){0.0f, 0.0f, 0.0f};
    euler.x -= change_y;
    euler.y -= change_x;

    camera->transform.rotation = cstrl_quat_mult(camera->transform.rotation, cstrl_quat_from_euler_angles(euler));
    float max_angle = 45.0f * cstrl_pi_180;
    vec3 rotation_euler = cstrl_euler_angles_from_quat(camera->transform.rotation);
    if (rotation_euler.x > max_angle)
    {
        // rotation_euler.x = max_angle;
        // camera->transform.rotation = cstrl_quat_from_euler_angles(rotation_euler);
    }
    else if (rotation_euler.x < -max_angle)
    {
        // rotation_euler.x = -max_angle;
        // camera->transform.rotation = cstrl_quat_from_euler_angles(rotation_euler);
    }
    camera->transform.rotation = cstrl_quat_normalize(camera->transform.rotation);
    log_trace("%f, %f, %f", rotation_euler.x, rotation_euler.y, rotation_euler.z);
}
