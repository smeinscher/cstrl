#include "game.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"
#include "cstrl/cstrl_util.h"
#include "objects/cup.h"
#include <stdio.h>
#include <stdlib.h>

#define PLAYER_SHOT_BOUND_MIN_X 258
#define PLAYER_SHOT_BOUND_MAX_X 420
#define PLAYER_SHOT_BOUND_MIN_Y 265
#define PLAYER_SHOT_BOUND_MAX_Y 455

static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;
static const char *WINDOW_TITLE = "Beer Pong Champion";

static int g_mouse_x = 0;
static int g_mouse_y = 0;

static CSTRL_PACKED_ENUM{PLAYER1_AIM_CUP,   PLAYER1_AIM_METER, PLAYER1_SHOOTING,      PLAYER2_AIM_CUP,
                         PLAYER2_AIM_METER, PLAYER2_SHOOTING,  PLAYER_TURNS_PROCESSED} g_player_state;

static bool g_mouse_in_shot_area = false;

static void key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    if (key == CSTRL_KEY_ESCAPE)
    {
        cstrl_platform_set_should_exit(true);
    }
}

static void mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos)
{
    g_mouse_x = xpos;
    g_mouse_y = ypos;
}

static void mouse_button_callback(cstrl_platform_state *state, int button, int action, int mods)
{
    if (button == CSTRL_MOUSE_BUTTON_LEFT)
    {
        if (action == CSTRL_ACTION_PRESS && g_mouse_in_shot_area)
        {
            if (g_player_state == PLAYER1_AIM_CUP)
            {
                g_player_state = PLAYER1_AIM_METER;
            }
            else if (g_player_state == PLAYER1_AIM_METER)
            {
                g_player_state = PLAYER1_SHOOTING;
            }
        }
    }
}

int bpc_game_run()
{
    cstrl_platform_state platform_state;
    if (!cstrl_platform_init(&platform_state, WINDOW_TITLE, (1920 - WINDOW_WIDTH) / 2, (1080 - WINDOW_HEIGHT) / 2,
                             WINDOW_WIDTH, WINDOW_HEIGHT))
    {
        cstrl_platform_shutdown(&platform_state);
        return 1;
    }

    cstrl_platform_set_key_callback(&platform_state, key_callback);
    cstrl_platform_set_mouse_position_callback(&platform_state, mouse_position_callback);
    cstrl_platform_set_mouse_button_callback(&platform_state, mouse_button_callback);

    cstrl_platform_set_show_cursor(&platform_state, false);

    srand(cstrl_platform_get_absolute_time());

    cstrl_renderer_init(&platform_state);

    cstrl_shader default_shader =
        cstrl_load_shaders_from_files("resources/shaders/default.vert", "resources/shaders/default.frag");

    cstrl_render_data *floor_render_data = cstrl_renderer_create_render_data();
    float x0 = 0.0f;
    float y0 = 0.0f;
    float x1 = WINDOW_WIDTH;
    float y1 = WINDOW_HEIGHT;
    float floor_positions[] = {x0, y1, x1, y0, x0, y0, x0, y1, x1, y0, x1, y1};
    float floor_uvs[] = {
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f,
    };
    float floor_colors[24];
    for (int i = 0; i < 6; i++)
    {
        floor_colors[i * 4] = 1.0f;
        floor_colors[i * 4 + 1] = 1.0f;
        floor_colors[i * 4 + 2] = 1.0f;
        floor_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(floor_render_data, floor_positions, 2, 6);
    cstrl_renderer_add_uvs(floor_render_data, floor_uvs);
    cstrl_renderer_add_colors(floor_render_data, floor_colors);

    cstrl_texture floor_texture = cstrl_texture_generate_from_path("resources/textures/beer_pong/bar_floor3.png");

    cstrl_render_data *table_render_data = cstrl_renderer_create_render_data();
    x0 = 256.0f;
    y0 = 264.0f;
    x1 = 1024.0f;
    y1 = 456.0f;
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
    cstrl_renderer_add_positions(table_render_data, table_positions, 2, 6);
    cstrl_renderer_add_uvs(table_render_data, table_uvs);
    cstrl_renderer_add_colors(table_render_data, table_colors);

    cstrl_texture table_texture = cstrl_texture_generate_from_path("resources/textures/beer_pong/table_pixelated.png");

    cstrl_render_data *cup_render_data = cstrl_renderer_create_render_data();
    cups_t cups;
    cups_init(&cups);
    da_float cup_positions;
    cstrl_da_float_init(&cup_positions, 240);
    for (int i = 0; i < 20; i++)
    {
        float x0 = cups.position[i].x - CUP_SIZE / 2.0f;
        float y0 = cups.position[i].y - CUP_SIZE / 2.0f;
        float x1 = cups.position[i].x + CUP_SIZE / 2.0f;
        float y1 = cups.position[i].y + CUP_SIZE / 2.0f;
        cstrl_da_float_push_back(&cup_positions, x0);
        cstrl_da_float_push_back(&cup_positions, y1);
        cstrl_da_float_push_back(&cup_positions, x1);
        cstrl_da_float_push_back(&cup_positions, y0);
        cstrl_da_float_push_back(&cup_positions, x0);
        cstrl_da_float_push_back(&cup_positions, y0);
        cstrl_da_float_push_back(&cup_positions, x0);
        cstrl_da_float_push_back(&cup_positions, y1);
        cstrl_da_float_push_back(&cup_positions, x1);
        cstrl_da_float_push_back(&cup_positions, y0);
        cstrl_da_float_push_back(&cup_positions, x1);
        cstrl_da_float_push_back(&cup_positions, y1);
    }
    da_float cup_uvs;
    cstrl_da_float_init(&cup_uvs, 240);
    for (int i = 0; i < 20; i++)
    {
        cstrl_da_float_push_back(&cup_uvs, 0.0f);
        cstrl_da_float_push_back(&cup_uvs, 1.0f);
        cstrl_da_float_push_back(&cup_uvs, 1.0f);
        cstrl_da_float_push_back(&cup_uvs, 0.0f);
        cstrl_da_float_push_back(&cup_uvs, 0.0f);
        cstrl_da_float_push_back(&cup_uvs, 0.0f);
        cstrl_da_float_push_back(&cup_uvs, 0.0f);
        cstrl_da_float_push_back(&cup_uvs, 1.0f);
        cstrl_da_float_push_back(&cup_uvs, 1.0f);
        cstrl_da_float_push_back(&cup_uvs, 0.0f);
        cstrl_da_float_push_back(&cup_uvs, 1.0f);
        cstrl_da_float_push_back(&cup_uvs, 1.0f);
    }
    float cup_colors[24 * 20];
    for (int i = 0; i < 6 * 20; i++)
    {
        cup_colors[i * 4] = 1.0f;
        cup_colors[i * 4 + 1] = 1.0f;
        cup_colors[i * 4 + 2] = 1.0f;
        cup_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(cup_render_data, cup_positions.array, 2, cup_positions.size / 2);
    cstrl_renderer_add_uvs(cup_render_data, cup_uvs.array);
    cstrl_renderer_add_colors(cup_render_data, cup_colors);

    cstrl_texture cup_texture = cstrl_texture_generate_from_path("resources/textures/beer_pong/cup.png");

    cstrl_render_data *ball_render_data = cstrl_renderer_create_render_data();
    da_float ball_positions;
    cstrl_da_float_init(&ball_positions, 12);
    float ball_size = 12.5f;
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    cstrl_da_float_push_back(&ball_positions, 0.0f);
    da_float ball_uvs;
    cstrl_da_float_init(&ball_uvs, 12);
    cstrl_da_float_push_back(&ball_uvs, 0.0f);
    cstrl_da_float_push_back(&ball_uvs, 1.0f);
    cstrl_da_float_push_back(&ball_uvs, 1.0f);
    cstrl_da_float_push_back(&ball_uvs, 0.0f);
    cstrl_da_float_push_back(&ball_uvs, 0.0f);
    cstrl_da_float_push_back(&ball_uvs, 0.0f);
    cstrl_da_float_push_back(&ball_uvs, 0.0f);
    cstrl_da_float_push_back(&ball_uvs, 1.0f);
    cstrl_da_float_push_back(&ball_uvs, 1.0f);
    cstrl_da_float_push_back(&ball_uvs, 0.0f);
    cstrl_da_float_push_back(&ball_uvs, 1.0f);
    cstrl_da_float_push_back(&ball_uvs, 1.0f);
    float ball_colors[24];
    for (int i = 0; i < 6; i++)
    {
        ball_colors[i * 4] = 1.0f;
        ball_colors[i * 4 + 1] = 1.0f;
        ball_colors[i * 4 + 2] = 1.0f;
        ball_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(ball_render_data, ball_positions.array, 2, ball_positions.size / 2);
    cstrl_renderer_add_uvs(ball_render_data, ball_uvs.array);
    cstrl_renderer_add_colors(ball_render_data, ball_colors);

    cstrl_texture ball_texture = cstrl_texture_generate_from_path("resources/textures/beer_pong/ball.png");

    cstrl_render_data *player_render_data = cstrl_renderer_create_render_data();
    da_float player_positions;
    cstrl_da_float_init(&player_positions, 24);
    float player_size_x = 68.0f;
    float player_size_y = 126.0f;
    float playerx0 = 165.0f;
    float playery0 = WINDOW_HEIGHT / 2.0f - player_size_y / 2.0f;
    float playerx1 = playerx0 + player_size_x;
    float playery1 = WINDOW_HEIGHT / 2.0f + player_size_y / 2.0f;
    cstrl_da_float_push_back(&player_positions, playerx0);
    cstrl_da_float_push_back(&player_positions, playery1);
    cstrl_da_float_push_back(&player_positions, playerx1);
    cstrl_da_float_push_back(&player_positions, playery0);
    cstrl_da_float_push_back(&player_positions, playerx0);
    cstrl_da_float_push_back(&player_positions, playery0);
    cstrl_da_float_push_back(&player_positions, playerx0);
    cstrl_da_float_push_back(&player_positions, playery1);
    cstrl_da_float_push_back(&player_positions, playerx1);
    cstrl_da_float_push_back(&player_positions, playery0);
    cstrl_da_float_push_back(&player_positions, playerx1);
    cstrl_da_float_push_back(&player_positions, playery1);

    playerx0 = 1115.0f;
    playerx1 = playerx0 - player_size_x;
    cstrl_da_float_push_back(&player_positions, playerx0);
    cstrl_da_float_push_back(&player_positions, playery1);
    cstrl_da_float_push_back(&player_positions, playerx1);
    cstrl_da_float_push_back(&player_positions, playery0);
    cstrl_da_float_push_back(&player_positions, playerx0);
    cstrl_da_float_push_back(&player_positions, playery0);
    cstrl_da_float_push_back(&player_positions, playerx0);
    cstrl_da_float_push_back(&player_positions, playery1);
    cstrl_da_float_push_back(&player_positions, playerx1);
    cstrl_da_float_push_back(&player_positions, playery0);
    cstrl_da_float_push_back(&player_positions, playerx1);
    cstrl_da_float_push_back(&player_positions, playery1);
    da_float player_uvs;
    cstrl_da_float_init(&player_uvs, 24);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 0.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    cstrl_da_float_push_back(&player_uvs, 1.0f);
    float player_colors[48];
    for (int i = 0; i < 12; i++)
    {
        player_colors[i * 4] = 1.0f;
        player_colors[i * 4 + 1] = 1.0f;
        player_colors[i * 4 + 2] = 1.0f;
        player_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(player_render_data, player_positions.array, 2, player_positions.size / 2);
    cstrl_renderer_add_uvs(player_render_data, player_uvs.array);
    cstrl_renderer_add_colors(player_render_data, player_colors);

    cstrl_texture player_texture = cstrl_texture_generate_from_path("resources/textures/beer_pong/gta_player.png");

    cstrl_render_data *cursor_render_data = cstrl_renderer_create_render_data();
    da_float cursor_positions;
    cstrl_da_float_init(&cursor_positions, 12);
    float cursor_size = 25.0f;
    float cursorx0 = WINDOW_WIDTH / 2.0f - cursor_size / 2.0f;
    float cursory0 = WINDOW_HEIGHT / 2.0f - cursor_size / 2.0f;
    float cursorx1 = WINDOW_WIDTH / 2.0f + cursor_size / 2.0f;
    float cursory1 = WINDOW_HEIGHT / 2.0f + cursor_size / 2.0f;
    cstrl_da_float_push_back(&cursor_positions, cursorx0);
    cstrl_da_float_push_back(&cursor_positions, cursory1);
    cstrl_da_float_push_back(&cursor_positions, cursorx1);
    cstrl_da_float_push_back(&cursor_positions, cursory0);
    cstrl_da_float_push_back(&cursor_positions, cursorx0);
    cstrl_da_float_push_back(&cursor_positions, cursory0);
    cstrl_da_float_push_back(&cursor_positions, cursorx0);
    cstrl_da_float_push_back(&cursor_positions, cursory1);
    cstrl_da_float_push_back(&cursor_positions, cursorx1);
    cstrl_da_float_push_back(&cursor_positions, cursory0);
    cstrl_da_float_push_back(&cursor_positions, cursorx1);
    cstrl_da_float_push_back(&cursor_positions, cursory1);
    da_float cursor_uvs;
    cstrl_da_float_init(&cursor_uvs, 12);
    cstrl_da_float_push_back(&cursor_uvs, 0.0f);
    cstrl_da_float_push_back(&cursor_uvs, 1.0f);
    cstrl_da_float_push_back(&cursor_uvs, 1.0f);
    cstrl_da_float_push_back(&cursor_uvs, 0.0f);
    cstrl_da_float_push_back(&cursor_uvs, 0.0f);
    cstrl_da_float_push_back(&cursor_uvs, 0.0f);
    cstrl_da_float_push_back(&cursor_uvs, 0.0f);
    cstrl_da_float_push_back(&cursor_uvs, 1.0f);
    cstrl_da_float_push_back(&cursor_uvs, 1.0f);
    cstrl_da_float_push_back(&cursor_uvs, 0.0f);
    cstrl_da_float_push_back(&cursor_uvs, 1.0f);
    cstrl_da_float_push_back(&cursor_uvs, 1.0f);
    float cursor_colors[24];
    for (int i = 0; i < 6; i++)
    {
        cursor_colors[i * 4] = 1.0f;
        cursor_colors[i * 4 + 1] = 1.0f;
        cursor_colors[i * 4 + 2] = 1.0f;
        cursor_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(cursor_render_data, cursor_positions.array, 2, cursor_positions.size / 2);
    cstrl_renderer_add_uvs(cursor_render_data, cursor_uvs.array);
    cstrl_renderer_add_colors(cursor_render_data, cursor_colors);

    cstrl_texture cursor_texture = cstrl_texture_generate_from_path("resources/textures/beer_pong/cursor.png");

    cstrl_render_data *target_render_data = cstrl_renderer_create_render_data();
    da_float target_positions;
    cstrl_da_float_init(&target_positions, 12);
    float target_size = 12.5f;
    float targetx0 = WINDOW_WIDTH / 2.0f - target_size / 2.0f;
    float targety0 = WINDOW_HEIGHT / 2.0f - target_size / 2.0f;
    float targetx1 = WINDOW_WIDTH / 2.0f + target_size / 2.0f;
    float targety1 = WINDOW_HEIGHT / 2.0f + target_size / 2.0f;
    cstrl_da_float_push_back(&target_positions, targetx0);
    cstrl_da_float_push_back(&target_positions, targety1);
    cstrl_da_float_push_back(&target_positions, targetx1);
    cstrl_da_float_push_back(&target_positions, targety0);
    cstrl_da_float_push_back(&target_positions, targetx0);
    cstrl_da_float_push_back(&target_positions, targety0);
    cstrl_da_float_push_back(&target_positions, targetx0);
    cstrl_da_float_push_back(&target_positions, targety1);
    cstrl_da_float_push_back(&target_positions, targetx1);
    cstrl_da_float_push_back(&target_positions, targety0);
    cstrl_da_float_push_back(&target_positions, targetx1);
    cstrl_da_float_push_back(&target_positions, targety1);
    da_float target_uvs;
    cstrl_da_float_init(&target_uvs, 12);
    cstrl_da_float_push_back(&target_uvs, 0.0f);
    cstrl_da_float_push_back(&target_uvs, 1.0f);
    cstrl_da_float_push_back(&target_uvs, 1.0f);
    cstrl_da_float_push_back(&target_uvs, 0.0f);
    cstrl_da_float_push_back(&target_uvs, 0.0f);
    cstrl_da_float_push_back(&target_uvs, 0.0f);
    cstrl_da_float_push_back(&target_uvs, 0.0f);
    cstrl_da_float_push_back(&target_uvs, 1.0f);
    cstrl_da_float_push_back(&target_uvs, 1.0f);
    cstrl_da_float_push_back(&target_uvs, 0.0f);
    cstrl_da_float_push_back(&target_uvs, 1.0f);
    cstrl_da_float_push_back(&target_uvs, 1.0f);
    float target_colors[24];
    for (int i = 0; i < 6; i++)
    {
        target_colors[i * 4] = 1.0f;
        target_colors[i * 4 + 1] = 1.0f;
        target_colors[i * 4 + 2] = 1.0f;
        target_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(target_render_data, target_positions.array, 2, target_positions.size / 2);
    cstrl_renderer_add_uvs(target_render_data, target_uvs.array);
    cstrl_renderer_add_colors(target_render_data, target_colors);

    cstrl_texture target_texture = cstrl_texture_generate_from_path("resources/textures/beer_pong/target.png");

    cstrl_render_data *meter_render_data = cstrl_renderer_create_render_data();
    da_float meter_positions;
    cstrl_da_float_init(&meter_positions, 12);
    float meter_size_x = 6.75f;
    float meter_size_y = 75.0f;
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    cstrl_da_float_push_back(&meter_positions, 0.0f);
    da_float meter_uvs;
    cstrl_da_float_init(&meter_uvs, 12);
    cstrl_da_float_push_back(&meter_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_uvs, 1.0f);
    cstrl_da_float_push_back(&meter_uvs, 1.0f);
    cstrl_da_float_push_back(&meter_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_uvs, 1.0f);
    cstrl_da_float_push_back(&meter_uvs, 1.0f);
    cstrl_da_float_push_back(&meter_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_uvs, 1.0f);
    cstrl_da_float_push_back(&meter_uvs, 1.0f);
    float meter_colors[24];
    for (int i = 0; i < 6; i++)
    {
        meter_colors[i * 4] = 1.0f;
        meter_colors[i * 4 + 1] = 1.0f;
        meter_colors[i * 4 + 2] = 1.0f;
        meter_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(meter_render_data, meter_positions.array, 2, meter_positions.size / 2);
    cstrl_renderer_add_uvs(meter_render_data, meter_uvs.array);
    cstrl_renderer_add_colors(meter_render_data, meter_colors);

    cstrl_texture meter_texture = cstrl_texture_generate_from_path("resources/textures/beer_pong/meter.png");

    cstrl_render_data *meter_bar_render_data = cstrl_renderer_create_render_data();
    da_float meter_bar_positions;
    cstrl_da_float_init(&meter_bar_positions, 12);
    float meter_bar_size_x = 12.5f;
    float meter_bar_size_y = 4.0f;
    float meter_barx0 = WINDOW_WIDTH / 2.0f - meter_bar_size_x / 2.0f;
    float meter_bary0 = WINDOW_HEIGHT / 2.0f - meter_bar_size_y / 2.0f;
    float meter_barx1 = WINDOW_WIDTH / 2.0f + meter_bar_size_x / 2.0f;
    float meter_bary1 = WINDOW_HEIGHT / 2.0f + meter_bar_size_y / 2.0f;
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
    da_float meter_bar_uvs;
    cstrl_da_float_init(&meter_bar_uvs, 12);
    cstrl_da_float_push_back(&meter_bar_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 1.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 1.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 1.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 1.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 0.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 1.0f);
    cstrl_da_float_push_back(&meter_bar_uvs, 1.0f);
    float meter_bar_colors[24];
    for (int i = 0; i < 6; i++)
    {
        meter_bar_colors[i * 4] = 1.0f;
        meter_bar_colors[i * 4 + 1] = 1.0f;
        meter_bar_colors[i * 4 + 2] = 1.0f;
        meter_bar_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(meter_bar_render_data, meter_bar_positions.array, 2, meter_bar_positions.size / 2);
    cstrl_renderer_add_uvs(meter_bar_render_data, meter_bar_uvs.array);
    cstrl_renderer_add_colors(meter_bar_render_data, meter_bar_colors);

    cstrl_texture meter_bar_texture = cstrl_texture_generate_from_path("resources/textures/beer_pong/meter_bar.png");

    cstrl_camera *camera = cstrl_camera_create(1280, 720, true);
    cstrl_camera_update(camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    cstrl_set_uniform_mat4(default_shader.program, "view", camera->view);
    cstrl_set_uniform_mat4(default_shader.program, "projection", camera->projection);

    vec2 player1_start = (vec2){playerx0 + player_size_x / 2.0f, playery0 + player_size_y / 2.0f};
    vec2 player2_start = (vec2){165.0f + player_size_x / 2.0f, playery0 + player_size_y / 2.0f};
    vec2 ball_position = player1_start;
    vec2 target_position;
    float target_error;
    float ball_progression = 0.0f;
    bool cleared_target_and_meter = false;
    bool cleared_ball = false;
    double turn_end_time = 0.0f;
    bool next_turn = false;

    cstrl_ui_context context;
    cstrl_ui_init(&context, &platform_state);

    cstrl_ui_layout base_layout = {0};
    base_layout.border.size = (cstrl_ui_border_size){4, 4, 4, 4, 0};
    base_layout.border.color = (cstrl_ui_color){0.96f, 0.98f, 0.85f, 1.0f};
    base_layout.border.gap_color = (cstrl_ui_color){0.29f, 0.89f, 0.75f, 1.0f};
    base_layout.border.gap_size = 1;
    base_layout.color = (cstrl_ui_color){0.36f, 0.32f, 0.84f, 1.0f};
    base_layout.font_color = (cstrl_ui_color){0.12f, 0.1f, 0.13f, 1.0f};
    cstrl_ui_layout sub_layout = {0};
    sub_layout.border.size = (cstrl_ui_border_size){1, 1, 1, 1, 0};
    sub_layout.border.color = (cstrl_ui_color){0.12f, 0.1f, 0.13f, 1.0f};
    sub_layout.color = (cstrl_ui_color){0.29f, 0.89f, 0.75f, 1.0f};
    sub_layout.font_color = (cstrl_ui_color){0.12f, 0.1f, 0.13f, 1.0f};

    double previous_frame_time = 0.0;
    double frame_lag = 0.0;
    while (!cstrl_platform_should_exit())
    {
        cstrl_renderer_clear(0.1f, 0.1f, 0.1f, 1.0f);
        cstrl_platform_pump_messages(&platform_state);
        double current_frame_time = cstrl_platform_get_absolute_time();
        double elapsed_frame_time = current_frame_time - previous_frame_time;
        previous_frame_time = current_frame_time;
        frame_lag += elapsed_frame_time;
        while (frame_lag >= 1.0 / 60.0)
        {
            if (g_player_state == PLAYER1_SHOOTING || g_player_state == PLAYER2_SHOOTING)
            {
                vec2 origin = g_player_state == PLAYER1_SHOOTING ? player1_start : player2_start;
                vec2 target_position_mod = target_position;
                target_position_mod.y += target_error * 50.0f;
                ball_progression += 10.0f / cstrl_vec2_length(cstrl_vec2_sub(target_position_mod, origin));
                ball_position = cstrl_vec2_add(cstrl_vec2_mult_scalar(origin, (1.0 - ball_progression)),
                                               cstrl_vec2_mult_scalar(target_position_mod, ball_progression));
                if (ball_progression >= 1.0f)
                {
                    g_player_state = g_player_state == PLAYER1_SHOOTING ? PLAYER2_AIM_CUP : PLAYER_TURNS_PROCESSED;
                    ball_progression = 0.0f;
                    ball_position = PLAYER1_SHOOTING ? player2_start : player1_start;
                    cleared_target_and_meter = false;
                    cleared_ball = false;
                    turn_end_time = cstrl_platform_get_absolute_time();
                    next_turn = false;
                    int made_cup = cups_shot_test(&cups, target_position_mod);
                    if (made_cup >= 0)
                    {
                        cup_positions.array[made_cup * 12] = 0.0f;
                        cup_positions.array[made_cup * 12 + 1] = 0.0f;
                        cup_positions.array[made_cup * 12 + 2] = 0.0f;
                        cup_positions.array[made_cup * 12 + 3] = 0.0f;
                        cup_positions.array[made_cup * 12 + 4] = 0.0f;
                        cup_positions.array[made_cup * 12 + 5] = 0.0f;
                        cup_positions.array[made_cup * 12 + 6] = 0.0f;
                        cup_positions.array[made_cup * 12 + 7] = 0.0f;
                        cup_positions.array[made_cup * 12 + 8] = 0.0f;
                        cup_positions.array[made_cup * 12 + 9] = 0.0f;
                        cup_positions.array[made_cup * 12 + 10] = 0.0f;
                        cup_positions.array[made_cup * 12 + 11] = 0.0f;
                        cstrl_renderer_modify_positions(cup_render_data, cup_positions.array, made_cup * 12, 12);
                    }
                }
            }
            else if (!next_turn && cstrl_platform_get_absolute_time() - turn_end_time > 2.0)
            {
                if (g_player_state == PLAYER2_AIM_CUP)
                {
                    g_player_state = PLAYER2_SHOOTING;
                    int selected_cup = rand() % 10 + 10;
                    target_position = cups.position[selected_cup];
                    target_error = (float)(rand() % 1000 - 500) / 1000.0f;
                }
                else
                {
                    g_player_state = PLAYER1_AIM_CUP;
                }
                next_turn = true;
            }
            frame_lag -= 1.0 / 60.0;
        } // end update loop

        if (g_mouse_x < PLAYER_SHOT_BOUND_MAX_X && g_mouse_x > PLAYER_SHOT_BOUND_MIN_X &&
            g_mouse_y < PLAYER_SHOT_BOUND_MAX_Y && g_mouse_y > PLAYER_SHOT_BOUND_MIN_Y)
        {
            g_mouse_in_shot_area = true;
        }
        else
        {
            g_mouse_in_shot_area = false;
        }

        if (g_mouse_in_shot_area && (g_player_state == PLAYER1_AIM_CUP || g_player_state == PLAYER1_AIM_METER))
        {
            cstrl_da_float_clear(&cursor_positions);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
            cstrl_da_float_push_back(&cursor_positions, 0.0f);
        }
        else
        {
            cstrl_da_float_clear(&cursor_positions);
            cursorx0 = g_mouse_x - cursor_size / 2.0f;
            cursory0 = g_mouse_y - cursor_size / 2.0f;
            cursorx1 = g_mouse_x + cursor_size / 2.0f;
            cursory1 = g_mouse_y + cursor_size / 2.0f;
            cstrl_da_float_push_back(&cursor_positions, cursorx0);
            cstrl_da_float_push_back(&cursor_positions, cursory1);
            cstrl_da_float_push_back(&cursor_positions, cursorx1);
            cstrl_da_float_push_back(&cursor_positions, cursory0);
            cstrl_da_float_push_back(&cursor_positions, cursorx0);
            cstrl_da_float_push_back(&cursor_positions, cursory0);
            cstrl_da_float_push_back(&cursor_positions, cursorx0);
            cstrl_da_float_push_back(&cursor_positions, cursory1);
            cstrl_da_float_push_back(&cursor_positions, cursorx1);
            cstrl_da_float_push_back(&cursor_positions, cursory0);
            cstrl_da_float_push_back(&cursor_positions, cursorx1);
            cstrl_da_float_push_back(&cursor_positions, cursory1);
        }
        cstrl_renderer_modify_positions(cursor_render_data, cursor_positions.array, 0, 12);
        if (g_player_state == PLAYER1_AIM_CUP)
        {
            if (!cleared_ball)
            {
                cstrl_da_float_clear(&ball_positions);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_da_float_push_back(&ball_positions, 0.0f);
                cstrl_renderer_modify_positions(ball_render_data, ball_positions.array, 0, 12);
                cleared_ball = true;
            }
            if (g_mouse_in_shot_area)
            {
                cstrl_da_float_clear(&target_positions);
                float target_offset_x = cos(cstrl_platform_get_absolute_time() * 10.0f) * 10.0f;
                float target_offset_y = sin(cstrl_platform_get_absolute_time() * 10.0f) * 10.0f;
                float targetx0 = g_mouse_x - target_size / 2.0f + target_offset_x;
                float targety0 = g_mouse_y - target_size / 2.0f + target_offset_y;
                float targetx1 = g_mouse_x + target_size / 2.0f + target_offset_x;
                float targety1 = g_mouse_y + target_size / 2.0f + target_offset_y;
                target_position.x = targetx0 + target_size / 2.0f;
                target_position.y = targety0 + target_size / 2.0f;
                cstrl_da_float_push_back(&target_positions, targetx0);
                cstrl_da_float_push_back(&target_positions, targety1);
                cstrl_da_float_push_back(&target_positions, targetx1);
                cstrl_da_float_push_back(&target_positions, targety0);
                cstrl_da_float_push_back(&target_positions, targetx0);
                cstrl_da_float_push_back(&target_positions, targety0);
                cstrl_da_float_push_back(&target_positions, targetx0);
                cstrl_da_float_push_back(&target_positions, targety1);
                cstrl_da_float_push_back(&target_positions, targetx1);
                cstrl_da_float_push_back(&target_positions, targety0);
                cstrl_da_float_push_back(&target_positions, targetx1);
                cstrl_da_float_push_back(&target_positions, targety1);
            }
            else
            {
                cstrl_da_float_clear(&target_positions);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
            }
            cstrl_renderer_modify_positions(target_render_data, target_positions.array, 0, 12);
        }
        else if (g_player_state == PLAYER1_AIM_METER)
        {
            if (g_mouse_in_shot_area)
            {
                cstrl_da_float_clear(&meter_positions);
                float meterx0 = g_mouse_x - meter_size_x / 2.0f;
                float metery0 = g_mouse_y - meter_size_y / 2.0f;
                float meterx1 = g_mouse_x + meter_size_x / 2.0f;
                float metery1 = g_mouse_y + meter_size_y / 2.0f;
                cstrl_da_float_push_back(&meter_positions, meterx0);
                cstrl_da_float_push_back(&meter_positions, metery1);
                cstrl_da_float_push_back(&meter_positions, meterx1);
                cstrl_da_float_push_back(&meter_positions, metery0);
                cstrl_da_float_push_back(&meter_positions, meterx0);
                cstrl_da_float_push_back(&meter_positions, metery0);
                cstrl_da_float_push_back(&meter_positions, meterx0);
                cstrl_da_float_push_back(&meter_positions, metery1);
                cstrl_da_float_push_back(&meter_positions, meterx1);
                cstrl_da_float_push_back(&meter_positions, metery0);
                cstrl_da_float_push_back(&meter_positions, meterx1);
                cstrl_da_float_push_back(&meter_positions, metery1);
                cstrl_da_float_clear(&meter_bar_positions);
                target_error = sin(cstrl_platform_get_absolute_time() * 4.0f);
                float meter_bar_offset_y = (target_error + 1.0f) / 2.0f * meter_size_y - meter_size_y / 2.0f;
                float meter_barx0 = g_mouse_x - meter_bar_size_x / 2.0f;
                float meter_bary0 = g_mouse_y - meter_bar_size_y / 2.0f + meter_bar_offset_y;
                float meter_barx1 = g_mouse_x + meter_bar_size_x / 2.0f;
                float meter_bary1 = g_mouse_y + meter_bar_size_y / 2.0f + meter_bar_offset_y;
                cstrl_da_float_push_back(&meter_bar_positions, meter_barx0);
                cstrl_da_float_push_back(&meter_bar_positions, meter_bary1);
                cstrl_da_float_push_back(&meter_bar_positions, meter_barx1);
                cstrl_da_float_push_back(&meter_bar_positions, meter_bary0);
                cstrl_da_float_push_back(&meter_bar_positions, meter_barx0);
                cstrl_da_float_push_back(&meter_bar_positions, meter_bary0);
                cstrl_da_float_push_back(&meter_bar_positions, meter_barx0);
                cstrl_da_float_push_back(&meter_bar_positions, meter_bary1);
                cstrl_da_float_push_back(&meter_bar_positions, meter_barx1);
                cstrl_da_float_push_back(&meter_bar_positions, meter_bary0);
                cstrl_da_float_push_back(&meter_bar_positions, meter_barx1);
                cstrl_da_float_push_back(&meter_bar_positions, meter_bary1);
            }
            else
            {
                cstrl_da_float_clear(&meter_positions);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_clear(&meter_bar_positions);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
            }
            cstrl_renderer_modify_positions(meter_render_data, meter_positions.array, 0, 12);
            cstrl_renderer_modify_positions(meter_bar_render_data, meter_bar_positions.array, 0, 12);
        }
        else if (g_player_state == PLAYER1_SHOOTING || g_player_state == PLAYER2_SHOOTING)
        {
            if (!cleared_target_and_meter)
            {
                cstrl_da_float_clear(&target_positions);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_da_float_push_back(&target_positions, 0.0f);
                cstrl_renderer_modify_positions(target_render_data, target_positions.array, 0, 12);

                cstrl_da_float_clear(&meter_positions);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_da_float_push_back(&meter_positions, 0.0f);
                cstrl_renderer_modify_positions(meter_render_data, meter_positions.array, 0, 12);

                cstrl_da_float_clear(&meter_bar_positions);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_da_float_push_back(&meter_bar_positions, 0.0f);
                cstrl_renderer_modify_positions(meter_bar_render_data, meter_bar_positions.array, 0, 12);
                cleared_target_and_meter = true;
            }
            cstrl_da_float_clear(&ball_positions);
            float x0 = ball_position.x - ball_size / 2.0f;
            float y0 = ball_position.y - ball_size / 2.0f;
            float x1 = ball_position.x + ball_size / 2.0f;
            float y1 = ball_position.y + ball_size / 2.0f;
            cstrl_da_float_push_back(&ball_positions, x0);
            cstrl_da_float_push_back(&ball_positions, y1);
            cstrl_da_float_push_back(&ball_positions, x1);
            cstrl_da_float_push_back(&ball_positions, y0);
            cstrl_da_float_push_back(&ball_positions, x0);
            cstrl_da_float_push_back(&ball_positions, y0);
            cstrl_da_float_push_back(&ball_positions, x0);
            cstrl_da_float_push_back(&ball_positions, y1);
            cstrl_da_float_push_back(&ball_positions, x1);
            cstrl_da_float_push_back(&ball_positions, y0);
            cstrl_da_float_push_back(&ball_positions, x1);
            cstrl_da_float_push_back(&ball_positions, y1);
            cstrl_renderer_modify_positions(ball_render_data, ball_positions.array, 0, 12);
        }

        cstrl_use_shader(default_shader);
        // cstrl_texture_bind(floor_texture);
        // cstrl_renderer_draw(floor_render_data);
        cstrl_texture_bind(table_texture);
        cstrl_renderer_draw(table_render_data);
        cstrl_texture_bind(cup_texture);
        cstrl_renderer_draw(cup_render_data);
        cstrl_texture_bind(ball_texture);
        cstrl_renderer_draw(ball_render_data);
        cstrl_texture_bind(player_texture);
        cstrl_renderer_draw(player_render_data);
        cstrl_texture_bind(target_texture);
        cstrl_renderer_draw(target_render_data);
        cstrl_texture_bind(meter_texture);
        cstrl_renderer_draw(meter_render_data);
        cstrl_texture_bind(meter_bar_texture);
        cstrl_renderer_draw(meter_bar_render_data);

        cstrl_ui_begin(&context);
        if (cstrl_ui_container_begin(&context, "Other", 5, 0, WINDOW_HEIGHT - 200, WINDOW_WIDTH, 200, GEN_ID(0), true,
                                     false, 2, &base_layout))
        {
            if (cstrl_ui_button(&context, "Button", 6, 10, 50, 150, 50, GEN_ID(0), &sub_layout))
            {
                printf("Howdy!\n");
            }
            cstrl_ui_container_end(&context);
        }
        cstrl_ui_end(&context);

        cstrl_use_shader(default_shader);
        cstrl_texture_bind(cursor_texture);
        cstrl_renderer_draw(cursor_render_data);

        cstrl_renderer_swap_buffers(&platform_state);
    }

    cstrl_camera_free(camera);
    cstrl_renderer_free_render_data(floor_render_data);
    cstrl_renderer_free_render_data(table_render_data);
    cstrl_renderer_free_render_data(cup_render_data);
    cstrl_renderer_free_render_data(ball_render_data);
    cstrl_renderer_free_render_data(player_render_data);
    cstrl_renderer_free_render_data(target_render_data);
    cstrl_renderer_free_render_data(meter_render_data);
    cstrl_renderer_free_render_data(meter_bar_render_data);
    cstrl_renderer_free_render_data(cursor_render_data);

    cstrl_renderer_shutdown(&platform_state);
    cstrl_platform_shutdown(&platform_state);

    return 0;
}
