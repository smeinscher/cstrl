#include "hero.h"
#include "cstrl/cstrl_math.h"
#include "guy.h"

void hero_update(hero_t *hero, aabb_tree_t *aabb_tree, hero_movement_t movement, cstrl_camera *camera)
{
    vec3 tmp_velocity = {0};
    if (movement & MOVE_UP)
    {
        tmp_velocity.z -= 1.0f;
    }
    if (movement & MOVE_DOWN)
    {
        tmp_velocity.z += 1.0f;
    }
    if (movement & MOVE_LEFT)
    {
        tmp_velocity = cstrl_vec3_add(tmp_velocity, camera->right);
    }
    if (movement & MOVE_RIGHT)
    {
        tmp_velocity = cstrl_vec3_sub(tmp_velocity, camera->right);
    }
    hero->velocity = cstrl_vec3_normalize(tmp_velocity);

    vec3 tmp_position = cstrl_vec3_add(hero->position, cstrl_vec3_mult_scalar(hero->velocity, hero->speed));
    vec3 new_aabb[2];
    new_aabb[0] =
        (vec3){tmp_position.x - GUY_SIZE / 2.0f, tmp_position.y - GUY_SIZE / 2.0f, tmp_position.z - GUY_SIZE / 2.0f};
    new_aabb[1] =
        (vec3){tmp_position.x + GUY_SIZE / 2.0f, tmp_position.y + GUY_SIZE / 2.0f, tmp_position.z + GUY_SIZE / 2.0f};
    da_int intersecting_nodes;
    cstrl_da_int_init(&intersecting_nodes, 4);
    cstrl_collision_aabb_tree_query(aabb_tree, new_aabb, &intersecting_nodes);
    if (intersecting_nodes.size < 2)
    {
        hero->position = tmp_position;
        cstrl_collision_aabb_tree_update_node(aabb_tree, hero->collision_index, new_aabb);
    }
}
