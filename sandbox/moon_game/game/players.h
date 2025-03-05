#ifndef PLAYERS_H
#define PLAYERS_H

#include "cstrl/cstrl_camera.h"
#include "formation.h"
#include "paths.h"
#include "projectile.h"
#include "units.h"
#include <stdbool.h>
#include <stddef.h>

#define MAX_PLAYER_COUNT 8

typedef struct players_t
{
    bool active[MAX_PLAYER_COUNT];
    float score[MAX_PLAYER_COUNT];
    int selected_formation[MAX_PLAYER_COUNT];
    units_t units[MAX_PLAYER_COUNT];
    formations_t formations[MAX_PLAYER_COUNT];
    paths_t paths[MAX_PLAYER_COUNT];
    da_int selected_units[MAX_PLAYER_COUNT];
    projectiles_t projectiles[MAX_PLAYER_COUNT];
    bool at_war[MAX_PLAYER_COUNT][MAX_PLAYER_COUNT];
} players_t;

void players_set_units_in_formation_selected(players_t *players, int player_id);

void players_add_selected_units_to_formation(players_t *players, int player_id);

bool players_init(players_t *players, int count);

void players_update(players_t *players, int player_id);

void players_move_units_normal_mode(players_t *players, int player_id, vec3 end_position);

void players_move_units_path_mode(players_t *players, int player_id, vec3 end_position);

bool players_select_units(players_t *players, int player_id, int viewport_width, int viewport_height,
                          vec2 selection_start, vec2 selection_end, cstrl_camera *camera, int selection_type);

void players_free(players_t *players);

#endif
