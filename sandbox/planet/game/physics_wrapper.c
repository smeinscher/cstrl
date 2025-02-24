#include "physics_wrapper.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_util.h"
#include "../helpers/helpers.h"

aabb_tree_t g_aabb_tree;

ray_cast_result_t curved_ray_cast(vec3 origin, vec3 start_position, vec3 end_position,
                                  da_int *excluded_nodes)
{
    da_float positions;
    cstrl_da_float_init(&positions, 3);
    generate_line_segments(&positions, start_position, end_position, 0.1f);
    ray_cast_result_t result;
    result.hit = false;
    result.node_index = -1;
    result.t = 1.0f;
    result.normal = (vec3){0.0f, 0.0f, 0.0f};
    result.intersection = (vec3){0.0f, 0.0f, 0.0f};
    result.aabb_center = (vec3){0.0f, 0.0f, 0.0f};
    if (positions.size == 0)
    {
        cstrl_da_float_free(&positions);
        return result;
    }
    for (int i = 0; i < positions.size / 3 - 1; i++)
    {
        vec3 start = {positions.array[i * 3], positions.array[i * 3 + 1], positions.array[i * 3 + 2]};
        vec3 end = {positions.array[i * 3 + 3], positions.array[i * 3 + 4], positions.array[i * 3 + 5]};
        vec3 difference = cstrl_vec3_sub(end, start);
        vec3 direction = cstrl_vec3_normalize(difference);
        result = cstrl_collision_aabb_tree_ray_cast(&g_aabb_tree, start_position, direction, 1.0f, excluded_nodes);
        if (result.hit)
        {
            break;
        }
    }
    cstrl_da_float_free(&positions);
    return result;
}

ray_cast_result_t regular_ray_cast(vec3 ray_origin, vec3 ray_direction, float max_distance, da_int *excluded_nodes)
{
    return cstrl_collision_aabb_tree_ray_cast(&g_aabb_tree, ray_origin, ray_direction, max_distance, excluded_nodes);
}

void fill_physics_positions(da_float *positions, da_float *ray_positions)
{
    for (int i = 0; i < g_aabb_tree.node_count; i++)
    {
        vec3 aabb[2];
        aabb[0] = g_aabb_tree.nodes[i].aabb[0];
        aabb[1] = g_aabb_tree.nodes[i].aabb[1];

        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
    }
    for (int i = 0; i < ray_positions->size; i++)
    {
        cstrl_da_float_push_back(positions, ray_positions->array[i]);
    }
}

collision_object_user_data_t get_collision_object_user_data(int node_id)
{
        return *(collision_object_user_data_t *)g_aabb_tree.nodes[node_id].user_data;
}

int insert_aabb(collision_object_user_data_t *user_data, vec3 *aabb)
{
    return cstrl_collision_aabb_tree_insert(&g_aabb_tree, user_data, aabb);
}

void update_aabb(int node_id, vec3 *aabb)
{

    cstrl_collision_aabb_tree_update_node(&g_aabb_tree, node_id, aabb);
}

void remove_aabb(int node_id)
{
    cstrl_collision_aabb_tree_remove(&g_aabb_tree, node_id);
}
