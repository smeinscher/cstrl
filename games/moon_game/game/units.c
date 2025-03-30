#include "units.h"
#include "../helpers/helpers.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_util.h"
#include "physics_wrapper.h"
#include <stdio.h>
#include <stdlib.h>

#define UNIT_SPEED_MODIFIER 0.0005f
#define UNIT_AVOIDANCE_FORCE 0.05f

const float BASE_UNIT_SPEEDS[MAX_UNIT_TYPES] = {0.25f, 0.25f, 0.0f, 2.0f, 10.0f, 10.0f, 1.0f};

const float BASE_UNIT_VIEW_DISTANCES[MAX_RAY_DIRECTIONS] = {UNIT_SIZE_X * 4.0f, UNIT_SIZE_X * 4.0f, UNIT_SIZE_X * 3.0f,
                                                            UNIT_SIZE_X * 2.0f, UNIT_SIZE_X * 2.0f, UNIT_SIZE_X * 2.0f,
                                                            UNIT_SIZE_X * 3.0f, UNIT_SIZE_X * 4.0f};

const float BASE_UNIT_HEALTH[MAX_UNIT_TYPES] = {10.0f, 10.0f, 1.0f, 75.0f, 75.0f, 75.0f, 150.0f};
const float BASE_UNIT_ATTACK[MAX_UNIT_TYPES] = {1.0f, 2.0f, 0.0f, 5.0f, 20.0f, 20.0f, 20.0f};

static void recalculate_ray_directions(units_t *units, int unit_id, vec3 direction)
{
    vec3 normalized_position = cstrl_vec3_normalize(units->position[unit_id]);
    units->rays[unit_id].base_rays[FORWARD] = direction;
    units->rays[unit_id].base_rays[BACK] = cstrl_vec3_negate(units->rays[unit_id].base_rays[FORWARD]);
    units->rays[unit_id].base_rays[RIGHT] = cstrl_vec3_normalize(cstrl_vec3_cross(direction, normalized_position));
    units->rays[unit_id].base_rays[LEFT] = cstrl_vec3_negate(units->rays[unit_id].base_rays[RIGHT]);
    units->rays[unit_id].base_rays[FORWARD_RIGHT] = cstrl_vec3_normalize(
        cstrl_vec3_add(units->rays[unit_id].base_rays[RIGHT], units->rays[unit_id].base_rays[FORWARD]));
    units->rays[unit_id].base_rays[FORWARD_LEFT] = cstrl_vec3_normalize(
        cstrl_vec3_add(units->rays[unit_id].base_rays[LEFT], units->rays[unit_id].base_rays[FORWARD]));
    units->rays[unit_id].base_rays[BACK_RIGHT] = cstrl_vec3_normalize(
        cstrl_vec3_add(units->rays[unit_id].base_rays[RIGHT], units->rays[unit_id].base_rays[BACK]));
    units->rays[unit_id].base_rays[BACK_LEFT] = cstrl_vec3_normalize(
        cstrl_vec3_add(units->rays[unit_id].base_rays[LEFT], units->rays[unit_id].base_rays[BACK]));
}

bool units_init(units_t *units)
{
    units->count = 0;
    units->capacity = 1;
    units->position = NULL;
    units->active = NULL;
    units->formation_id = NULL;
    units->type = NULL;
    units->aabb_min = NULL;
    units->aabb_max = NULL;
    units->collision_id = NULL;
    units->rays = NULL;
    units->velocity = NULL;
    units->stats = NULL;
    units->attacking = NULL;
    units->last_attack_time = NULL;

    cstrl_da_int_init(&units->free_ids, 1);

    units->position = malloc(sizeof(vec3));
    if (!units->position)
    {
        printf("Error allocating memory for unit positions\n");
        units_free(units);
        return false;
    }
    units->active = malloc(sizeof(bool));
    if (!units->active)
    {
        printf("Error allocating memory for unit active\n");
        units_free(units);
        return false;
    }
    units->formation_id = malloc(sizeof(int));
    if (!units->formation_id)
    {
        printf("Error allocating memory for unit formation ids\n");
        units_free(units);
        return false;
    }
    units->type = malloc(sizeof(int));
    if (!units->type)
    {
        printf("Error allocating memory for unit type\n");
        units_free(units);
        return false;
    }
    units->aabb_min = malloc(sizeof(vec3));
    if (!units->aabb_min)
    {
        printf("Error allocating memory for unit aabb min\n");
        units_free(units);
        return false;
    }
    units->aabb_max = malloc(sizeof(vec3));
    if (!units->aabb_max)
    {
        printf("Error allocating memory for unit aabb max\n");
        units_free(units);
        return false;
    }
    units->collision_id = malloc(sizeof(int));
    if (!units->collision_id)
    {
        printf("Error allocating memory for unit collision id\n");
        units_free(units);
        return false;
    }
    units->rays = malloc(sizeof(unit_rays_t));
    if (!units->rays)
    {
        printf("Error allocating memory for unit rays\n");
        units_free(units);
        return false;
    }
    units->stats = malloc(sizeof(unit_stats_t));
    if (!units->stats)
    {
        printf("Error allocating memory for unit stats\n");
        units_free(units);
        return false;
    }
    units->velocity = malloc(sizeof(vec3));
    if (!units->velocity)
    {
        printf("Error allocating memory for unit velocity\n");
        units_free(units);
        return false;
    }
    units->attacking = malloc(sizeof(bool));
    if (!units->attacking)
    {
        printf("Error allocating memory for unit attacking\n");
        units_free(units);
        return false;
    }
    units->last_attack_time = malloc(sizeof(double));
    if (!units->last_attack_time)
    {
        printf("Error allocating memory for unit last attack time\n");
        units_free(units);
        return false;
    }

    return true;
}

int units_hit(units_t *units, vec3 position)
{
    float new_x0 = position.x - UNIT_SIZE.x / 2.0f;
    float new_x1 = position.x + UNIT_SIZE.x / 2.0f;
    float new_y0 = position.y - UNIT_SIZE.y / 2.0f;
    float new_y1 = position.y + UNIT_SIZE.y / 2.0f;
    // float new_z = position.z;
    for (int i = 0; i < units->count; i++)
    {
        float taken_x0 = units->position[i].x - UNIT_SIZE.x / 2.0f;
        float taken_x1 = units->position[i].x + UNIT_SIZE.x / 2.0f;
        float taken_y0 = units->position[i].y - UNIT_SIZE.y / 2.0f;
        float taken_y1 = units->position[i].y + UNIT_SIZE.y / 2.0f;
        // float taken_z = units->position[i].z;

        bool same_side = cstrl_vec3_dot(position, units->position[i]) > 0.0f;
        if ((new_x0 < taken_x1 && new_x1 > taken_x0 && new_y0 < taken_y1 && new_y1 > taken_y0) && same_side)
        {
            return i;
        }
    }
    return -1;
}

int units_add(units_t *units, int player_id, vec3 position, int type)
{
    int new_id = 0;
    if (units->free_ids.size == 0)
    {
        new_id = units->count++;
        if (units->count > units->capacity)
        {
            units->capacity *= 2;
            if (!cstrl_realloc_vec3(&units->position, units->capacity))
            {
                printf("Error allocating unit transforms\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&units->active, units->capacity))
            {
                printf("Error allocating unit active status\n");
                return -1;
            }
            if (!cstrl_realloc_int(&units->formation_id, units->capacity))
            {
                printf("Error allocating unit formation_ids\n");
                return -1;
            }
            if (!cstrl_realloc_int(&units->type, units->capacity))
            {
                printf("Error allocating unit type\n");
                return -1;
            }
            if (!cstrl_realloc_vec3(&units->aabb_min, units->capacity))
            {
                printf("Error allocating unit aabb min\n");
                return -1;
            }
            if (!cstrl_realloc_vec3(&units->aabb_max, units->capacity))
            {
                printf("Error allocating unit aabb max\n");
                return -1;
            }
            if (!cstrl_realloc_int(&units->collision_id, units->capacity))
            {
                printf("Error allocating unit collision id\n");
                return -1;
            }
            {
                unit_rays_t *temp = realloc(units->rays, units->capacity * sizeof(unit_rays_t));
                if (!temp)
                {
                    printf("Error allocating unit ray directions\n");
                    return -1;
                }
                units->rays = temp;
            }
            {
                unit_stats_t *temp = realloc(units->stats, units->capacity * sizeof(unit_stats_t));
                if (!temp)
                {
                    printf("Error allocating unit stats\n");
                    return -1;
                }
                units->stats = temp;
            }
            if (!cstrl_realloc_vec3(&units->velocity, units->capacity))
            {
                printf("Error allocating unit velocity\n");
                return -1;
            }
            if (!cstrl_realloc_bool(&units->attacking, units->capacity))
            {
                printf("Error allocating unit attacking\n");
                return -1;
            }
            if (!cstrl_realloc_double(&units->last_attack_time, units->capacity))
            {
                printf("Error allocating unit last attack time\n");
                return -1;
            }
        }
    }
    else
    {
        new_id = cstrl_da_int_pop_back(&units->free_ids);
    }

    units->position[new_id] = position;
    units->active[new_id] = true;
    units->formation_id[new_id] = -1;
    units->type[new_id] = type;
    units->aabb_min[new_id] =
        cstrl_vec3_sub(position, (vec3){UNIT_SIZE_X * 0.5f, UNIT_SIZE_Y * 0.5f, UNIT_SIZE_X * 0.5f});
    units->aabb_max[new_id] =
        cstrl_vec3_add(position, (vec3){UNIT_SIZE_X * 0.5f, UNIT_SIZE_Y * 0.5f, UNIT_SIZE_X * 0.5f});
    collision_object_user_data_t *user_data = malloc(sizeof(collision_object_user_data_t));
    user_data->player_id = player_id;
    user_data->type = COLLISION_UNIT;
    user_data->id = new_id;
    units->collision_id[new_id] = insert_aabb(user_data, (vec3[]){units->aabb_min[new_id], units->aabb_max[new_id]});
    recalculate_ray_directions(units, new_id, (vec3){0.0f, 1.0f, 0.0f});
    units->velocity[new_id] = (vec3){0.0f, 0.0f, 0.0f};
    units->stats[new_id].max_health = BASE_UNIT_HEALTH[type];
    units->stats[new_id].current_health = BASE_UNIT_HEALTH[type];
    units->stats[new_id].attack_strength = BASE_UNIT_ATTACK[type];
    units->attacking[new_id] = false;
    units->last_attack_time[new_id] = 0.0;
    return new_id;
}

bool units_move(units_t *units, int unit_id, vec3 target_position)
{
    vec3 start_position = units->position[unit_id];
    if (get_spherical_path_length(start_position, target_position) <
        BASE_UNIT_SPEEDS[units->type[unit_id]] * UNIT_SPEED_MODIFIER * 1.5f)
    {
        units->velocity[unit_id] = (vec3){0.0f, 0.0f, 0.0f};
        return true;
    }
    vec3 new_position = cstrl_vec3_normalize(cstrl_vec3_add(
        start_position, cstrl_vec3_mult_scalar(units->velocity[unit_id],
                                               BASE_UNIT_SPEEDS[units->type[unit_id]] * UNIT_SPEED_MODIFIER)));
    if (units->type[unit_id] != JET && units->type[unit_id] != PLANE)
    {
        new_position = cstrl_vec3_mult_scalar(new_position, 1.0f + UNIT_SIZE_X * 0.5f);
    }
    else
    {
        new_position = cstrl_vec3_mult_scalar(new_position, 1.0f + UNIT_SIZE_X * 5.0f);
    }
    units->position[unit_id] = new_position;

    units_update_aabb(units, unit_id);
    return false;
}

void units_remove(units_t *units, int unit_id)
{
    units->position[unit_id] = (vec3){0.0f, 0.0f, 0.0f};
    units->active[unit_id] = false;
    cstrl_da_int_push_back(&units->free_ids, unit_id);
    remove_aabb(units->collision_id[unit_id]);
}

void units_free(units_t *units)
{
    units->count = 0;
    units->capacity = 0;
    free(units->position);
    free(units->active);
    free(units->formation_id);
    free(units->type);
    free(units->aabb_min);
    free(units->aabb_max);
    free(units->collision_id);
    free(units->rays);
    free(units->velocity);
    free(units->stats);
    free(units->attacking);
    free(units->last_attack_time);
    cstrl_da_int_free(&units->free_ids);
}

void units_update_aabb(units_t *units, int unit_id)
{

    units->aabb_min[unit_id] =
        cstrl_vec3_sub(units->position[unit_id], (vec3){UNIT_SIZE_X * 0.5f, UNIT_SIZE_Y * 0.5f, UNIT_SIZE_X * 0.5f});
    units->aabb_max[unit_id] =
        cstrl_vec3_add(units->position[unit_id], (vec3){UNIT_SIZE_X * 0.5f, UNIT_SIZE_Y * 0.5f, UNIT_SIZE_X * 0.5f});
    update_aabb(units->collision_id[unit_id], (vec3[]){units->aabb_min[unit_id], units->aabb_max[unit_id]});
}
