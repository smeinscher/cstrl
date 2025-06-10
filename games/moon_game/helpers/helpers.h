#ifndef HELPERS_H
#define HELPERS_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_util.h"
#include <stdbool.h>

bool planet_hit_check(vec3 d, float *t, vec3 origin, vec3 center, float radius);

void generate_line_segments(da_float *positions, vec3 origin, vec3 destination, float granularity);

vec3 screen_ray_cast(vec2 screen_coords, vec2 screen_size, mat4 projection, mat4 view);

vec2 world_to_screen(vec3 world_coords, vec2 screen_size, mat4 projection, mat4 view);

vec3 modify_point(vec3 point, transform_t transform);

void get_points(vec3 *p0, vec3 *p1, vec3 *p2, vec3 *p3, transform_t transform);

vec3 get_point_on_path(vec3 origin, vec3 start_position, vec3 end_position, float t);

float get_spherical_path_length(vec3 start_position, vec3 end_position);

void generate_cube_positions(float *positions);

#endif
