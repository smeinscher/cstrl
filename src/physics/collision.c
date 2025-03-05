#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_util.h"
#include "log.c/log.h"
#include <stdlib.h>

static const vec3 g_fat_aabb_margin = {0.2f, 0.2f, 0.2f};

static bool is_leaf_node(aabb_tree_node_t node)
{
    return node.child0 == -1 && node.child1 == -1;
}

static int allocate_node(aabb_tree_t *tree)
{
    if (tree->node_count >= tree->node_capacity)
    {
        if (tree->node_capacity == 0)
        {
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
    }
    tree->nodes[tree->node_count].aabb[0] = (vec3){0.0f, 0.0f, 0.0f};
    tree->nodes[tree->node_count].aabb[1] = (vec3){0.0f, 0.0f, 0.0f};
    tree->nodes[tree->node_count].fat_aabb[0] = (vec3){0.0f, 0.0f, 0.0f};
    tree->nodes[tree->node_count].fat_aabb[1] = (vec3){0.0f, 0.0f, 0.0f};
    tree->nodes[tree->node_count].active = true;
    tree->nodes[tree->node_count].user_data = NULL;
    tree->nodes[tree->node_count].parent_index = -1;
    tree->nodes[tree->node_count].child0 = -1;
    tree->nodes[tree->node_count].child1 = -1;
    return tree->node_count++;
}

static void aabb_union(vec3 *aabb_out, vec3 *aabb_a, vec3 *aabb_b)
{
    aabb_out[0].x = aabb_a[0].x < aabb_b[0].x ? aabb_a[0].x : aabb_b[0].x;
    aabb_out[0].y = aabb_a[0].y < aabb_b[0].y ? aabb_a[0].y : aabb_b[0].y;
    aabb_out[0].z = aabb_a[0].z < aabb_b[0].z ? aabb_a[0].z : aabb_b[0].z;
    aabb_out[1].x = aabb_a[1].x > aabb_b[1].x ? aabb_a[1].x : aabb_b[1].x;
    aabb_out[1].y = aabb_a[1].y > aabb_b[1].y ? aabb_a[1].y : aabb_b[1].y;
    aabb_out[1].z = aabb_a[1].z > aabb_b[1].z ? aabb_a[1].z : aabb_b[1].z;
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

static void rotate_tree_at_index(aabb_tree_t *tree, int index)
{
    // right rotation
    int new_root = tree->nodes[index].child0;
    if (new_root == -1)
    {
        return;
    }
    if (tree->nodes[index].parent_index == -1)
    {
        tree->root_index = new_root;
    }
    tree->nodes[new_root].parent_index = tree->nodes[index].parent_index;
    tree->nodes[index].parent_index = new_root;
    tree->nodes[index].child0 = tree->nodes[new_root].child1;
    tree->nodes[new_root].child1 = index;
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

    if (aabb_intersection(
            !is_leaf_node(tree->nodes[current]) ? tree->nodes[current].fat_aabb : tree->nodes[current].aabb, aabb))
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
    if (tree->nodes[node_index].child0 == -1 && tree->nodes[node_index].child1 == -1)
    {
        // TODO: parameterize somehow
        tree->nodes[node_index].fat_aabb[0] = cstrl_vec3_sub(tree->nodes[node_index].aabb[0], g_fat_aabb_margin);
        tree->nodes[node_index].fat_aabb[1] = cstrl_vec3_add(tree->nodes[node_index].aabb[1], g_fat_aabb_margin);
    }
    else if (tree->nodes[node_index].child0 == -1)
    {
        tree->nodes[node_index].aabb[0] = tree->nodes[tree->nodes[node_index].child1].fat_aabb[0];
        tree->nodes[node_index].aabb[1] = tree->nodes[tree->nodes[node_index].child1].fat_aabb[1];
    }
    else if (tree->nodes[node_index].child1 == -1)
    {
        tree->nodes[node_index].aabb[0] = tree->nodes[tree->nodes[node_index].child0].fat_aabb[0];
        tree->nodes[node_index].aabb[1] = tree->nodes[tree->nodes[node_index].child0].fat_aabb[1];
    }
    else
    {
        aabb_union(tree->nodes[node_index].fat_aabb, tree->nodes[tree->nodes[node_index].child0].fat_aabb,
                   tree->nodes[tree->nodes[node_index].child1].fat_aabb);
    }
}

static void tree_insert(aabb_tree_t *tree, int node_index)
{
    int parent_index = tree->nodes[node_index].parent_index;
    if (tree->nodes[parent_index].child0 == -1 && tree->nodes[parent_index].child1 == -1)
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
                return (vec2){-1.0f, -1.0f};
            }
            tmin.v[i] = -cstrl_infinity;
            tmax.v[i] = cstrl_infinity;
        }
    }

    float tnear = fmaxf(fmaxf(tmin.x, tmin.y), tmin.z);
    float tfar = fminf(fminf(tmax.x, tmax.y), tmax.z);
    if (tnear > tfar || tfar < 0.0f)
    {
        return (vec2){-1.0f, -1.0f};
    }
    return (vec2){tnear, tfar};
}

static vec2 ray_intersects_gpt(vec3 ray_origin, vec3 ray_direction, vec3 *aabb)
{
    // Calculate intersection values for each axis (x, y, z)
    vec3 t0 = cstrl_vec3_div(cstrl_vec3_sub(aabb[0], ray_origin), ray_direction);
    vec3 t1 = cstrl_vec3_div(cstrl_vec3_sub(aabb[1], ray_origin), ray_direction);

    // Use individual t-values for comparison
    float tmin_x = fminf(t0.x, t1.x);
    float tmax_x = fmaxf(t0.x, t1.x);
    float tmin_y = fminf(t0.y, t1.y);
    float tmax_y = fmaxf(t0.y, t1.y);
    float tmin_z = fminf(t0.z, t1.z);
    float tmax_z = fmaxf(t0.z, t1.z);

    // Calculate the global min/max t-values
    float tmin = fmaxf(fmaxf(tmin_x, tmin_y), tmin_z);
    float tmax = fminf(fminf(tmax_x, tmax_y), tmax_z);

    // If the ray misses the box, return no intersection
    if (tmin > tmax || tmax < 0.0f)
    {
        return (vec2){-1.0f, -1.0f}; // No intersection
    }

    return (vec2){tmin, tmax}; // Return the intersection times
}

static vec2 ray_intersects_no_gpt(vec3 ray_origin, vec3 ray_direction, vec3 *aabb)
{
    vec3 t0 = cstrl_vec3_div(cstrl_vec3_sub(aabb[0], ray_origin), ray_direction);
    vec3 t1 = cstrl_vec3_div(cstrl_vec3_sub(aabb[1], ray_origin), ray_direction);
    vec3 tmin, tmax;

    if (cstrl_vec3_length(t0) < cstrl_vec3_length(t1))
    {
        tmin = t0;
        tmax = t1;
    }
    else
    {
        tmin = t1;
        tmax = t0;
    }
    float t_near = fmaxf(fmaxf(tmin.x, tmin.y), tmin.z);
    float t_far = fminf(fminf(tmax.x, tmax.y), tmax.z);
    return (vec2){t_near, t_far};
}

CSTRL_API void cstrl_collision_aabb_tree_update_node(aabb_tree_t *tree, int node_index, vec3 *new_aabb)
{
    tree->nodes[node_index].aabb[0] = new_aabb[0];
    tree->nodes[node_index].aabb[1] = new_aabb[1];
    tree->nodes[node_index].fat_aabb[0] = cstrl_vec3_sub(new_aabb[0], g_fat_aabb_margin);
    tree->nodes[node_index].fat_aabb[1] = cstrl_vec3_add(new_aabb[1], g_fat_aabb_margin);
    int parent = tree->nodes[node_index].parent_index;
    while (parent != -1)
    {
        update_aabb(tree, parent);
        parent = tree->nodes[parent].parent_index;
    }
}

CSTRL_API void cstrl_collision_aabb_tree_query(aabb_tree_t *tree, vec3 *aabb, da_int *intersecting_nodes)
{
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
    update_aabb(tree, new_node);

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
    }
    else
    {
        tree->root_index = -1;
        tree->node_count = 0;
    }
    free(tree->nodes[node_index].user_data);
    tree->nodes[node_index].user_data = NULL;
    tree->nodes[node_index].active = false;
}

CSTRL_API ray_cast_result_t cstrl_collision_aabb_tree_ray_cast(aabb_tree_t *tree, vec3 ray_origin, vec3 ray_direction,
                                                               float max_distance, da_int *excluded_nodes)
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

        if (excluded_nodes != NULL && cstrl_da_int_find_first(excluded_nodes, node_index) != CSTRL_DA_INT_ITEM_NOT_FOUND)
        {
            continue;
        }

        vec2 ray_aabb_result = ray_intersects(ray_origin, ray_direction,
                                              !is_leaf_node(tree->nodes[node_index]) ? tree->nodes[node_index].fat_aabb
                                                                                     : tree->nodes[node_index].aabb);
        if (ray_aabb_result.x < ray_aabb_result.y)
        {
            float t;
            if (ray_aabb_result.x > 0.0f)
            {
                t = ray_aabb_result.x;
            }
            else
            {
                t = ray_aabb_result.y;
            }
            if (t > max_distance || result.hit && result.t < t)
            {
                continue;
            }
            if (tree->nodes[node_index].child0 == -1 && tree->nodes[node_index].child1 == -1)
            {
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
                        vec3 center = cstrl_vec3_add(center, tree->nodes[node_index].aabb[0]);
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
                cstrl_da_int_push_back(&nodes, tree->nodes[node_index].child0);
                cstrl_da_int_push_back(&nodes, tree->nodes[node_index].child1);
            }
        }
    }
    cstrl_da_int_free(&nodes);
    return result;
}
