#include "test_collision.h"
#include "../../test_manager/test_types.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_util.h"

static void print_tree(aabb_tree_t *tree)
{
    da_int this_level;
    cstrl_da_int_init(&this_level, 1);
    cstrl_da_int_push_back(&this_level, tree->root_index);
    while (this_level.size != 0)
    {
        da_int next_level;
        cstrl_da_int_init(&next_level, 1);
        for (int i = 0; i < this_level.size; i++)
        {
            printf("%d ", this_level.array[i]);
            if (tree->nodes[this_level.array[i]].child0 != -1)
            {
                cstrl_da_int_push_back(&next_level, tree->nodes[this_level.array[i]].child0);
            }
            if (tree->nodes[this_level.array[i]].child1 != -1)
            {
                cstrl_da_int_push_back(&next_level, tree->nodes[this_level.array[i]].child1);
            }
        }
        printf("\n");
        cstrl_da_int_clear(&this_level);
        for (int i = 0; i < next_level.size; i++)
        {
            cstrl_da_int_push_back(&this_level, next_level.array[i]);
        }
        cstrl_da_int_free(&next_level);
    }
    cstrl_da_int_free(&this_level);
}

int test_cstrl_collision_aabb_tree_insert_leaf()
{
    aabb_tree_t tree = {0};
    vec3 aabb_box_a[2];
    aabb_box_a[0] = (vec3){0.0f, 0.0f, 0.0f};
    aabb_box_a[1] = (vec3){5.0f, 5.0f, 5.0f};
    int id_a = 42;
    cstrl_collision_aabb_tree_insert(&tree, &id_a, aabb_box_a);

    expect_int_to_be(1, tree.node_count);
    expect_bool_to_be(true, cstrl_vec3_is_equal(tree.nodes[0].aabb[0], aabb_box_a[0]));
    expect_bool_to_be(true, cstrl_vec3_is_equal(tree.nodes[0].aabb[1], aabb_box_a[1]));
    expect_int_to_be(42, *(int *)tree.nodes[0].user_data);
    expect_int_to_be(-1, tree.nodes[0].parent_index);
    expect_int_to_be(-1, tree.nodes[0].child0);
    expect_int_to_be(-1, tree.nodes[0].child1);

    vec3 aabb_box_b[2];
    aabb_box_b[0] = (vec3){0.1f, 0.1f, 0.1f};
    aabb_box_b[1] = (vec3){0.2f, 0.2f, 0.2f};
    int id_b = 100;
    cstrl_collision_aabb_tree_insert(&tree, &id_b, aabb_box_b);

    vec3 aabb_box_c[2];
    aabb_box_c[0] = (vec3){0.5f, 0.5f, 0.5f};
    aabb_box_c[1] = (vec3){0.6f, 0.6f, 0.6f};
    int id_c = 55;
    cstrl_collision_aabb_tree_insert(&tree, &id_c, aabb_box_c);

    vec3 aabb_box_d[2];
    aabb_box_d[0] = (vec3){0.6f, 0.6f, 0.6f};
    aabb_box_d[1] = (vec3){0.8f, 0.8f, 0.8f};
    int id_d = 13;
    cstrl_collision_aabb_tree_insert(&tree, &id_d, aabb_box_d);

    vec3 aabb_box_e[2];
    aabb_box_e[0] = (vec3){1.6f, 1.6f, 1.6f};
    aabb_box_e[1] = (vec3){2.8f, 2.8f, 2.8f};
    int id_e = 7;
    cstrl_collision_aabb_tree_insert(&tree, &id_e, aabb_box_e);

    vec3 aabb_box_f[2];
    aabb_box_f[0] = (vec3){-1.6f, -1.6f, -1.6f};
    aabb_box_f[1] = (vec3){-0.8f, -0.8f, -0.8f};
    int id_f = 8;
    cstrl_collision_aabb_tree_insert(&tree, &id_f, aabb_box_f);

    vec3 aabb_box_g[2];
    aabb_box_g[0] = (vec3){-10.8f, -10.8f, -10.8f};
    aabb_box_g[1] = (vec3){-5.0f, -5.0f, -5.0f};
    int id_g = 9;
    cstrl_collision_aabb_tree_insert(&tree, &id_g, aabb_box_g);

    print_tree(&tree);
    for (int i = 0; i < tree.node_count; i++)
    {
        int id = tree.nodes[i].user_data != NULL ? *(int *)tree.nodes[i].user_data : -1;
        log_debug("%d. %d | P: %d | L: %d R: %d", i, id, tree.nodes[i].parent_index, tree.nodes[i].child0,
                  tree.nodes[i].child1);
    }

    ray_cast_result_t result = cstrl_collision_aabb_tree_ray_cast(
        &tree, (vec3){-12.0f, -12.0f, -12.0f}, cstrl_vec3_normalize((vec3){1.0f, 1.0f, 1.0f}), 5.0f, NULL);

    if (result.hit)
    {
        printf("index = %d id = %d, t = %f\n", *(int *)tree.nodes[result.node_index].user_data, result.node_index,
               result.t);
        if (result.hit)
        {
            vec3 *aabb = tree.nodes[result.node_index].aabb;
            printf("(%f, %f, %f), (%f, %f, %f)\n", aabb[0].x, aabb[0].y, aabb[0].z, aabb[1].x, aabb[1].y, aabb[1].z);
        }
    }
    else
    {
        printf("failed\n");
    }

    return 1;
}
