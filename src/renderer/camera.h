//
// Created by sterling on 7/5/24.
//

#ifndef OMEGA_CAMERA_H
#define OMEGA_CAMERA_H

#include <cstrl/cstrl_math.h>

#include <stdbool.h>

void camera_update();

void camera_set_moving_up(bool moving_up);

void camera_set_moving_down(bool moving_down);

void camera_set_moving_left(bool moving_left);

void camera_set_moving_right(bool moving_right);

float camera_get_zoom();

float camera_get_zoom_factor();

float camera_get_zoom_ratio();

void camera_set_zoom(float zoom);

void camera_increment_zoom(float amount);

void camera_decrement_zoom(float amount);

vec3 camera_get_position();

void camera_set_position(const vec3 position);

void camera_set_max_position(const vec2 position);

void camera_set_min_position(const vec2 position);

mat4 camera_get_projection();

mat4 camera_get_view();

int camera_get_viewport_width();

int camera_get_viewport_height();

void camera_set_viewport_width(int viewport_width);

void camera_set_viewport_height(int viewport_height);

#endif // OMEGA_CAMERA_H
