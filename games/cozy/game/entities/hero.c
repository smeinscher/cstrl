#include "hero.h"
#include "cstrl/cstrl_math.h"

void hero_update(hero_t *hero, hero_movement_t movement)
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
}
