#ifndef UNITS_H
#define UNITS_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"

#define UNIT_SIZE_X 0.075f
#define UNIT_SIZE_Y 0.075f
#define UNIT_SIZE_Z 0.0f
#define UNIT_SIZE (vec3){UNIT_SIZE_X, UNIT_SIZE_Y, UNIT_SIZE_Z}

typedef enum team_t
{
    RED,
    BLUE,
    GREEN,
    YELLOW,
    WHITE,
    CYAN,
    PURPLE,
    NO_TEAM
} team_t;

typedef enum unit_type
{
    ASTRONAUT,
    ASTRONAUT_ARMED,
    CITY,
    HUMVEE,
    JET,
    PLANE,
    TANK,
    MAX_UNIT_TYPES
} unit_type;

typedef enum ray_directions_t
{
    FORWARD,
    FORWARD_LEFT,
    LEFT,
    BACK_LEFT,
    BACK,
    BACK_RIGHT,
    RIGHT,
    FORWARD_RIGHT,
    MAX_RAY_DIRECTIONS
} ray_directions_t;

extern const float BASE_UNIT_SPEEDS[MAX_UNIT_TYPES];
extern const float BASE_UNIT_VIEW_DISTANCES[MAX_RAY_DIRECTIONS];
extern const float BASE_UNIT_HEALTH[MAX_UNIT_TYPES];
extern const float BASE_UNIT_ATTACK[MAX_UNIT_TYPES];

typedef struct unit_rays_t
{
    vec3 base_rays[MAX_RAY_DIRECTIONS];
    float view_distances[MAX_RAY_DIRECTIONS];
} unit_rays_t;

typedef struct unit_stats_t
{
    float max_health;
    float current_health;
    float attack_strength;
} unit_stats_t;

typedef struct unit_data_t
{
    int player_id;
    int unit_id;
} unit_data_t;

typedef struct units_t
{
    size_t count;
    size_t capacity;
    vec3 *position;
    bool *active;
    int *formation_id;
    int *type;
    vec3 *aabb_min;
    vec3 *aabb_max;
    int *collision_id;
    unit_rays_t *rays;
    vec3 *velocity;
    unit_stats_t *stats;
    bool *attacking;
    double *last_attack_time;
    da_int free_ids;
} units_t;

bool units_init(units_t *units);

int units_hit(units_t *units, vec3 position);

int units_add(units_t *units, int player_id, vec3 positions, int type);

bool units_move(units_t *units, int unit_id, vec3 target_position);

void units_remove(units_t *units, int unit_id);

void units_free(units_t *units);

void units_update_aabb(units_t *units, int unit_id);

#endif // UNITS_H
