#define _CRT_SECURE_NO_WARNINGS
#include "scenes.h"
#include "../entities/player.h"
#include "../gameplay/gameplay.h"
#include "cstrl/cstrl_audio.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_ui.h"
#include "scene_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BASE_SCREEN_DIMENSION_X 320
#define BASE_SCREEN_DIMENSION_Y 180

#define CURSOR_SIZE 8.0f

static cstrl_platform_state *g_platform_state;

static cstrl_shader g_default_shader;
static cstrl_camera *g_main_camera;

static cstrl_render_data *g_cursor_render_data;
static cstrl_texture g_cursor_texture;

static sound_t g_theme_sound;
static source_t g_theme_source;

static CSTRL_PACKED_ENUM{QUICK_GAME_TRANSITION, SIMULATION_TRANSITION, MAIN_MENU_TRANSITION,
                         NO_TRANSITION} g_transition_state = NO_TRANSITION;

static cstrl_ui_context g_ui_context;
static cstrl_ui_layout g_ui_base_layout;
static cstrl_ui_layout g_ui_stats_layout;
static cstrl_ui_layout g_ui_button_layout;
static cstrl_ui_layout g_ui_menu_layout;
static cstrl_ui_layout g_ui_menu_button_layout;

static player_stats_t g_player_stats[MAX_PLAYER_COUNT];
static int g_selected_player = 0;

static CSTRL_PACKED_ENUM{OPTIONS_MENU_STATE, STATS_MENU_STATE, NO_MENU_STATE} g_menu_state = NO_MENU_STATE;

typedef struct main_game_user_data_t
{
    bool simulation_mode;
    int human_players;
} main_game_user_data_t;

static void render_cursor()
{
    float cursor_positions[12] = {0};
    if (gameplay_show_cursor())
    {
        int mouse_x, mouse_y;
        gameplay_get_mouse_position(&mouse_x, &mouse_y);
        float cursorx0 = (float)mouse_x - CURSOR_SIZE / 2.0f;
        float cursory0 = (float)mouse_y - CURSOR_SIZE / 2.0f;
        float cursorx1 = (float)mouse_x + CURSOR_SIZE / 2.0f;
        float cursory1 = (float)mouse_y + CURSOR_SIZE / 2.0f;
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
    cstrl_use_shader(g_default_shader);
    cstrl_texture_bind(g_cursor_texture);
    cstrl_renderer_draw(g_cursor_render_data);
}

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
    // g_main_camera->position.y += (float)BASE_SCREEN_DIMENSION_Y / 4.0;
    cstrl_camera_update(g_main_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
    cstrl_set_uniform_mat4(g_default_shader.program, "view", g_main_camera->view);
    cstrl_set_uniform_mat4(g_default_shader.program, "projection", g_main_camera->projection);

    if (!cstrl_audio_load_ogg("resources/sounds/potential.ogg", &g_theme_sound))
    {
        printf("Failed to load theme song. Oof!\n");
    }
    if (!cstrl_audio_create_source(&g_theme_source))
    {
        printf("Failed to load theme source. Bonkers!\n");
    }

    cstrl_ui_init(&g_ui_context, g_platform_state);
    g_ui_base_layout = (cstrl_ui_layout){0};
    g_ui_base_layout.child_alignment = CSTRL_UI_ALIGN_CENTER;
    g_ui_button_layout = (cstrl_ui_layout){0};
    g_ui_button_layout.color = (cstrl_ui_color){1.0f, 0.0f, 0.0f, 1.0f};
    g_ui_button_layout.border.color = (cstrl_ui_color){1.0f, 1.0f, 1.0f, 1.0f};
    g_ui_button_layout.border.size = (cstrl_ui_border_size){4, 4, 4, 4, 0};
    g_ui_button_layout.font_color = (cstrl_ui_color){1.0f, 1.0f, 1.0f, 1.0f};
    g_ui_button_layout.child_alignment = CSTRL_UI_ALIGN_CENTER;
    g_ui_stats_layout = (cstrl_ui_layout){0};
    g_ui_stats_layout.color = (cstrl_ui_color){0.0f, 0.0f, 0.0f, 1.0f};
    g_ui_stats_layout.border.color = (cstrl_ui_color){0.4f, 0.4f, 0.4f, 1.0f};
    g_ui_stats_layout.border.size = (cstrl_ui_border_size){2, 2, 2, 2, 0};
    g_ui_stats_layout.padding = (cstrl_ui_padding){0.0f, 0.0f, 0.0f, 0.0f};
    g_ui_stats_layout.child_alignment = CSTRL_UI_ALIGN_LEFT;
    g_ui_menu_layout = (cstrl_ui_layout){0};
    g_ui_menu_layout.color = (cstrl_ui_color){0.2f, 0.2f, 0.2f, 1.0f};
    g_ui_menu_layout.border.color = (cstrl_ui_color){0.4f, 0.4f, 0.4f, 1.0f};
    g_ui_menu_layout.border.size = (cstrl_ui_border_size){1, 1, 1, 1, 0};
    g_ui_menu_button_layout = (cstrl_ui_layout){0};
    g_ui_menu_layout.child_alignment = CSTRL_UI_ALIGN_CENTER;
    g_ui_menu_button_layout.color = (cstrl_ui_color){0.4f, 0.4f, 0.4f, 1.0f};
    g_ui_menu_button_layout.border.color = (cstrl_ui_color){0.8f, 0.8f, 0.8f, 1.0f};
    g_ui_menu_button_layout.border.size = (cstrl_ui_border_size){1, 1, 1, 1, 0};
    g_ui_menu_button_layout.font_color = (cstrl_ui_color){0.8f, 0.8f, 0.8f, 1.0f};
    g_ui_menu_button_layout.child_alignment = CSTRL_UI_ALIGN_CENTER;
}

/*
 *
 *  LOGO SCENE
 *
 */

static cstrl_render_data *g_logo_render_data;
static cstrl_texture g_logo_texture;

static int g_tick_counter = 0;

void logo_scene_init(void *user_data)
{
    g_logo_render_data = cstrl_renderer_create_render_data();
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
    cstrl_renderer_add_positions(g_logo_render_data, table_positions, 2, 6);
    cstrl_renderer_add_uvs(g_logo_render_data, table_uvs);
    cstrl_renderer_add_colors(g_logo_render_data, table_colors);

    g_logo_texture =
        cstrl_texture_generate_from_path("resources/textures/beer_pong/logo.png", CSTRL_TEXTURE_FILTER_NEAREST);
}

void logo_scene_update()
{
    g_tick_counter++;
    if (g_tick_counter == 300)
    {
        scene_set(MAIN_MENU_SCENE, NULL);
        cstrl_audio_play(&g_theme_source, &g_theme_sound, true);
        g_tick_counter = 0;
    }
}

void logo_scene_render()
{
    cstrl_use_shader(g_default_shader);
    cstrl_texture_bind(g_logo_texture);
    cstrl_renderer_draw(g_logo_render_data);
}

void logo_scene_shutdown()
{
    cstrl_renderer_free_render_data(g_logo_render_data);
}

/*
 *
 *  MAIN MENU SCENE
 *
 */

void main_menu_scene_init(void *user_data)
{
    cstrl_platform_set_key_callback(g_platform_state, gameplay_key_callback);
    cstrl_platform_set_mouse_position_callback(g_platform_state, gameplay_mouse_position_callback);
    cstrl_platform_set_mouse_button_callback(g_platform_state, gameplay_mouse_button_callback);

    for (int i = 0; i < MAX_PLAYER_COUNT; i++)
    {
        g_player_stats[i].accuracy = 50;
        g_player_stats[i].focus = 50;
        g_player_stats[i].defence = 0;
        g_player_stats[i].charisma = 0;
        g_player_stats[i].tolerance = 0;
    }
    gameplay_init(BASE_SCREEN_DIMENSION_X, BASE_SCREEN_DIMENSION_Y, &g_default_shader, false, 0, g_player_stats);
}

void main_menu_scene_update()
{
    if (g_transition_state == QUICK_GAME_TRANSITION)
    {
        g_transition_state = NO_TRANSITION;
        main_game_user_data_t *user_data = malloc(sizeof(main_game_user_data_t));
        user_data->simulation_mode = false;
        user_data->human_players = PLAYER1;
        scene_set(MAIN_GAME_SCENE, user_data);
        return;
    }
    if (g_transition_state == SIMULATION_TRANSITION)
    {
        g_transition_state = NO_TRANSITION;
        main_game_user_data_t *user_data = malloc(sizeof(main_game_user_data_t));
        user_data->simulation_mode = true;
        user_data->human_players = 0;
        scene_set(MAIN_GAME_SCENE, user_data);
        return;
    }
    gameplay_update();
}

void main_menu_scene_render()
{
    gameplay_render();
    cstrl_ui_begin(&g_ui_context);
    int width, height;
    cstrl_platform_get_window_size(g_platform_state, &width, &height);
    if (cstrl_ui_container_begin(&g_ui_context, "", 0, 0, 0, width, 200, GEN_ID(0), true, false, 2, &g_ui_base_layout))
    {
        if (cstrl_ui_button(&g_ui_context, "Quit", 4, 1780, 20, 120, 50, GEN_ID(0), &g_ui_button_layout))
        {
            cstrl_platform_set_should_exit(true);
        }
        cstrl_ui_container_end(&g_ui_context);
    }
    if (cstrl_ui_container_begin(&g_ui_context, "", 0, 0, height - 200, width, 200, GEN_ID(0), true, false, 2,
                                 &g_ui_base_layout))
    {
        if (cstrl_ui_button(&g_ui_context, "Quick Game", 10, 360, 75, 240, 100, GEN_ID(0), &g_ui_button_layout))
        {
            g_menu_state = STATS_MENU_STATE;
        }
        if (cstrl_ui_button(&g_ui_context, "Simulation", 10, 840, 75, 240, 100, GEN_ID(0), &g_ui_button_layout))
        {
            g_menu_state = STATS_MENU_STATE;
        }
        if (cstrl_ui_button(&g_ui_context, "Options", 7, 1320, 75, 240, 100, GEN_ID(0), &g_ui_button_layout))
        {
            g_menu_state = OPTIONS_MENU_STATE;
        }
        cstrl_ui_container_end(&g_ui_context);
    }
    if (g_menu_state == OPTIONS_MENU_STATE)
    {
        if (cstrl_ui_container_begin(&g_ui_context, "", 0, width / 2 - 200, height / 2 - 300, 400, 600, GEN_ID(0), true,
                                     false, 2, &g_ui_menu_layout))
        {
            if (cstrl_ui_button(&g_ui_context, "Mute", 4, 20, 20, 144, 60, GEN_ID(0), &g_ui_menu_button_layout))
            {
                cstrl_audio_stop(&g_theme_source);
            }
            cstrl_ui_container_end(&g_ui_context);
        }
    }
    else if (g_menu_state == STATS_MENU_STATE)
    {
        if (cstrl_ui_container_begin(&g_ui_context, "", 0, width / 2 - 200, height / 2 - 300, 400, 600, GEN_ID(0), true,
                                     false, 2, &g_ui_menu_layout))
        {
            int acc_text_width = cstrl_ui_text_width(&g_ui_context, "ACC", 3, 1.0f);
            int prev_text_height = cstrl_ui_text_height(&g_ui_context, "ACC", 3, 1.0f);
            cstrl_ui_text(&g_ui_context, "ACC", 3, 10 + acc_text_width / 2, 30, 60, 60, GEN_ID(0),
                          CSTRL_UI_ALIGN_CENTER, &g_ui_menu_button_layout);
            if (g_player_stats[g_selected_player].accuracy > 0)
            {
                if (cstrl_ui_button(&g_ui_context, "-", 1, acc_text_width + 20, 20, 30, 30, GEN_ID(0),
                                    &g_ui_menu_button_layout))
                {
                    g_player_stats[g_selected_player].accuracy--;
                }
            }
            char buffer[4];
            sprintf(buffer, "%3d", g_player_stats[g_selected_player].accuracy);
            int text_width = cstrl_ui_text_width(&g_ui_context, buffer, 3, 1.0f);
            int text_height = cstrl_ui_text_height(&g_ui_context, buffer, 3, 1.0f);
            cstrl_ui_text(&g_ui_context, buffer, 3, 120 - text_width / 2 + acc_text_width, 20 + text_height / 2, 60, 60,
                          GEN_ID(0), CSTRL_UI_ALIGN_CENTER, &g_ui_menu_button_layout);
            if (g_player_stats[g_selected_player].accuracy < 100)
            {
                if (cstrl_ui_button(&g_ui_context, "+", 1, 120 + text_width + acc_text_width, 20, 30, 30, GEN_ID(0),
                                    &g_ui_menu_button_layout))
                {
                    g_player_stats[g_selected_player].accuracy++;
                }
            }

            int foc_text_width = cstrl_ui_text_width(&g_ui_context, "ACC", 3, 1.0f);
            cstrl_ui_text(&g_ui_context, "FOC", 3, 10 + foc_text_width / 2, 30 + prev_text_height, 60, 60, GEN_ID(0),
                          CSTRL_UI_ALIGN_CENTER, &g_ui_menu_button_layout);
            if (g_player_stats[g_selected_player].focus > 0)
            {
                if (cstrl_ui_button(&g_ui_context, "-", 1, acc_text_width + 20, 20 + prev_text_height, 30, 30,
                                    GEN_ID(0), &g_ui_menu_button_layout))
                {
                    g_player_stats[g_selected_player].focus--;
                }
            }
            sprintf(buffer, "%3d", g_player_stats[g_selected_player].focus);
            text_width = cstrl_ui_text_width(&g_ui_context, buffer, 3, 1.0f);
            text_height = cstrl_ui_text_height(&g_ui_context, buffer, 3, 1.0f);
            cstrl_ui_text(&g_ui_context, buffer, 3, 120 - text_width / 2 + acc_text_width,
                          20 + text_height / 2 + prev_text_height, 60, 60, GEN_ID(0), CSTRL_UI_ALIGN_CENTER,
                          &g_ui_menu_button_layout);
            if (g_player_stats[g_selected_player].focus < 100)
            {
                if (cstrl_ui_button(&g_ui_context, "+", 1, 120 + text_width + acc_text_width, 20 + prev_text_height, 30,
                                    30, GEN_ID(0), &g_ui_menu_button_layout))
                {
                    g_player_stats[g_selected_player].focus++;
                }
            }

            prev_text_height += cstrl_ui_text_height(&g_ui_context, "FOC", 3, 1.0f);
            int player_text_width = cstrl_ui_text_width(&g_ui_context, "PLY", 3, 1.0f);
            cstrl_ui_text(&g_ui_context, "PLY", 3, 10 + foc_text_width / 2, 30 + prev_text_height, 60, 60, GEN_ID(0),
                          CSTRL_UI_ALIGN_CENTER, &g_ui_menu_button_layout);
            if (g_selected_player > 0)
            {
                if (cstrl_ui_button(&g_ui_context, "-", 1, acc_text_width + 20, 20 + prev_text_height, 30, 30,
                                    GEN_ID(0), &g_ui_menu_button_layout))
                {
                    g_selected_player--;
                }
            }
            sprintf(buffer, "%d", g_selected_player);
            text_width = cstrl_ui_text_width(&g_ui_context, buffer, 1, 1.0f);
            text_height = cstrl_ui_text_height(&g_ui_context, buffer, 1, 1.0f);
            cstrl_ui_text(&g_ui_context, buffer, 1, 120 - text_width / 2 + acc_text_width,
                          20 + text_height / 2 + prev_text_height, 60, 60, GEN_ID(0), CSTRL_UI_ALIGN_CENTER,
                          &g_ui_menu_button_layout);
            if (g_selected_player < MAX_PLAYER_COUNT - 1)
            {
                if (cstrl_ui_button(&g_ui_context, "+", 1, 120 + text_width + acc_text_width, 20 + prev_text_height, 30,
                                    30, GEN_ID(0), &g_ui_menu_button_layout))
                {
                    g_selected_player++;
                }
            }

            if (cstrl_ui_button(&g_ui_context, "Play", 4, 20, 520, 144, 60, GEN_ID(0), &g_ui_menu_button_layout))
            {
                // g_transition_state = QUICK_GAME_TRANSITION;
                g_transition_state = SIMULATION_TRANSITION;
            }
            cstrl_ui_container_end(&g_ui_context);
        }
    }
    cstrl_ui_end(&g_ui_context);
    render_cursor();
}

void main_menu_scene_shutdown()
{
    gameplay_shutdown();
}

/*
 *
 *  MAIN GAME SCENE
 *
 */

void main_game_scene_init(void *user_data)
{
    cstrl_platform_set_key_callback(g_platform_state, gameplay_key_callback);
    cstrl_platform_set_mouse_position_callback(g_platform_state, gameplay_mouse_position_callback);
    cstrl_platform_set_mouse_button_callback(g_platform_state, gameplay_mouse_button_callback);

    main_game_user_data_t *data = user_data;
    gameplay_init(BASE_SCREEN_DIMENSION_X, BASE_SCREEN_DIMENSION_Y, &g_default_shader, data->simulation_mode,
                  data->human_players, g_player_stats);

    free(user_data);
}

void main_game_scene_update()
{
    if (g_transition_state == MAIN_MENU_TRANSITION)
    {
        g_transition_state = NO_TRANSITION;
        scene_set(MAIN_MENU_SCENE, NULL);
        return;
    }
    gameplay_update();
}

void main_game_scene_render()
{
    gameplay_render();
    cstrl_ui_begin(&g_ui_context);
    int width, height;
    cstrl_platform_get_window_size(g_platform_state, &width, &height);
    if (cstrl_ui_container_begin(&g_ui_context, "", 0, 0, 0, width, 200, GEN_ID(0), true, false, 2, &g_ui_stats_layout))
    {
        char buffer[64];
        switch (gameplay_get_current_player_turn())
        {
        case 0:
            strcpy(buffer, "Player 1 Turn");
            break;
        case 1:
            strcpy(buffer, "Player 2 Turn");
            break;
        case 2:
            strcpy(buffer, "Player 3 Turn");
            break;
        case 3:
            strcpy(buffer, "Player 4 Turn");
            break;
        default:
            strcpy(buffer, "Transition...");
            break;
        }
        cstrl_ui_text(&g_ui_context, buffer, 13, width - cstrl_ui_text_width(&g_ui_context, buffer, 13, 1.0) - 20,
                      200 - cstrl_ui_text_height(&g_ui_context, buffer, 13, 1.0), 100, 50, GEN_ID(0),
                      CSTRL_UI_ALIGN_CENTER, &g_ui_button_layout);
        int team1, team2;
        gameplay_get_team_wins(&team1, &team2);
        sprintf(buffer, "Team 1: %5d | Team 2: %5d", team1, team2);
        cstrl_ui_text(&g_ui_context, buffer, 29, width - cstrl_ui_text_width(&g_ui_context, buffer, 29, 1.0) - 20,
                      100 - cstrl_ui_text_height(&g_ui_context, buffer, 13, 1.0), 100, 50, GEN_ID(0),
                      CSTRL_UI_ALIGN_CENTER, &g_ui_button_layout);
        for (int i = 0; i < MAX_PLAYER_COUNT; i++)
        {
            player_metrics_t metrics = gameplay_get_player_metrics(i);
            sprintf(buffer, "Player %d Metrics", i + 1);
            int start = (cstrl_ui_text_width(&g_ui_context, buffer, 16, 1.0) + 40) * (i + 1) - 200;
            cstrl_ui_text(&g_ui_context, buffer, 16, start, 20, 100, 50, GEN_ID(i), CSTRL_UI_ALIGN_CENTER,
                          &g_ui_button_layout);
            sprintf(buffer, "CM: %5d", metrics.cups_made);
            cstrl_ui_text(&g_ui_context, buffer, 9, start,
                          20 + 10 + cstrl_ui_text_height(&g_ui_context, buffer, 13, 1.0), 100, 50, GEN_ID(i),
                          CSTRL_UI_ALIGN_CENTER, &g_ui_button_layout);
            sprintf(buffer, "SM: %5d", metrics.successful_shots);
            cstrl_ui_text(&g_ui_context, buffer, 9, start,
                          20 + (10 + cstrl_ui_text_height(&g_ui_context, buffer, 13, 1.0) * 2), 100, 50, GEN_ID(i),
                          CSTRL_UI_ALIGN_CENTER, &g_ui_button_layout);
            sprintf(buffer, "SA: %5d", metrics.attempted_shots);
            cstrl_ui_text(&g_ui_context, buffer, 9, start,
                          20 + (10 + cstrl_ui_text_height(&g_ui_context, buffer, 13, 1.0) * 3), 100, 50, GEN_ID(i),
                          CSTRL_UI_ALIGN_CENTER, &g_ui_button_layout);
            float avg = 0.0f;
            if (metrics.attempted_shots != 0)
            {
                avg = (float)metrics.successful_shots / (float)metrics.attempted_shots * 100.0f;
            }
            sprintf(buffer, "Avg: %3.3f", avg);
            cstrl_ui_text(&g_ui_context, buffer, 10, start,
                          20 + (10 + cstrl_ui_text_height(&g_ui_context, buffer, 13, 1.0) * 4), 100, 50, GEN_ID(i),
                          CSTRL_UI_ALIGN_CENTER, &g_ui_button_layout);
        }
        cstrl_ui_container_end(&g_ui_context);
    }
    if (cstrl_ui_container_begin(&g_ui_context, "", 0, 0, height - 200, width, 200, GEN_ID(0), true, false, 2,
                                 &g_ui_base_layout))
    {
        if (cstrl_ui_button(&g_ui_context, "Reset", 5, 360, 75, 240, 100, GEN_ID(0), &g_ui_button_layout))
        {
            gameplay_reset();
        }
        if (cstrl_ui_button(&g_ui_context, "Pause", 5, 840, 75, 240, 100, GEN_ID(0), &g_ui_button_layout))
        {
            gameplay_toggle_pause();
        }
        if (cstrl_ui_button(&g_ui_context, "Main Menu", 9, 1320, 75, 240, 100, GEN_ID(0), &g_ui_button_layout))
        {
            g_transition_state = MAIN_MENU_TRANSITION;
        }
        cstrl_ui_container_end(&g_ui_context);
    }
    if (gameplay_team_can_rerack(0))
    {
        if (cstrl_ui_container_begin(&g_ui_context, "", 0, 300, 740, 200, 100, GEN_ID(0), true, false, 2,
                                     &g_ui_base_layout))
        {
            if (cstrl_ui_button(&g_ui_context, "Rerack", 6, 0, 0, 144, 60, GEN_ID(0), &g_ui_button_layout))
            {
                gameplay_team_rerack(0);
            }
        }
    }
    cstrl_ui_end(&g_ui_context);
    render_cursor();
}

void main_game_scene_shutdown()
{
    gameplay_shutdown();
}

void scenes_clean()
{
    cstrl_camera_free(g_main_camera);
    cstrl_renderer_free_render_data(g_cursor_render_data);
    cstrl_audio_stop(&g_theme_source);
    cstrl_audio_unload(&g_theme_sound);
}
