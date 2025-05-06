#ifndef BALL_H
#define BALL_H

#include "cstrl/cstrl_math.h"
#include "cup.h"
#include "player.h"
#define MAX_BALLS 2

#define BALL_SIZE 4.0f

#define INITIAL_BALL_SPEED 2.0f
#define SIMULATION_BALL_SPEED 20.0f
#define INITIAL_BOUNCE_DISTANCE 20

typedef struct balls_t
{
    da_int cups_hit[MAX_BALLS];
    vec2 position[MAX_BALLS];
    vec2 target[MAX_BALLS];
    vec2 origin[MAX_BALLS];
    vec2 error[MAX_BALLS];
    vec2 velocity[MAX_BALLS];
    float speed[MAX_BALLS];
    float bounce_length[MAX_BALLS];
    float angle[MAX_BALLS];
    float cup_hit_distance[MAX_BALLS];
    int cup_made[MAX_BALLS];
    int team[MAX_BALLS];
    bool shot_complete[MAX_BALLS];
    bool active[MAX_BALLS];
} balls_t;

void balls_init(balls_t *balls);

void balls_shoot(balls_t *balls, vec2 target, vec2 origin, vec2 error, float speed, int team);

void balls_update(balls_t *balls, cups_t *cups, players_t *players);

void balls_clear(balls_t *balls);

void balls_free(balls_t *balls);

#endif
