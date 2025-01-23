#include "challenge.h"
#include "cstrl/cstrl_math.h"
#include <stdio.h>

typedef struct projectile_t
{
    vec4 position;
    vec4 velocity;
} projectile_t;

typedef struct environment_t
{
    vec4 gravity;
    vec4 wind;
} environment_t;

void tick(environment_t *env, projectile_t *proj)
{
    vec4 position = cstrl_vec4_add(proj->position, proj->velocity);
    vec4 change = cstrl_vec4_add(env->gravity, env->wind);
    vec4 velocity = cstrl_vec4_add(proj->velocity, change);

    proj->position = position;
    proj->velocity = velocity;
}

int ray_tracer_challenge()
{
    projectile_t projectile;
    projectile.position = (vec4){0.0f, 1.0f, 0.0f, 1.0f};
    projectile.velocity = cstrl_vec4_normalize((vec4){1.0f, 1.0f, 0.0f, 0.0f});

    environment_t environment;
    environment.gravity = (vec4){0.0f, -0.1f, 0.0f, 0.0f};
    environment.wind = (vec4){-0.01f, 0.0f, 0.0f, 0.0f};

    while (projectile.position.y > 0.0f)
    {
        printf("%f, %f, %f\n", projectile.position.x, projectile.position.y, projectile.position.z);
        tick(&environment, &projectile);
    }
    return 0;
}
