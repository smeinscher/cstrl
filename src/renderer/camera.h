//
// Created by sterling on 7/5/24.
//

#ifndef OMEGA_CAMERA_H
#define OMEGA_CAMERA_H

#include <cstrl/cstrl_math.h>

#include <stdbool.h>

typedef struct camera
{
    float fov;
    vec2i viewport;
    vec3 forward;
    mat4 view;
    mat4 projection;
    transform transform;
} camera;

camera *cstrl_camera_create(int viewport_width, int viewport_height);

void cstrl_camera_free(camera *camera);

void cstrl_camera_update(camera *camera, bool moving_up, bool moving_down, bool moving_left, bool moving_right,
                         bool turning_up, bool turning_down, bool turning_left, bool turning_right);

void cstrl_camera_rotate(camera *camera, float change_y_axis, float change_x_axis);

#endif // OMEGA_CAMERA_H
