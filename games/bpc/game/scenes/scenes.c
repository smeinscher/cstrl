#include "scenes.h"
#include "../entities/ball.h"
#include "../entities/cup.h"
#include "../entities/player.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_ui.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>

#define BASE_SCREEN_DIMENSION_X 320
#define BASE_SCREEN_DIMENSION_Y 180

static cstrl_platform_state *g_platform_state;

static cstrl_shader g_default_shader;
static cstrl_camera *g_main_camera;

static cstrl_render_data *g_cursor_render_data;
static cstrl_texture g_cursor_texture;

void scenes_init(cstrl_platform_state *platform_state)
{
    g_platform_state = platform_state;
    cstrl_platform_set_show_cursor(g_platform_state, false);
    g_default_shader =
        cstrl_load_shaders_from_files("resources/shaders/default.vert", "resources/shaders/default.frag");
    int window_width, window_height;
    cstrl_platform_get_window_size(g_platform_state, &window_width, &window_height);
    g_cursor_render_data = cstrl_renderer_create_render_data();
    float cursor_positions[12];
    float cursor_size = 25.0f;
    float cursorx0 = (float)window_width / 2.0f - cursor_size / 2.0f;
    float cursory0 = (float)window_height / 2.0f - cursor_size / 2.0f;
    float cursorx1 = (float)window_width / 2.0f + cursor_size / 2.0f;
    float cursory1 = (float)window_height / 2.0f + cursor_size / 2.0f;
    cursor_positions[0] = cursorx0;
    cursor_positions[1] = cursory1;
    cursor_positions[2] = cursorx1;
    cursor_positions[3] = cursory0;
    cursor_positions[4] = cursorx0;
    cursor_positions[5] = cursory0;
    cursor_positions[6] = cursorx0;
    cursor_positions[7] = cursory1;
    cursor_positions[8] = cursorx1;
    cursor_positions[9] = cursory0;
    cursor_positions[10] = cursorx1;
    cursor_positions[11] = cursory1;
    float cursor_uvs[12];
    cursor_uvs[0] = 0.0f;
    cursor_uvs[1] = 1.0f;
    cursor_uvs[2] = 1.0f;
    cursor_uvs[3] = 0.0f;
    cursor_uvs[4] = 0.0f;
    cursor_uvs[5] = 0.0f;
    cursor_uvs[6] = 0.0f;
    cursor_uvs[7] = 1.0f;
    cursor_uvs[8] = 1.0f;
    cursor_uvs[9] = 0.0f;
    cursor_uvs[10] = 1.0f;
    cursor_uvs[11] = 1.0f;
    float cursor_colors[24];
    for (int i = 0; i < 6; i++)
    {
        cursor_colors[i * 4] = 1.0f;
        cursor_colors[i * 4 + 1] = 1.0f;
        cursor_colors[i * 4 + 2] = 1.0f;
        cursor_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(g_cursor_render_data, cursor_positions, 2, 6);
    cstrl_renderer_add_uvs(g_cursor_render_data, cursor_uvs);
    cstrl_renderer_add_colors(g_cursor_render_data, cursor_colors);

    g_cursor_texture =
        cstrl_texture_generate_from_path("resources/textures/beer_pong/cursor.png", CSTRL_TEXTURE_FILTER_NEAREST);

    g_main_camera = cstrl_camera_create(BASE_SCREEN_DIMENSION_X, BASE_SCREEN_DIMENSION_Y, true);
    cstrl_camera_update(g_main_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    cstrl_set_uniform_mat4(g_default_shader.program, "view", g_main_camera->view);
    cstrl_set_uniform_mat4(g_default_shader.program, "projection", g_main_camera->projection);
}

/*
 *
 *  MAIN MENU SCENE
 *
 */

static void main_menu_key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    if (key == CSTRL_KEY_ESCAPE)
    {
        cstrl_platform_set_should_exit(true);
    }
}

void main_menu_scene_init(void *user_data)
{
    cstrl_platform_set_key_callback(g_platform_state, main_menu_key_callback);
}

void main_menu_scene_update()
{
}

void main_menu_scene_render()
{
}

void main_menu_scene_shutdown()
{
}

/*
 *
 *  MAIN GAME SCENE
 */

#define CURSOR_SIZE 8.0f
#define TARGET_SIZE 3.0f
#define METER_SIZE_X 2.0f
#define METER_SIZE_Y 20.0f
#define METER_BAR_SIZE_X 3.125f
#define METER_BAR_SIZE_Y 1.0f

#define PLAYER_SHOT_BOUND_MIN_X 40
#define PLAYER_SHOT_BOUND_MAX_X 90
#define PLAYER_SHOT_BOUND_MIN_Y 60
#define PLAYER_SHOT_BOUND_MAX_Y 120

#define TEAM1_SECTION_MIN_X 230
#define TEAM1_SECTION_MAX_X 280
#define TEAM1_SECTION_MIN_Y 60
#define TEAM1_SECTION_MAX_Y 120
#define TEAM2_SECTION_MIN_X 40
#define TEAM2_SECTION_MAX_X 90
#define TEAM2_SECTION_MIN_Y 60
#define TEAM2_SECTION_MAX_Y 120

#define MAKE_RADIUS (CUP_SIZE / 5.0f)
#define MAKE_RADIUS_BOUNCE (CUP_SIZE / 6.0f)

static int g_mouse_x;
static int g_mouse_y;

static cstrl_render_data *g_background_render_data;
static cstrl_render_data *g_cup_render_data;
static cstrl_render_data *g_ball_render_data;
static cstrl_render_data *g_target_render_data;
static cstrl_render_data *g_meter_render_data;
static cstrl_render_data *g_meter_bar_render_data;
static cstrl_texture g_background_texture;
static cstrl_texture g_cup_texture;
static cstrl_texture g_ball_texture;
static cstrl_texture g_target_texture;
static cstrl_texture g_meter_texture;
static cstrl_texture g_meter_bar_texture;

static cups_t g_cups;
static players_t g_players;
static balls_t g_balls;

static vec2 g_team1_start;
static vec2 g_team2_start;
static vec2 g_target_position;
static vec2 g_ball_origin;
static float g_target_error;

static bool g_cleared_target_and_meter = false;
static bool g_cleared_ball = false;
static bool g_reset_game_update = false;
static bool g_reset_game_render = false;
static bool g_overtime_init_update = false;
static bool g_overtime_init_render = false;

static cstrl_ui_context g_ui_context;
static cstrl_ui_layout g_base_layout;
static cstrl_ui_layout g_sub_layout;

static bool g_mouse_in_shot_area = false;

static bool g_paused = false;

static int g_tick_counter = 0;

static bool g_transition = false;

static void main_game_key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    if (key == CSTRL_KEY_ESCAPE)
    {
        cstrl_platform_set_should_exit(true);
    }
}

static void main_game_mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos)
{
    g_mouse_x = xpos / 4;
    g_mouse_y = ypos / 4;
}

static void main_game_mouse_button_callback(cstrl_platform_state *state, int button, int action, int mods)
{
    if (button == CSTRL_MOUSE_BUTTON_LEFT)
    {
        if (action == CSTRL_ACTION_PRESS && g_mouse_in_shot_area)
        {
            if (g_players.human[g_players.current_player_turn] && g_players.current_turn_state != STARTED_SHOT)
            {
                players_advance_turn_state(&g_players);
                if (g_players.current_turn_state == STARTED_SHOT)
                {
                    g_cleared_target_and_meter = false;
                }
            }
        }
    }
}

void main_game_scene_init(void *user_data)
{
#ifdef SIMULATION
    printf("Simulation Mode\n");
#endif
    cstrl_platform_set_key_callback(g_platform_state, main_game_key_callback);
    cstrl_platform_set_mouse_position_callback(g_platform_state, main_game_mouse_position_callback);
    cstrl_platform_set_mouse_button_callback(g_platform_state, main_game_mouse_button_callback);

    g_background_render_data = cstrl_renderer_create_render_data();
    // x0 = 256.0f;
    // y0 = 264.0f;
    // x1 = 1024.0f;
    // y1 = 456.0f;
    float x0 = 0.0f;
    float y0 = 0.0f;
    float x1 = BASE_SCREEN_DIMENSION_X;
    float y1 = BASE_SCREEN_DIMENSION_Y;
    float table_positions[] = {x0, y1, x1, y0, x0, y0, x0, y1, x1, y0, x1, y1};
    float table_uvs[] = {
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    };
    float table_colors[24];
    for (int i = 0; i < 6; i++)
    {
        table_colors[i * 4] = 1.0f;
        table_colors[i * 4 + 1] = 1.0f;
        table_colors[i * 4 + 2] = 1.0f;
        table_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(g_background_render_data, table_positions, 2, 6);
    cstrl_renderer_add_uvs(g_background_render_data, table_uvs);
    cstrl_renderer_add_colors(g_background_render_data, table_colors);

    g_background_texture =
        cstrl_texture_generate_from_path("resources/textures/beer_pong/bar.png", CSTRL_TEXTURE_FILTER_NEAREST);

    g_cup_render_data = cstrl_renderer_create_render_data();
    cups_init(&g_cups, false);
    float cup_positions[240];
    for (int i = 0; i < 20; i++)
    {
        float cupx0 = g_cups.position[i].x - CUP_SIZE / 2.0f;
        float cupy0 = g_cups.position[i].y - CUP_SIZE / 2.0f;
        float cupx1 = g_cups.position[i].x + CUP_SIZE / 2.0f;
        float cupy1 = g_cups.position[i].y + CUP_SIZE / 2.0f;
        cup_positions[i * 12] = cupx0;
        cup_positions[i * 12 + 1] = cupy1;
        cup_positions[i * 12 + 2] = cupx1;
        cup_positions[i * 12 + 3] = cupy0;
        cup_positions[i * 12 + 4] = cupx0;
        cup_positions[i * 12 + 5] = cupy0;
        cup_positions[i * 12 + 6] = cupx0;
        cup_positions[i * 12 + 7] = cupy1;
        cup_positions[i * 12 + 8] = cupx1;
        cup_positions[i * 12 + 9] = cupy0;
        cup_positions[i * 12 + 10] = cupx1;
        cup_positions[i * 12 + 11] = cupy1;
    }
    float cup_uvs[240];
    for (int i = 0; i < 20; i++)
    {
        cup_uvs[i * 12] = 0.0f;
        cup_uvs[i * 12 + 1] = 1.0f;
        cup_uvs[i * 12 + 2] = 1.0f;
        cup_uvs[i * 12 + 3] = 0.0f;
        cup_uvs[i * 12 + 4] = 0.0f;
        cup_uvs[i * 12 + 5] = 0.0f;
        cup_uvs[i * 12 + 6] = 0.0f;
        cup_uvs[i * 12 + 7] = 1.0f;
        cup_uvs[i * 12 + 8] = 1.0f;
        cup_uvs[i * 12 + 9] = 0.0f;
        cup_uvs[i * 12 + 10] = 1.0f;
        cup_uvs[i * 12 + 11] = 1.0f;
    }
    float cup_colors[24 * 20];
    for (int i = 0; i < 6 * 20; i++)
    {
        cup_colors[i * 4] = 1.0f;
        cup_colors[i * 4 + 1] = 1.0f;
        cup_colors[i * 4 + 2] = 1.0f;
        cup_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(g_cup_render_data, cup_positions, 2, 120);
    cstrl_renderer_add_uvs(g_cup_render_data, cup_uvs);
    cstrl_renderer_add_colors(g_cup_render_data, cup_colors);

    g_cup_texture =
        cstrl_texture_generate_from_path("resources/textures/beer_pong/cup.png", CSTRL_TEXTURE_FILTER_NEAREST);

    g_ball_render_data = cstrl_renderer_create_render_data();
    float ball_positions[MAX_BALLS * 12] = {0};
    float ball_uvs[MAX_BALLS * 12];
    for (int i = 0; i < MAX_BALLS; i++)
    {
        ball_uvs[i * 12] = 0.0f;
        ball_uvs[i * 12 + 1] = 1.0f;
        ball_uvs[i * 12 + 2] = 1.0f;
        ball_uvs[i * 12 + 3] = 0.0f;
        ball_uvs[i * 12 + 4] = 0.0f;
        ball_uvs[i * 12 + 5] = 0.0f;
        ball_uvs[i * 12 + 6] = 0.0f;
        ball_uvs[i * 12 + 7] = 1.0f;
        ball_uvs[i * 12 + 8] = 1.0f;
        ball_uvs[i * 12 + 9] = 0.0f;
        ball_uvs[i * 12 + 10] = 1.0f;
        ball_uvs[i * 12 + 11] = 1.0f;
    }
    float ball_colors[MAX_BALLS * 24];
    for (int i = 0; i < MAX_BALLS * 6; i++)
    {
        ball_colors[i * 4] = 1.0f;
        ball_colors[i * 4 + 1] = 1.0f;
        ball_colors[i * 4 + 2] = 1.0f;
        ball_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(g_ball_render_data, ball_positions, 2, MAX_BALLS * 6);
    cstrl_renderer_add_uvs(g_ball_render_data, ball_uvs);
    cstrl_renderer_add_colors(g_ball_render_data, ball_colors);

    g_ball_texture =
        cstrl_texture_generate_from_path("resources/textures/beer_pong/ball.png", CSTRL_TEXTURE_FILTER_NEAREST);

    g_target_render_data = cstrl_renderer_create_render_data();
    float target_positions[12] = {0};
    float target_size = 12.5f;
    float target_uvs[12];
    target_uvs[0] = 0.0f;
    target_uvs[1] = 1.0f;
    target_uvs[2] = 1.0f;
    target_uvs[3] = 0.0f;
    target_uvs[4] = 0.0f;
    target_uvs[5] = 0.0f;
    target_uvs[6] = 0.0f;
    target_uvs[7] = 1.0f;
    target_uvs[8] = 1.0f;
    target_uvs[9] = 0.0f;
    target_uvs[10] = 1.0f;
    target_uvs[11] = 1.0f;
    float target_colors[24];
    for (int i = 0; i < 6; i++)
    {
        target_colors[i * 4] = 1.0f;
        target_colors[i * 4 + 1] = 1.0f;
        target_colors[i * 4 + 2] = 1.0f;
        target_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(g_target_render_data, target_positions, 2, 6);
    cstrl_renderer_add_uvs(g_target_render_data, target_uvs);
    cstrl_renderer_add_colors(g_target_render_data, target_colors);

    g_target_texture =
        cstrl_texture_generate_from_path("resources/textures/beer_pong/target.png", CSTRL_TEXTURE_FILTER_NEAREST);

    g_meter_render_data = cstrl_renderer_create_render_data();
    float meter_positions[12] = {0};
    float meter_uvs[12];
    meter_uvs[0] = 0.0f;
    meter_uvs[1] = 1.0f;
    meter_uvs[2] = 1.0f;
    meter_uvs[3] = 0.0f;
    meter_uvs[4] = 0.0f;
    meter_uvs[5] = 0.0f;
    meter_uvs[6] = 0.0f;
    meter_uvs[7] = 1.0f;
    meter_uvs[8] = 1.0f;
    meter_uvs[9] = 0.0f;
    meter_uvs[10] = 1.0f;
    meter_uvs[11] = 1.0f;
    float meter_colors[24];
    for (int i = 0; i < 6; i++)
    {
        meter_colors[i * 4] = 1.0f;
        meter_colors[i * 4 + 1] = 1.0f;
        meter_colors[i * 4 + 2] = 1.0f;
        meter_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(g_meter_render_data, meter_positions, 2, 6);
    cstrl_renderer_add_uvs(g_meter_render_data, meter_uvs);
    cstrl_renderer_add_colors(g_meter_render_data, meter_colors);

    g_meter_texture =
        cstrl_texture_generate_from_path("resources/textures/beer_pong/meter.png", CSTRL_TEXTURE_FILTER_NEAREST);

    g_meter_bar_render_data = cstrl_renderer_create_render_data();
    float meter_bar_positions[12] = {0};
    float meter_bar_uvs[12];
    meter_bar_uvs[0] = 0.0f;
    meter_bar_uvs[1] = 1.0f;
    meter_bar_uvs[2] = 1.0f;
    meter_bar_uvs[3] = 0.0f;
    meter_bar_uvs[4] = 0.0f;
    meter_bar_uvs[5] = 0.0f;
    meter_bar_uvs[6] = 0.0f;
    meter_bar_uvs[7] = 1.0f;
    meter_bar_uvs[8] = 1.0f;
    meter_bar_uvs[9] = 0.0f;
    meter_bar_uvs[10] = 1.0f;
    meter_bar_uvs[11] = 1.0f;
    float meter_bar_colors[24];
    for (int i = 0; i < 6; i++)
    {
        meter_bar_colors[i * 4] = 1.0f;
        meter_bar_colors[i * 4 + 1] = 1.0f;
        meter_bar_colors[i * 4 + 2] = 1.0f;
        meter_bar_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(g_meter_bar_render_data, meter_bar_positions, 2, 6);
    cstrl_renderer_add_uvs(g_meter_bar_render_data, meter_bar_uvs);
    cstrl_renderer_add_colors(g_meter_bar_render_data, meter_bar_colors);

    g_meter_bar_texture =
        cstrl_texture_generate_from_path("resources/textures/beer_pong/meter_bar.png", CSTRL_TEXTURE_FILTER_NEAREST);

    players_init(&g_players, true, PLAYER1);
    balls_init(&g_balls);

    float player_size_x = 17.0f;
    float player_size_y = 31.5;
    float playerx0 = 30.0f;
    float playery0 = (float)BASE_SCREEN_DIMENSION_Y / 2.0f - player_size_y / 2.0f;
    float playerx1 = playerx0 + player_size_x;
    float playery1 = (float)BASE_SCREEN_DIMENSION_Y / 2.0f + player_size_y / 2.0f;
    playerx0 = 290.0f;
    playerx1 = playerx0 - player_size_x;

    g_team1_start = (vec2){290.0f, playery0 + player_size_y / 2.0f};
    g_team2_start = (vec2){30.0f, playery0 + player_size_y / 2.0f};

    cstrl_ui_init(&g_ui_context, g_platform_state);

    g_base_layout.border.size = (cstrl_ui_border_size){4, 4, 4, 4, 0};
    g_base_layout.border.color = (cstrl_ui_color){0.0f, 0.0f, 0.0f, 1.0f};
    g_base_layout.border.gap_color = (cstrl_ui_color){0.29f, 0.89f, 0.75f, 1.0f};
    g_base_layout.border.gap_size = 0;
    g_base_layout.color = (cstrl_ui_color){0.43f, 0.18f, 0.29f, 1.0f};
    g_base_layout.font_color = (cstrl_ui_color){0.0f, 0.0f, 0.0f, 1.0f};

    g_sub_layout.border.size = (cstrl_ui_border_size){1, 1, 1, 1, 0};
    g_sub_layout.border.color = (cstrl_ui_color){0.0, 0.0f, 0.0f, 1.0f};
    g_sub_layout.color = (cstrl_ui_color){0.75f, 0.18f, 0.19f, 1.0f};
    g_sub_layout.font_color = (cstrl_ui_color){0.0f, 0.0f, 0.0f, 1.0f};
}

static void main_game_scene_shoot_ball(bool human, int team)
{
    if (!human)
    {
        int selected_cup;
        do
        {
            selected_cup = rand() % 10 + (team * 10);
        } while (cstrl_da_int_find_first(&g_cups.freed, selected_cup) != CSTRL_DA_INT_ITEM_NOT_FOUND);
        vec2 target_position = g_cups.position[selected_cup];
        vec2 target_error = cstrl_vec2_mult_scalar(
            (vec2){(float)(rand() % 1000 - 500) / 2000.0f, (float)(rand() % 1000 - 500) / 2000.0f}, 12.5f);
        balls_shoot(&g_balls, target_position, team == 0 ? g_team1_start : g_team2_start, target_error,
                    INITIAL_BALL_SPEED, team);
    }
    else
    {
        balls_shoot(&g_balls, g_target_position, team == 0 ? g_team1_start : g_team2_start,
                    (vec2){0.0f, g_target_error * 12.5f}, INITIAL_BALL_SPEED, team);
    }
}

static void main_game_scene_eye_to_eye_stage_update()
{
    switch (g_players.current_turn_state)
    {
    case AIM_TARGET:
    case AIM_METER:
        break;
    case STARTED_SHOT:
        if (g_players.current_player_turn == PLAYER1_TURN)
        {
            main_game_scene_shoot_ball(g_players.human[0], 0);
            main_game_scene_shoot_ball(g_players.human[2], 1);
        }
        else
        {
            main_game_scene_shoot_ball(g_players.human[1], 0);
            main_game_scene_shoot_ball(g_players.human[3], 1);
        }
        players_advance_turn_state(&g_players);
        break;
    case SHOOTING:
        balls_update(&g_balls, &g_cups);
        int completed_shots = 0;
        bool team1_make = false;
        bool team2_make = false;
        for (int i = 0; i < MAX_BALLS; i++)
        {
            if (g_balls.shot_complete[i])
            {
                completed_shots++;
                if (g_balls.cup_made[i] >= 0)
                {
                    if (g_balls.team[i] == 0)
                    {
                        team1_make = true;
                    }
                    else
                    {
                        team2_make = true;
                    }
                }
            }
        }
        if (completed_shots >= 2)
        {
            if (team1_make && !team2_make)
            {
                printf("Team 1 Wins First Turn\n");
                g_players.current_turn_state = TURN_END;
                g_players.current_player_turn = PLAYER1_TURN;
                g_transition = true;
            }
            else if (team2_make && !team1_make)
            {
                printf("Team 2 Wins First Turn\n");
                g_players.current_turn_state = TURN_END;
                g_players.current_player_turn = PLAYER3_TURN;
                g_transition = true;
            }
            else
            {
                players_advance_turn_state(&g_players);
            }
            balls_clear(&g_balls);
        }
        break;
    case TURN_END:
#ifndef SIMULATION
        g_tick_counter++;
        if (g_tick_counter > 60)
#endif
        {
            if (g_transition)
            {
                printf("Main Game Stage\n");
                g_players.base_game_state = MAIN_GAME_STAGE;
                g_players.current_turn_state =
                    g_players.human[g_players.current_player_turn] ? AIM_TARGET : STARTED_SHOT;
                g_transition = false;
            }
            else
            {
                players_advance_turn_state(&g_players);
            }
            g_tick_counter = 0;
        }
        break;
    }
}

static void main_game_scene_main_game_stage_update()
{
    switch (g_players.current_turn_state)
    {
    case AIM_TARGET:
    case AIM_METER:
        break;
    case STARTED_SHOT:
        main_game_scene_shoot_ball(g_players.human[g_players.current_player_turn],
                                   g_players.current_player_turn < PLAYER3_TURN ? 0 : 1);
        players_advance_turn_state(&g_players);
        break;
    case SHOOTING:
        balls_update(&g_balls, &g_cups);
        if (g_balls.shot_complete[0])
        {
            if (g_balls.cup_made[0] >= 0)
            {
                float cup_positions[12] = {0};
                for (int i = 0; i < g_balls.cups_hit[0].size; i++)
                {
                    cups_make(&g_cups, g_balls.cups_hit[0].array[i]);
                    cstrl_renderer_modify_positions(g_cup_render_data, cup_positions, g_balls.cups_hit[0].array[i] * 12,
                                                    12);
                }
            }
            player_shot_state_t shot_state = g_balls.cup_made[0] == -1       ? MISS_SHOT
                                             : g_balls.cups_hit[0].size == 1 ? MAKE_SHOT
                                                                             : BOUNCE_SHOT;
            players_complete_shot(&g_players, shot_state, g_balls.cup_made[0] == -1 ? 0 : g_balls.cups_hit[0].size);
            players_advance_turn_state(&g_players);
            g_cleared_ball = false;
            balls_clear(&g_balls);
        }
        break;
    case TURN_END:
#ifndef SIMULATION
        g_tick_counter++;
        if (g_tick_counter > 60)
#endif
        {
            players_advance_turn_state(&g_players);
            g_tick_counter = 0;
        }
        break;
    }
}

void main_game_scene_update()
{
    if (!g_paused)
    {
        if (g_mouse_x < PLAYER_SHOT_BOUND_MAX_X && g_mouse_x > PLAYER_SHOT_BOUND_MIN_X &&
            g_mouse_y < PLAYER_SHOT_BOUND_MAX_Y && g_mouse_y > PLAYER_SHOT_BOUND_MIN_Y)
        {
            g_mouse_in_shot_area = true;
        }
        else
        {
            g_mouse_in_shot_area = false;
        }
        switch (g_players.base_game_state)
        {
        case EYE_TO_EYE_STAGE:
            main_game_scene_eye_to_eye_stage_update();
            break;
        case MAIN_GAME_STAGE:
        case REBUTTAL_ATTEMPT1_STAGE:
        case REBUTTAL_ATTEMPT2_STAGE:
        case OVERTIME_STAGE:
            main_game_scene_main_game_stage_update();
            break;
        case OVERTIME_TRANSITION_STAGE:
            g_overtime_init_update = true;
            g_overtime_init_render = true;
            break;
        case GAME_OVER_STAGE:
            g_reset_game_update = true;
            g_reset_game_render = true;
            break;
        default:
            break;
        }
    }
    if (g_overtime_init_update)
    {
        cups_free(&g_cups);
        cups_init(&g_cups, true);
        g_players.base_game_state = OVERTIME_STAGE;
        g_cleared_target_and_meter = false;
        g_reset_game_update = false;
        balls_clear(&g_balls);
        g_overtime_init_update = false;
    }
    if (g_reset_game_update)
    {
        printf("Resetting Update...\n");
        cups_free(&g_cups);
        cups_init(&g_cups, false);
        players_reset(&g_players);
        g_cleared_target_and_meter = false;
        g_reset_game_update = false;
        balls_clear(&g_balls);
    }
}

static void main_game_scene_main_game_stage_render()
{
    float cursor_positions[12] = {0};
    if (!g_players.human[g_players.current_player_turn] || !g_mouse_in_shot_area ||
        g_players.current_turn_state >= STARTED_SHOT)
    {
        float cursorx0 = (float)g_mouse_x - CURSOR_SIZE / 2.0f;
        float cursory0 = (float)g_mouse_y - CURSOR_SIZE / 2.0f;
        float cursorx1 = (float)g_mouse_x + CURSOR_SIZE / 2.0f;
        float cursory1 = (float)g_mouse_y + CURSOR_SIZE / 2.0f;
        cursor_positions[0] = cursorx0;
        cursor_positions[1] = cursory1;
        cursor_positions[2] = cursorx1;
        cursor_positions[3] = cursory0;
        cursor_positions[4] = cursorx0;
        cursor_positions[5] = cursory0;
        cursor_positions[6] = cursorx0;
        cursor_positions[7] = cursory1;
        cursor_positions[8] = cursorx1;
        cursor_positions[9] = cursory0;
        cursor_positions[10] = cursorx1;
        cursor_positions[11] = cursory1;
    }
    cstrl_renderer_modify_positions(g_cursor_render_data, cursor_positions, 0, 12);
    if (g_players.human[g_players.current_player_turn])
    {
        if (g_players.current_turn_state == AIM_TARGET)
        {
            if (!g_cleared_ball)
            {
                float ball_positions[12] = {0};
                cstrl_renderer_modify_positions(g_ball_render_data, ball_positions, 0, 12);
                g_cleared_ball = true;
            }
            float target_positions[12] = {0};
            if (g_mouse_in_shot_area)
            {
                float target_offset_x = (float)cos(cstrl_platform_get_absolute_time() * 20.0f) * 4.0f;
                float target_offset_y = (float)sin(cstrl_platform_get_absolute_time() * 20.0f) * 4.0f;
                float targetx0 = (float)g_mouse_x - TARGET_SIZE / 2.0f + target_offset_x;
                float targety0 = (float)g_mouse_y - TARGET_SIZE / 2.0f + target_offset_y;
                float targetx1 = (float)g_mouse_x + TARGET_SIZE / 2.0f + target_offset_x;
                float targety1 = (float)g_mouse_y + TARGET_SIZE / 2.0f + target_offset_y;
                g_target_position.x = targetx0 + TARGET_SIZE / 2.0f;
                g_target_position.y = targety0 + TARGET_SIZE / 2.0f;
                g_ball_origin =
                    g_players.current_player_turn == PLAYER1_TURN || g_players.current_player_turn == PLAYER2_TURN
                        ? g_team1_start
                        : g_team2_start;
                target_positions[0] = targetx0;
                target_positions[1] = targety1;
                target_positions[2] = targetx1;
                target_positions[3] = targety0;
                target_positions[4] = targetx0;
                target_positions[5] = targety0;
                target_positions[6] = targetx0;
                target_positions[7] = targety1;
                target_positions[8] = targetx1;
                target_positions[9] = targety0;
                target_positions[10] = targetx1;
                target_positions[11] = targety1;
            }
            cstrl_renderer_modify_positions(g_target_render_data, target_positions, 0, 12);
        }
        else if (g_players.current_turn_state == AIM_METER)
        {
            float meter_positions[12] = {0};
            float meter_bar_positions[12] = {0};
            if (g_mouse_in_shot_area)
            {
                float meterx0 = (float)g_mouse_x - METER_SIZE_X / 2.0f;
                float metery0 = (float)g_mouse_y - METER_SIZE_Y / 2.0f;
                float meterx1 = (float)g_mouse_x + METER_SIZE_X / 2.0f;
                float metery1 = (float)g_mouse_y + METER_SIZE_Y / 2.0f;
                meter_positions[0] = meterx0;
                meter_positions[1] = metery1;
                meter_positions[2] = meterx1;
                meter_positions[3] = metery0;
                meter_positions[4] = meterx0;
                meter_positions[5] = metery0;
                meter_positions[6] = meterx0;
                meter_positions[7] = metery1;
                meter_positions[8] = meterx1;
                meter_positions[9] = metery0;
                meter_positions[10] = meterx1;
                meter_positions[11] = metery1;
                g_target_error = (float)sin(cstrl_platform_get_absolute_time() * 14.0f);
                float meter_bar_offset_y = (g_target_error + 1.0f) / 2.0f * METER_SIZE_Y - METER_SIZE_Y / 2.0f;
                float meter_barx0 = (float)g_mouse_x - METER_BAR_SIZE_X / 2.0f;
                float meter_bary0 = (float)g_mouse_y - METER_BAR_SIZE_Y / 2.0f + meter_bar_offset_y;
                float meter_barx1 = (float)g_mouse_x + METER_BAR_SIZE_X / 2.0f;
                float meter_bary1 = (float)g_mouse_y + METER_BAR_SIZE_Y / 2.0f + meter_bar_offset_y;
                meter_bar_positions[0] = meter_barx0;
                meter_bar_positions[1] = meter_bary1;
                meter_bar_positions[2] = meter_barx1;
                meter_bar_positions[3] = meter_bary0;
                meter_bar_positions[4] = meter_barx0;
                meter_bar_positions[5] = meter_bary0;
                meter_bar_positions[6] = meter_barx0;
                meter_bar_positions[7] = meter_bary1;
                meter_bar_positions[8] = meter_barx1;
                meter_bar_positions[9] = meter_bary0;
                meter_bar_positions[10] = meter_barx1;
                meter_bar_positions[11] = meter_bary1;
            }
            cstrl_renderer_modify_positions(g_meter_render_data, meter_positions, 0, 12);
            cstrl_renderer_modify_positions(g_meter_bar_render_data, meter_bar_positions, 0, 12);
        }
    }
    if (g_players.current_turn_state >= SHOOTING)
    {
        if (!g_cleared_target_and_meter)
        {
            float target_positions[12] = {0};
            cstrl_renderer_modify_positions(g_target_render_data, target_positions, 0, 12);

            float meter_positions[12] = {0};
            cstrl_renderer_modify_positions(g_meter_render_data, meter_positions, 0, 12);

            float meter_bar_positions[12] = {0};
            cstrl_renderer_modify_positions(g_meter_bar_render_data, meter_bar_positions, 0, 12);
            g_cleared_target_and_meter = true;
        }
        for (int i = 0; i < MAX_BALLS; i++)
        {
            if (!g_balls.active[i])
            {
                continue;
            }
            float ball_positions[12];
            float ballx0_origin = -BALL_SIZE / 2.0f;
            float bally0_origin = -BALL_SIZE / 2.0f;
            float ballx1_origin = BALL_SIZE / 2.0f;
            float bally1_origin = -BALL_SIZE / 2.0f;
            float ballx2_origin = -BALL_SIZE / 2.0f;
            float bally2_origin = BALL_SIZE / 2.0f;
            float ballx3_origin = BALL_SIZE / 2.0f;
            float bally3_origin = BALL_SIZE / 2.0f;
            float ballx0 =
                ballx0_origin * cosf(g_balls.angle[i]) - bally0_origin * sinf(g_balls.angle[i]) + g_balls.position[i].x;
            float bally0 =
                bally0_origin * cosf(g_balls.angle[i]) + ballx0_origin * sinf(g_balls.angle[i]) + g_balls.position[i].y;
            float ballx1 =
                ballx1_origin * cosf(g_balls.angle[i]) - bally1_origin * sinf(g_balls.angle[i]) + g_balls.position[i].x;
            float bally1 =
                bally1_origin * cosf(g_balls.angle[i]) + ballx1_origin * sinf(g_balls.angle[i]) + g_balls.position[i].y;
            float ballx2 =
                ballx2_origin * cosf(g_balls.angle[i]) - bally2_origin * sinf(g_balls.angle[i]) + g_balls.position[i].x;
            float bally2 =
                bally2_origin * cosf(g_balls.angle[i]) + ballx2_origin * sinf(g_balls.angle[i]) + g_balls.position[i].y;
            float ballx3 =
                ballx3_origin * cosf(g_balls.angle[i]) - bally3_origin * sinf(g_balls.angle[i]) + g_balls.position[i].x;
            float bally3 =
                bally3_origin * cosf(g_balls.angle[i]) + ballx3_origin * sinf(g_balls.angle[i]) + g_balls.position[i].y;
            ball_positions[0] = ballx2;
            ball_positions[1] = bally2;
            ball_positions[2] = ballx1;
            ball_positions[3] = bally1;
            ball_positions[4] = ballx0;
            ball_positions[5] = bally0;
            ball_positions[6] = ballx2;
            ball_positions[7] = bally2;
            ball_positions[8] = ballx1;
            ball_positions[9] = bally1;
            ball_positions[10] = ballx3;
            ball_positions[11] = bally3;
            cstrl_renderer_modify_positions(g_ball_render_data, ball_positions, i * 12, 12);
        }
    }

    cstrl_use_shader(g_default_shader);
    cstrl_texture_bind(g_background_texture);
    cstrl_renderer_draw(g_background_render_data);
    cstrl_texture_bind(g_cup_texture);
    cstrl_renderer_draw(g_cup_render_data);
    cstrl_texture_bind(g_ball_texture);
    cstrl_renderer_draw(g_ball_render_data);
    // cstrl_texture_bind(g_player_texture);
    // cstrl_renderer_draw(g_player_render_data);
    cstrl_texture_bind(g_target_texture);
    cstrl_renderer_draw(g_target_render_data);
    cstrl_texture_bind(g_meter_texture);
    cstrl_renderer_draw(g_meter_render_data);
    cstrl_texture_bind(g_meter_bar_texture);
    cstrl_renderer_draw(g_meter_bar_render_data);

    cstrl_ui_begin(&g_ui_context);
    int window_width, window_height;
    cstrl_platform_get_window_size(g_platform_state, &window_width, &window_height);
    if (cstrl_ui_container_begin(&g_ui_context, "Options", 7, 0, window_height - 200, window_width, 200, GEN_ID(0),
                                 true, false, 2, &g_base_layout))
    {
        if (cstrl_ui_button(&g_ui_context, "Reset", 5, 10, 50, 150, 50, GEN_ID(0), &g_sub_layout))
        {
            g_reset_game_update = true;
            g_reset_game_render = true;
        }
        if (cstrl_ui_button(&g_ui_context, "Pause", 5, 10, 110, 150, 50, GEN_ID(0), &g_sub_layout))
        {
            g_paused = !g_paused;
        }
        cstrl_ui_container_end(&g_ui_context);
    }
    if (g_paused)
    {

        if (cstrl_ui_container_begin(&g_ui_context, "Paused", 6, window_width / 2 - 25, window_height / 2 - 50, 100, 50,
                                     GEN_ID(0), true, false, 2, &g_base_layout))
        {
        }
    }
    cstrl_ui_end(&g_ui_context);

    cstrl_use_shader(g_default_shader);
    cstrl_texture_bind(g_cursor_texture);
    cstrl_renderer_draw(g_cursor_render_data);
}

void main_game_scene_render()
{
    main_game_scene_main_game_stage_render();
    if (g_overtime_init_render)
    {
        float cup_positions[240] = {0};
        for (int i = 4; i < 20; i++)
        {
            if (i > 9 && i < 14)
            {
                continue;
            }
            float x0 = g_cups.position[i].x - CUP_SIZE / 2.0f;
            float y0 = g_cups.position[i].y - CUP_SIZE / 2.0f;
            float x1 = g_cups.position[i].x + CUP_SIZE / 2.0f;
            float y1 = g_cups.position[i].y + CUP_SIZE / 2.0f;
            cup_positions[i * 12] = x0;
            cup_positions[i * 12 + 1] = y1;
            cup_positions[i * 12 + 2] = x1;
            cup_positions[i * 12 + 3] = y0;
            cup_positions[i * 12 + 4] = x0;
            cup_positions[i * 12 + 5] = y0;
            cup_positions[i * 12 + 6] = x0;
            cup_positions[i * 12 + 7] = y1;
            cup_positions[i * 12 + 8] = x1;
            cup_positions[i * 12 + 9] = y0;
            cup_positions[i * 12 + 10] = x1;
            cup_positions[i * 12 + 11] = y1;
        }
        cstrl_renderer_modify_positions(g_cup_render_data, cup_positions, 0, 240);

        float target_positions[12] = {0};
        cstrl_renderer_modify_positions(g_target_render_data, target_positions, 0, 12);

        float meter_positions[12] = {0};
        cstrl_renderer_modify_positions(g_meter_render_data, meter_positions, 0, 12);

        float meter_bar_positions[12] = {0};
        cstrl_renderer_modify_positions(g_meter_bar_render_data, meter_bar_positions, 0, 12);
        g_cleared_target_and_meter = true;

        g_overtime_init_render = false;
    }
    if (g_reset_game_render)
    {
        printf("Resetting Render...\n");
        float cup_positions[240];
        for (int i = 0; i < 20; i++)
        {
            float x0 = g_cups.position[i].x - CUP_SIZE / 2.0f;
            float y0 = g_cups.position[i].y - CUP_SIZE / 2.0f;
            float x1 = g_cups.position[i].x + CUP_SIZE / 2.0f;
            float y1 = g_cups.position[i].y + CUP_SIZE / 2.0f;
            cup_positions[i * 12] = x0;
            cup_positions[i * 12 + 1] = y1;
            cup_positions[i * 12 + 2] = x1;
            cup_positions[i * 12 + 3] = y0;
            cup_positions[i * 12 + 4] = x0;
            cup_positions[i * 12 + 5] = y0;
            cup_positions[i * 12 + 6] = x0;
            cup_positions[i * 12 + 7] = y1;
            cup_positions[i * 12 + 8] = x1;
            cup_positions[i * 12 + 9] = y0;
            cup_positions[i * 12 + 10] = x1;
            cup_positions[i * 12 + 11] = y1;
        }
        cstrl_renderer_modify_positions(g_cup_render_data, cup_positions, 0, 240);

        float target_positions[12] = {0};
        cstrl_renderer_modify_positions(g_target_render_data, target_positions, 0, 12);

        float meter_positions[12] = {0};
        cstrl_renderer_modify_positions(g_meter_render_data, meter_positions, 0, 12);

        float meter_bar_positions[12] = {0};
        cstrl_renderer_modify_positions(g_meter_bar_render_data, meter_bar_positions, 0, 12);
        g_cleared_target_and_meter = true;

        g_reset_game_render = false;
    }
}

void main_game_scene_shutdown()
{
    cstrl_renderer_free_render_data(g_background_render_data);
    cstrl_renderer_free_render_data(g_cup_render_data);
    cstrl_renderer_free_render_data(g_ball_render_data);
    cstrl_renderer_free_render_data(g_target_render_data);
    cstrl_renderer_free_render_data(g_meter_render_data);
    cstrl_renderer_free_render_data(g_meter_bar_render_data);
}

void scenes_clean()
{
    cstrl_camera_free(g_main_camera);
    cstrl_renderer_free_render_data(g_cursor_render_data);
}
