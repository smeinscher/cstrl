//
// Created by sterling on 7/5/24.
//

#include "camera.h"

#include "cstrl/cstrl_platform.h"
#include "log.c/log.h"

static int g_viewport_width = 1280;
static int g_viewport_height = 720;

static transform g_camera_transform;
static vec2 g_max_camera_position = {INFINITY, INFINITY};
static vec2 g_min_camera_position = {-INFINITY, -INFINITY};

static bool g_camera_moving_up = false;
static bool g_camera_moving_down = false;
static bool g_camera_moving_left = false;
static bool g_camera_moving_right = false;
static float g_camera_speed = 0.1f;

static float g_zoom_factor = 100.0f;
static float g_zoom = 100.0f;

static mat4 g_view;
static mat4 g_projection;

static vec3 g_forward = (vec3){0.0f, 0.0f, -1.0f};

void camera_update()
{
    vec3 velocity = {0.0f, 0.0f, 0.0f};
    if (g_camera_moving_up)
    {
        velocity = cstrl_vec3_add(velocity, g_forward);
    }
    if (g_camera_moving_down)
    {
        velocity = cstrl_vec3_sub(velocity, g_forward);
    }
    if (g_camera_moving_left)
    {
        velocity =
            cstrl_vec3_sub(velocity, cstrl_vec3_normalize(cstrl_vec3_cross(g_forward, (vec3){0.0f, 1.0f, 0.0f})));
    }
    if (g_camera_moving_right)
    {
        velocity =
            cstrl_vec3_add(velocity, cstrl_vec3_normalize(cstrl_vec3_cross(g_forward, (vec3){0.0f, 1.0f, 0.0f})));
    }
    velocity = cstrl_vec3_normalize(velocity);
    // velocity = cstrl_vec3_rotate_by_quat(velocity, g_camera_transform.rotation);
    // velocity = cstrl_vec3_normalize(velocity);
    velocity = cstrl_vec3_mult_scalar(velocity, g_camera_speed);
    velocity = cstrl_vec3_mult_scalar(velocity, camera_get_zoom_ratio());
    g_camera_transform.position = cstrl_vec3_add(g_camera_transform.position, velocity);

    if (g_camera_transform.position.x >= g_max_camera_position.x * camera_get_zoom_ratio())
    {
        g_camera_transform.position.x = g_max_camera_position.x * camera_get_zoom_ratio();
    }
    if (g_camera_transform.position.y >= g_max_camera_position.y * camera_get_zoom_ratio())
    {
        g_camera_transform.position.y = g_max_camera_position.y * camera_get_zoom_ratio();
    }
    if (g_camera_transform.position.x <= g_min_camera_position.x * camera_get_zoom_ratio())
    {
        g_camera_transform.position.x = g_min_camera_position.x * camera_get_zoom_ratio();
    }
    if (g_camera_transform.position.y <= g_min_camera_position.y * camera_get_zoom_ratio())
    {
        g_camera_transform.position.y = g_min_camera_position.y * camera_get_zoom_ratio();
    }
    g_forward = cstrl_vec3_rotate_by_quat((vec3){0.0f, 0.0f, -1.0f}, g_camera_transform.rotation);
    g_forward = cstrl_vec3_normalize(g_forward);
    vec3 right = cstrl_vec3_rotate_by_quat((vec3){1.0f, 0.0f, 0.0f}, cstrl_quat_inverse(g_camera_transform.rotation));
    vec3 up = {0.0f, 1.0f, 0.0f};
    vec3 camera_position_plus_forward = cstrl_vec3_add(g_camera_transform.position, g_forward);
    float cam_x = sin(cstrl_platform_get_absolute_time()) * 5.0f;
    float cam_z = cos(cstrl_platform_get_absolute_time()) * 5.0f;
    g_view =
        cstrl_mat4_look_at(g_camera_transform.position, cstrl_vec3_add(g_camera_transform.position, g_forward), up);

    // float left = (float)g_viewport_width - (float)g_viewport_width / (g_zoom_factor / g_zoom);
    // float right = (float)g_viewport_width / (g_zoom_factor / g_zoom);
    // float bottom = (float)g_viewport_height / (g_zoom_factor / g_zoom);
    // float top = (float)g_viewport_height - (float)g_viewport_height / (g_zoom_factor / g_zoom);
    // g_projection = cstrl_ortho(left, right, bottom, top, 0.1f, 1000.0f);
    g_projection = cstrl_mat4_perspective(0.785398f, (float)g_viewport_width / (float)g_viewport_height, 0.1f, 100.0f);
    // g_projection = cstrl_ortho(-1.0f, 1.0f, -1.0f, 1.0f, 0.1f, 100.0f);
}

void camera_set_moving_up(bool moving_up)
{
    g_camera_moving_up = moving_up;
}

void camera_set_moving_down(bool moving_down)
{
    g_camera_moving_down = moving_down;
}

void camera_set_moving_left(bool moving_left)
{
    g_camera_moving_left = moving_left;
}

void camera_set_moving_right(bool moving_right)
{
    g_camera_moving_right = moving_right;
}

float camera_get_zoom()
{
    return g_zoom;
}

float camera_get_zoom_factor()
{
    return g_zoom_factor;
}

float camera_get_zoom_ratio()
{
    return g_zoom_factor / g_zoom;
}

void camera_set_zoom(float zoom)
{
    g_zoom = zoom;
}

void camera_increment_zoom(float amount)
{
    g_zoom += amount;
}

void camera_decrement_zoom(float amount)
{
    g_zoom -= amount;
}

vec3 camera_get_position()
{
    return g_camera_transform.position;
}

void camera_set_position(vec3 position)
{
    g_camera_transform.position.x = position.x;
    g_camera_transform.position.y = position.y;
    g_camera_transform.position.z = position.z;
}

quat camera_get_rotation()
{
    return g_camera_transform.rotation;
}

void camera_set_rotation(quat rotation)
{
    rotation = cstrl_quat_normalize(rotation);
    g_camera_transform.rotation = rotation;
}

void camera_set_max_position(vec2 position)
{
    g_max_camera_position.x = position.x;
    g_max_camera_position.y = position.y;
}

void camera_set_min_position(vec2 position)
{
    g_min_camera_position.x = position.x;
    g_min_camera_position.y = position.y;
}

mat4 camera_get_projection()
{
    return g_projection;
}

mat4 camera_get_view()
{
    return g_view;
}

int camera_get_viewport_width()
{
    return g_viewport_width;
}

int camera_get_viewport_height()
{
    return g_viewport_height;
}

void camera_set_viewport_width(int viewport_width)
{
    g_viewport_width = viewport_width;
}

void camera_set_viewport_height(int viewport_height)
{
    g_viewport_height = viewport_height;
}

void camera_process_mouse_movement(int offset_x, int offset_y)
{
    float offset_x_float = (float)offset_x * 0.001f;
    float offset_y_float = (float)offset_y * 0.001f;

    vec3 euler = (vec3){0.0f, 0.0f, 0.0f};
    euler.x += offset_y_float;
    euler.y -= offset_x_float;

    float max_angle = 69.0f;
    vec3 rotation_euler = cstrl_euler_angles_from_quat(g_camera_transform.rotation);
    if (rotation_euler.x + euler.x > max_angle * (cstrl_pi / 180.0f))
    {
        euler.x = cstrl_max(max_angle * (cstrl_pi / 180.0f) - rotation_euler.x, 0.0f);
    }
    else if (rotation_euler.x + euler.x < -max_angle * (cstrl_pi / 180.0f))
    {
        euler.x = cstrl_min(-max_angle * (cstrl_pi / 180.0f) - rotation_euler.x, 0.0f);
    }
    g_camera_transform.rotation = cstrl_quat_mult(cstrl_quat_from_euler_angles(euler), g_camera_transform.rotation);
    g_camera_transform.rotation = cstrl_quat_normalize(g_camera_transform.rotation);

    rotation_euler = cstrl_euler_angles_from_quat(g_camera_transform.rotation);
    log_trace("%f, %f, %f", rotation_euler.x, rotation_euler.y, rotation_euler.z);
}
