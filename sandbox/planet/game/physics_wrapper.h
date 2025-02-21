#ifndef PHYSICS_WRAPPER_H
#define PHYSICS_WRAPPER_H

#include "cstrl/cstrl_physics.h"
typedef enum collision_object_type
{
    COLLISION_UNIT,
    COLLISION_PROJECTILE
} collision_object_type;

typedef struct collision_object_user_data_t
{
    int player_id;
    int type;
    int id;
} collision_object_user_data_t;

ray_cast_result_t curved_ray_cast(vec3 origin, vec3 start_position, vec3 end_position, da_int *excluded_nodes);

ray_cast_result_t regular_ray_cast(vec3 ray_origin, vec3 ray_direction, float max_distance, da_int *excluded_nodes);

void fill_physics_positions(da_float *positions, da_float *ray_positions);

collision_object_user_data_t get_collision_object_user_data(int node_id);

int insert_aabb(collision_object_user_data_t *user_data, vec3 *aabb);

void update_aabb(int node_id, vec3 *aabb);

void remove_aabb(int node_id);

#endif
