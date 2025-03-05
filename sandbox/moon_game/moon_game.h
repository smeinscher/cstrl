#ifndef PLANET_H
#define PLANET_H

#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"
#include "game/ai.h"
#include "game/players.h"
#include <stdbool.h>
#if defined(CSTRL_MOON_GAME_EXPORT)
#if defined(_MSC_VER)
#define GAME_API _declspec(dllexport)
#else
#define GAME_API __attribute__((visibility("default")))
#endif
#else
#if defined(_MSC_VER)
#define GAME_API __declspec(dllimport)
#else
#define GAME_API
#endif
#endif

typedef struct game_camera_objects_t
{
    cstrl_camera *main_camera;
    cstrl_camera *ui_camera;
} game_camera_objects_t;

typedef struct game_mouse_data_t
{
    int position_x;
    int position_y;
    int last_position_x;
    int last_position_y;
} game_mouse_data_t;

typedef struct game_unit_selection_data_t
{
    int current_selection_type;
    vec2 selection_start;
    vec2 selection_end;
} game_unit_selection_data_t;

typedef struct game_player_data_t
{
    int count;
    int human_player;
    players_t players;
    ai_t ai;
} game_player_data_t;

typedef struct game_state_t
{
    bool moving_planet;
    game_mouse_data_t mouse_data;
    game_unit_selection_data_t unit_selection_data;
    game_player_data_t player_data;
} game_state_t;

typedef struct render_toggles_t
{
    bool render_path_markers;
    bool render_path_lines;
    bool border_update;
    bool physics_debug_draw_enabled;
    bool main_view_projection_update;
    bool ui_view_projection_update;
    bool render_planet;
} render_toggles_t;

typedef struct render_state_t
{
    game_camera_objects_t camera_objects;
    render_toggles_t toggles;
    cstrl_render_data **render_data;
    cstrl_shader *shaders;
    cstrl_texture *textures;
    // TODO: find another way to organize these
    unsigned int view_projection_ubo;
    unsigned int fbo;
    unsigned int rbo;
    unsigned int framebuffer_vao;
    cstrl_ui_context ui_context;

} render_state_t;

GAME_API int moon_game();

GAME_API int moon_game_init(render_state_t *render_state, game_state_t *game_state, cstrl_platform_state *platform_state);

GAME_API int moon_game_update(game_state_t *game_state, cstrl_platform_state *platform_state, double *previous_time, double *lag);

GAME_API int moon_game_render(render_state_t *render_state, game_state_t *game_state, cstrl_platform_state *platform_state);

GAME_API void moon_game_shutdown(render_state_t *render_state, game_state_t *game_state, cstrl_platform_state *platform_state);

#endif // PLANET_H
