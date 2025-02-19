#ifndef HELPERS_H
#define HELPERS_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_types.h"
#include <stdbool.h>

bool hit_check(vec3 d, float *t, vec3 origin, vec3 center, float radius);

void generate_line_segments(da_float *positions, vec3 origin, vec3 destination, float granularity);

vec3 screen_ray_cast(vec2 screen_coords, vec2 screen_size, mat4 projection, mat4 view);

vec2 world_to_screen(vec3 world_coords, vec2 screen_size, mat4 projection, mat4 view);

vec3 modify_point(vec3 point, transform transform);

void get_points(vec3 *p0, vec3 *p1, vec3 *p2, vec3 *p3, transform transform);

vec3 get_point_on_path(vec3 origin, vec3 start_position, vec3 end_position, float t);

ray_cast_result_t curved_ray_cast(aabb_tree_t *tree, vec3 origin, vec3 start_position, vec3 end_position, da_int *excluded_nodes);

float get_spherical_path_length(vec3 start_position, vec3 end_position);

void generate_cube_positions(float *positions);

#endif
