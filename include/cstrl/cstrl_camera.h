//
// Created by 12105 on 12/15/2024.
//

#ifndef CSTRL_CAMERA_H
#define CSTRL_CAMERA_H
#include "cstrl_math.h"

#include <stdbool.h>

typedef struct cstrl_camera
{
    bool is_orthographic;
    float fov;
    float near;
    float far;
    vec2i viewport;
    vec3 position;
    vec3 forward;
    vec3 up;
    vec3 right;
    mat4 view;
    mat4 projection;
} cstrl_camera;

typedef enum cstrl_camera_direction_mask
{
    CSTRL_CAMERA_DIRECTION_NONE = 0,
    CSTRL_CAMERA_DIRECTION_UP = 1,
    CSTRL_CAMERA_DIRECTION_DOWN = 2,
    CSTRL_CAMERA_DIRECTION_LEFT = 4,
    CSTRL_CAMERA_DIRECTION_RIGHT = 8,
} cstrl_camera_direction_mask;

CSTRL_API cstrl_camera *cstrl_camera_create(int viewport_width, int viewport_height, bool is_orthographic);

CSTRL_API void cstrl_camera_free(cstrl_camera *camera);

CSTRL_API void cstrl_camera_update(cstrl_camera *camera, cstrl_camera_direction_mask movement,
                         cstrl_camera_direction_mask rotation);

CSTRL_API void cstrl_camera_euler_rotate(vec3 amount);

CSTRL_API void cstrl_camera_first_person_rotate(cstrl_camera *camera, float change_y_axis, float change_x_axis);

CSTRL_API void cstrl_camera_set_rotation(cstrl_camera *camera, quat rotation);

CSTRL_API void cstrl_camera_rotate_around_point(cstrl_camera *camera, vec3 point, float vertical_angle_change, float horizontal_angle_change);

CSTRL_API quat cstrl_camera_get_rotation(cstrl_camera *camera);

#endif // CSTRL_CAMERA_H
