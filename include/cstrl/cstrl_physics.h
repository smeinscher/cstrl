#ifndef CSTRL_PHYSICS_H
#define CSTRL_PHYSICS_H

#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"

typedef struct aabb_tree_node_t
{
    vec3 aabb[2];
    vec3 fat_aabb[2];
    int parent_index;
    int child0;
    int child1;
    bool active;
    void *user_data;
} aabb_tree_node_t;

typedef struct aabb_tree_t
{
    aabb_tree_node_t *nodes;
    int node_count;
    int node_capacity;
    int root_index;
} aabb_tree_t;

typedef struct ray_cast_result_t
{
    bool hit;
    float t;
    int node_index;
    vec3 normal;
    vec3 intersection;
    vec3 aabb_center;
} ray_cast_result_t;

CSTRL_API void cstrl_collision_aabb_tree_update_node(aabb_tree_t *tree, int node_index, vec3 *new_aabb);

CSTRL_API void cstrl_collision_aabb_tree_query(aabb_tree_t *tree, vec3 *aabb, da_int *intersecting_nodes);

CSTRL_API int cstrl_collision_aabb_tree_insert(aabb_tree_t *tree, void *user_data, vec3 *aabb);

CSTRL_API void cstrl_collision_aabb_tree_remove(aabb_tree_t *tree, int node_index);

CSTRL_API ray_cast_result_t cstrl_collision_aabb_tree_ray_cast(aabb_tree_t *tree, vec3 ray_origin, vec3 ray_direction, float max_distance, da_int *excluded_nodes);

#endif // CSTRL_PHYSICS_H
