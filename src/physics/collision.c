#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_util.h"
#include "log.c/log.h"
#include <float.h>
#include <stdlib.h>

static const vec3 g_fat_aabb_margin = {0.05f, 0.05f, 0.05f};

static bool is_leaf_node(aabb_tree_node_t node)
{
    return node.child0 == -1 && node.child1 == -1;
}

static int allocate_node(aabb_tree_t *tree)
{
    int node_index;
    if (tree->freed_nodes.size != 0)
    {
        node_index = cstrl_da_int_pop_back(&tree->freed_nodes);
    }
    else if (tree->node_count >= tree->node_capacity)
    {
        if (tree->node_capacity == 0)
        {
            cstrl_da_int_init(&tree->freed_nodes, 64);
            tree->node_capacity = 1;
        }
        else
        {
            tree->node_capacity *= 2;
        }
        aabb_tree_node_t *temp = realloc(tree->nodes, tree->node_capacity * sizeof(aabb_tree_node_t));
        if (!temp)
        {
            log_error("Failed to realloc tree node");
            return -1;
        }
        tree->nodes = temp;
        node_index = tree->node_count;
        tree->node_count++;
    }
    else
    {
        node_index = tree->node_count;
        tree->node_count++;
    }
    tree->nodes[node_index] = (aabb_tree_node_t){0};
    tree->nodes[node_index].active = true;
    tree->nodes[node_index].parent_index = -1;
    tree->nodes[node_index].child0 = -1;
    tree->nodes[node_index].child1 = -1;
    return node_index;
}

static void aabb_union(vec3 *aabb_out, vec3 *aabb_a, vec3 *aabb_b)
{
    aabb_out[0].x = fminf(aabb_a[0].x, aabb_b[0].x);
    aabb_out[0].y = fminf(aabb_a[0].y, aabb_b[0].y);
    aabb_out[0].z = fminf(aabb_a[0].z, aabb_b[0].z);
    aabb_out[1].x = fmaxf(aabb_a[1].x, aabb_b[1].x);
    aabb_out[1].y = fmaxf(aabb_a[1].y, aabb_b[1].y);
    aabb_out[1].z = fmaxf(aabb_a[1].z, aabb_b[1].z);
}

static float compute_surface_area(vec3 *aabb)
{
    vec3 a = aabb[0];
    vec3 b = aabb[1];
    vec3 c = {a.x, b.y, a.z};
    vec3 d = {a.x, a.y, b.z};
    vec3 e = {b.x, a.y, a.z};
    float length = cstrl_vec3_length(cstrl_vec3_sub(a, c));
    float width = cstrl_vec3_length(cstrl_vec3_sub(a, d));
    float height = cstrl_vec3_length(cstrl_vec3_sub(a, e));
    return 2 * (length * width + length * height + width * height);
}

static bool aabb_intersection(vec3 *aabb_a, vec3 *aabb_b)
{
    return (aabb_a[0].x <= aabb_b[1].x && aabb_a[1].x >= aabb_b[0].x && aabb_a[0].y <= aabb_b[1].y &&
            aabb_a[1].y >= aabb_b[0].y && aabb_a[0].z <= aabb_b[1].z && aabb_a[1].z >= aabb_b[0].z);
}

static void query(aabb_tree_t *tree, vec3 *aabb, da_int *intersecting_nodes, int current)
{
    if (current == -1)
    {
        return;
    }

    if (aabb_intersection(tree->nodes[current].aabb, aabb))
    {
        if (tree->nodes[current].child0 == -1 && tree->nodes[current].child1 == -1)
        {
            cstrl_da_int_push_back(intersecting_nodes, current);
        }
        else
        {
            query(tree, aabb, intersecting_nodes, tree->nodes[current].child0);
            query(tree, aabb, intersecting_nodes, tree->nodes[current].child1);
        }
    }
}

static void update_aabb(aabb_tree_t *tree, int node_index)
{
    if (is_leaf_node(tree->nodes[node_index]))
    {
        // TODO: parameterize somehow
        // TODO: Do I even need this?
        /*tree->nodes[node_index].fat_aabb[0] = cstrl_vec3_sub(tree->nodes[node_index].aabb[0], g_fat_aabb_margin);
        tree->nodes[node_index].fat_aabb[1] = cstrl_vec3_add(tree->nodes[node_index].aabb[1], g_fat_aabb_margin);*/
        return;
    }
    else if (tree->nodes[node_index].child0 == -1)
    {
        tree->nodes[node_index].aabb[0] = is_leaf_node(tree->nodes[tree->nodes[node_index].child1])
                                              ? tree->nodes[tree->nodes[node_index].child1].fat_aabb[0]
                                              : tree->nodes[tree->nodes[node_index].child1].aabb[0];
        tree->nodes[node_index].aabb[1] = is_leaf_node(tree->nodes[tree->nodes[node_index].child1])
                                              ? tree->nodes[tree->nodes[node_index].child1].fat_aabb[1]
                                              : tree->nodes[tree->nodes[node_index].child1].aabb[1];
    }
    else if (tree->nodes[node_index].child1 == -1)
    {
        tree->nodes[node_index].aabb[0] = is_leaf_node(tree->nodes[tree->nodes[node_index].child0])
                                              ? tree->nodes[tree->nodes[node_index].child0].fat_aabb[0]
                                              : tree->nodes[tree->nodes[node_index].child0].aabb[0];
        tree->nodes[node_index].aabb[1] = is_leaf_node(tree->nodes[tree->nodes[node_index].child0])
                                              ? tree->nodes[tree->nodes[node_index].child0].fat_aabb[1]
                                              : tree->nodes[tree->nodes[node_index].child0].aabb[1];
    }
    else
    {
        aabb_union(tree->nodes[node_index].aabb,
                   is_leaf_node(tree->nodes[tree->nodes[node_index].child0])
                       ? tree->nodes[tree->nodes[node_index].child0].fat_aabb
                       : tree->nodes[tree->nodes[node_index].child0].aabb,
                   is_leaf_node(tree->nodes[tree->nodes[node_index].child1])
                       ? tree->nodes[tree->nodes[node_index].child1].fat_aabb
                       : tree->nodes[tree->nodes[node_index].child1].aabb);
    }
}

static void tree_insert(aabb_tree_t *tree, int node_index)
{
    int parent_index = tree->nodes[node_index].parent_index;
    if (is_leaf_node(tree->nodes[parent_index]))
    {
        int new_parent = allocate_node(tree);
        if (tree->nodes[parent_index].parent_index != -1)
        {
            int grandparent = tree->nodes[parent_index].parent_index;
            if (tree->nodes[grandparent].child0 == parent_index)
            {
                tree->nodes[grandparent].child0 = new_parent;
            }
            else
            {
                tree->nodes[grandparent].child1 = new_parent;
            }
        }
        else
        {
            tree->root_index = new_parent;
        }
        tree->nodes[new_parent].parent_index = tree->nodes[parent_index].parent_index;
        tree->nodes[new_parent].child0 = node_index;
        tree->nodes[new_parent].child1 = parent_index;
        tree->nodes[node_index].parent_index = new_parent;
        tree->nodes[parent_index].parent_index = new_parent;
        parent_index = new_parent;
    }
    else
    {
        if (tree->nodes[parent_index].child0 == -1)
        {
            tree->nodes[node_index].parent_index = tree->nodes[parent_index].child1;
        }
        else if (tree->nodes[parent_index].child1 == -1)
        {
            tree->nodes[node_index].parent_index = tree->nodes[parent_index].child0;
        }
        else
        {
            vec3 aabb[2];
            aabb_union(aabb, tree->nodes[node_index].aabb, tree->nodes[tree->nodes[parent_index].child0].aabb);
            float cost0 = compute_surface_area(aabb);
            aabb_union(aabb, tree->nodes[node_index].aabb, tree->nodes[tree->nodes[parent_index].child1].aabb);
            float cost1 = compute_surface_area(aabb);
            if (cost0 < cost1)
            {
                tree->nodes[node_index].parent_index = tree->nodes[parent_index].child0;
            }
            else
            {
                tree->nodes[node_index].parent_index = tree->nodes[parent_index].child1;
            }
        }

        tree_insert(tree, node_index);
    }
    if (parent_index == node_index)
    {
        printf("Oof\n");
    }
    update_aabb(tree, parent_index);
}

static vec2 ray_intersects(vec3 ray_origin, vec3 ray_direction, vec3 *aabb)
{
    vec3 tmin, tmax;
    for (int i = 0; i < 3; i++)
    {
        if (fabsf(ray_direction.v[i]) > cstrl_epsilon)
        {
            tmin.v[i] = (aabb[0].v[i] - ray_origin.v[i]) / ray_direction.v[i];
            tmax.v[i] = (aabb[1].v[i] - ray_origin.v[i]) / ray_direction.v[i];

            if (tmin.v[i] > tmax.v[i])
            {
                float temp = tmin.v[i];
                tmin.v[i] = tmax.v[i];
                tmax.v[i] = temp;
            }
        }
        else
        {
            if (ray_origin.v[i] < aabb[0].v[i] || ray_origin.v[i] > aabb[1].v[i])
            {
                return (vec2){-FLT_MAX, FLT_MAX};
            }
            tmin.v[i] = -FLT_MAX;
            tmax.v[i] = FLT_MAX;
        }
    }

    float tnear = fmaxf(fmaxf(fmaxf(tmin.x, tmin.y), tmin.z), 0.0f);
    float tfar = fminf(fminf(tmax.x, tmax.y), tmax.z);
    if (tnear > tfar || tfar < 0.0f)
    {
        return (vec2){-FLT_MAX, FLT_MAX};
    }
    return (vec2){tnear, tfar};
}

static bool ray_origin_in_aabb(vec3 ray_origin, vec3 *aabb)
{
    return ray_origin.x >= aabb[0].x && ray_origin.x <= aabb[1].x && ray_origin.y >= aabb[0].y &&
           ray_origin.y <= aabb[1].y && ray_origin.z >= aabb[0].z && ray_origin.z <= aabb[1].z;
}

CSTRL_API int cstrl_collision_aabb_tree_update_node(aabb_tree_t *tree, int node_index, vec3 *new_aabb)
{
    tree->nodes[node_index].aabb[0] = new_aabb[0];
    tree->nodes[node_index].aabb[1] = new_aabb[1];
    if (tree->nodes[node_index].fat_aabb[0].x <= new_aabb[0].x &&
        tree->nodes[node_index].fat_aabb[0].y <= new_aabb[0].y &&
        tree->nodes[node_index].fat_aabb[0].z <= new_aabb[0].z &&
        tree->nodes[node_index].fat_aabb[1].x >= new_aabb[1].x &&
        tree->nodes[node_index].fat_aabb[1].y >= new_aabb[1].y &&
        tree->nodes[node_index].fat_aabb[1].z >= new_aabb[1].z)
    {
        return node_index;
    }
    void *user_data = tree->nodes[node_index].user_data;
    cstrl_collision_aabb_tree_remove(tree, node_index);
    node_index = allocate_node(tree);
    tree->nodes[node_index].aabb[0] = new_aabb[0];
    tree->nodes[node_index].aabb[1] = new_aabb[1];
    tree->nodes[node_index].fat_aabb[0] = cstrl_vec3_sub(new_aabb[0], g_fat_aabb_margin);
    tree->nodes[node_index].fat_aabb[1] = cstrl_vec3_add(new_aabb[1], g_fat_aabb_margin);
    tree->nodes[node_index].parent_index = tree->root_index;
    tree->nodes[node_index].user_data = user_data;
    tree_insert(tree, node_index);
    // int parent = tree->nodes[node_index].parent_index;
    // while (parent != -1)
    // {
    //     update_aabb(tree, parent);
    //     parent = tree->nodes[parent].parent_index;
    // }
    return node_index;
}

CSTRL_API void cstrl_collision_aabb_tree_query(aabb_tree_t *tree, vec3 *aabb, da_int *intersecting_nodes)
{
    if (tree->node_count == 0)
    {
        return;
    }
    query(tree, aabb, intersecting_nodes, tree->root_index);
}

CSTRL_API int cstrl_collision_aabb_tree_insert(aabb_tree_t *tree, void *user_data, vec3 *aabb)
{
    int new_node = allocate_node(tree);

    tree->nodes[new_node].aabb[0] = aabb[0];
    tree->nodes[new_node].aabb[1] = aabb[1];
    tree->nodes[new_node].fat_aabb[0] = cstrl_vec3_sub(aabb[0], g_fat_aabb_margin);
    tree->nodes[new_node].fat_aabb[1] = cstrl_vec3_add(aabb[1], g_fat_aabb_margin);
    tree->nodes[new_node].user_data = user_data;

    if (tree->node_count == 1)
    {
        tree->root_index = new_node;
        return new_node;
    }

    tree->nodes[new_node].parent_index = tree->root_index;
    // update_aabb(tree, new_node);

    tree_insert(tree, new_node);

    return new_node;
}

CSTRL_API void cstrl_collision_aabb_tree_remove(aabb_tree_t *tree, int node_index)
{
    if (tree->nodes[node_index].active == false)
    {
        log_warn("Attempting to remove collision node that is not active, skipping...");
        return;
    }
    int parent_index = tree->nodes[node_index].parent_index;
    if (parent_index != -1)
    {
        int sibling;
        if (tree->nodes[parent_index].child0 == node_index)
        {
            sibling = tree->nodes[parent_index].child1;
        }
        else
        {
            sibling = tree->nodes[parent_index].child0;
        }
        // TODO: check if sibling is -1
        if (tree->nodes[parent_index].parent_index != -1)
        {
            tree->nodes[sibling].parent_index = tree->nodes[parent_index].parent_index;
            if (parent_index == tree->nodes[tree->nodes[parent_index].parent_index].child0)
            {
                tree->nodes[tree->nodes[parent_index].parent_index].child0 = sibling;
            }
            else
            {
                tree->nodes[tree->nodes[parent_index].parent_index].child1 = sibling;
            }
        }
        else
        {
            tree->root_index = sibling;
            tree->nodes[sibling].parent_index = -1;
        }
        tree->nodes[parent_index].active = false;
        cstrl_da_int_push_back(&tree->freed_nodes, parent_index);
    }
    else
    {
        tree->root_index = -1;
    }
    tree->nodes[node_index].active = false;
    cstrl_da_int_push_back(&tree->freed_nodes, node_index);
}

CSTRL_API ray_cast_result_t cstrl_collision_aabb_tree_ray_cast(aabb_tree_t *tree, vec3 ray_origin, vec3 ray_direction,
                                                               float max_distance, da_int *excluded_nodes,
                                                               da_int *intersected_nodes)
{
    ray_cast_result_t result;
    result.hit = false;
    result.node_index = -1;
    result.t = 1.0f;
    result.normal = (vec3){0.0f, 0.0f, 0.0f};
    result.intersection = (vec3){0.0f, 0.0f, 0.0f};
    result.aabb_center = (vec3){0.0f, 0.0f, 0.0f};

    da_int nodes;
    cstrl_da_int_init(&nodes, 1);

    if (tree->root_index != -1)
    {
        cstrl_da_int_push_back(&nodes, tree->root_index);
    }
    while (nodes.size != 0)
    {
        int node_index = cstrl_da_int_pop_front(&nodes);

        if (excluded_nodes != NULL &&
            cstrl_da_int_find_first(excluded_nodes, node_index) != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            continue;
        }

        if (ray_origin_in_aabb(ray_origin, tree->nodes[node_index].aabb))
        {
            if (!is_leaf_node(tree->nodes[node_index]))
            {
                cstrl_da_int_push_back(&nodes, tree->nodes[node_index].child0);
                cstrl_da_int_push_back(&nodes, tree->nodes[node_index].child1);
            }
            else
            {
                // TODO: figure out how to handle when inside other object
                result.hit = true;
                result.node_index = node_index;
                result.t = 0.0f;
                result.intersection = ray_origin;
                vec3 center_distance = cstrl_vec3_mult_scalar(
                    cstrl_vec3_sub(tree->nodes[node_index].aabb[1], tree->nodes[node_index].aabb[0]), 0.5f);
                vec3 center = cstrl_vec3_add(center_distance, tree->nodes[node_index].aabb[0]);
                result.aabb_center = center;
                vec3 normal = cstrl_vec3_div_scalar(cstrl_vec3_sub(result.intersection, center),
                                                    cstrl_vec3_length(center_distance));
                result.normal = cstrl_vec3_normalize(normal);
            }
            continue;
        }

        vec2 ray_aabb_result = ray_intersects(ray_origin, ray_direction, tree->nodes[node_index].aabb);
        if (ray_aabb_result.x >= ray_aabb_result.y || ray_aabb_result.x == -FLT_MAX || ray_aabb_result.y == FLT_MAX)
        {
            continue;
        }
        float t = ray_aabb_result.x > 0.0f ? ray_aabb_result.x : ray_aabb_result.y;
        if (t > max_distance)
        {
            continue;
        }
        else if (result.hit && result.t < t)
        {
            if (!is_leaf_node(tree->nodes[node_index]))
            {
                if (tree->nodes[node_index].child0 != -1)
                {
                    cstrl_da_int_push_back(&nodes, tree->nodes[node_index].child0);
                }
                if (tree->nodes[node_index].child1 != -1)
                {
                    cstrl_da_int_push_back(&nodes, tree->nodes[node_index].child1);
                }
            }
            else
            {
                if (intersected_nodes != NULL)
                {
                    cstrl_da_int_push_back(intersected_nodes, node_index);
                }
            }
            continue;
        }
        if (is_leaf_node(tree->nodes[node_index]))
        {
            if (intersected_nodes != NULL)
            {
                cstrl_da_int_push_back(intersected_nodes, node_index);
            }
            if (result.hit)
            {
                if (t < result.t)
                {
                    result.node_index = node_index;
                    result.t = t;
                    vec3 direction = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(ray_direction), t * max_distance);
                    result.intersection = cstrl_vec3_add(ray_origin, direction);
                    vec3 center_distance = cstrl_vec3_mult_scalar(
                        cstrl_vec3_sub(tree->nodes[node_index].aabb[1], tree->nodes[node_index].aabb[0]), 0.5f);
                    vec3 center = cstrl_vec3_add(center_distance, tree->nodes[node_index].aabb[0]);
                    result.aabb_center = center;
                    vec3 normal = cstrl_vec3_div_scalar(cstrl_vec3_sub(result.intersection, center),
                                                        cstrl_vec3_length(center_distance));
                    result.normal = cstrl_vec3_normalize(normal);
                }
            }
            else
            {
                result.hit = true;
                result.node_index = node_index;
                result.t = t;
                vec3 direction = cstrl_vec3_mult_scalar(cstrl_vec3_normalize(ray_direction), t * max_distance);
                result.intersection = cstrl_vec3_add(ray_origin, direction);
                vec3 center_distance = cstrl_vec3_mult_scalar(
                    cstrl_vec3_sub(tree->nodes[node_index].aabb[1], tree->nodes[node_index].aabb[0]), 0.5f);
                vec3 center = cstrl_vec3_add(center_distance, tree->nodes[node_index].aabb[0]);
                result.aabb_center = center;
                vec3 normal = cstrl_vec3_div_scalar(cstrl_vec3_sub(result.intersection, center),
                                                    cstrl_vec3_length(center_distance));
                result.normal = cstrl_vec3_normalize(normal);
            }
        }
        else
        {
            if (tree->nodes[node_index].child0 != -1)
            {
                cstrl_da_int_push_back(&nodes, tree->nodes[node_index].child0);
            }
            if (tree->nodes[node_index].child1 != -1)
            {
                cstrl_da_int_push_back(&nodes, tree->nodes[node_index].child1);
            }
        }
    }
    cstrl_da_int_free(&nodes);
    return result;
}
