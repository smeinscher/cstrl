#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "../entities/player.h"
#include <stdbool.h>

void gameplay_key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods);

void gameplay_mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos);

void gameplay_mouse_button_callback(cstrl_platform_state *state, int button, int action, int mods);

bool gameplay_show_cursor();

void gameplay_get_mouse_position(int *mouse_x, int *mouse_y);

void gameplay_toggle_pause();

void gameplay_reset();

int gameplay_get_current_player_turn();

player_stats_t gameplay_get_player_stats(int player_id);

player_metrics_t gameplay_get_player_metrics(int player_id);

void gameplay_get_team_wins(int *team1, int *team2);

bool gameplay_team_can_rerack(int team);

void gameplay_team_rerack(int team);

void gameplay_init(int base_screen_x, int base_screen_y, cstrl_shader *default_shader, bool simulation, int human_players, player_stats_t *stats);

void gameplay_update();

void gameplay_render();

void gameplay_shutdown();

#endif // GAMEPLAY_H
