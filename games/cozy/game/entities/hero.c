#include "hero.h"
#include "cstrl/cstrl_math.h"
#include "guy.h"

void hero_update(aabb_tree_t *aabb_tree, hero_t *hero, hero_movement_t movement)
{
    vec2 tmp_velocity = {0};
    if (movement & MOVE_UP)
    {
        tmp_velocity.y -= 1.0f;
    }
    if (movement & MOVE_DOWN)
    {
        tmp_velocity.y += 1.0f;
    }
    if (movement & MOVE_LEFT)
    {
        tmp_velocity.x -= 1.0f;
    }
    if (movement & MOVE_RIGHT)
    {
        tmp_velocity.x += 1.0f;
    }
    hero->velocity = cstrl_vec2_normalize(tmp_velocity);

    hero->position = cstrl_vec2_add(hero->position, cstrl_vec2_mult_scalar(hero->velocity, hero->speed));

    vec3 new_aabb[2];
    new_aabb[0] = (vec3){hero->position.x - GUY_SIZE / 2.0f, hero->position.y - GUY_SIZE / 2.0f, 0.0f};
    new_aabb[1] = (vec3){hero->position.x + GUY_SIZE / 2.0f, hero->position.y + GUY_SIZE / 2.0f, 1.0f};
    cstrl_collision_aabb_tree_update_node(aabb_tree, hero->collision_index, new_aabb);
}
