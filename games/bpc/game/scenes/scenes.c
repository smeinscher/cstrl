#include "scenes.h"
#include "../gameplay/gameplay.h"
#include "cstrl/cstrl_audio.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include "scene_manager.h"
#include <stdio.h>

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

    gameplay_init(BASE_SCREEN_DIMENSION_X, BASE_SCREEN_DIMENSION_Y, &g_default_shader, false);
    cstrl_audio_play(&g_theme_source, &g_theme_sound, true);
}

void main_menu_scene_update()
{
    gameplay_update();
}

void main_menu_scene_render()
{
    gameplay_render();
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
}

void main_game_scene_update()
{
}

void main_game_scene_render()
{
}

void main_game_scene_shutdown()
{
}

void scenes_clean()
{
    cstrl_camera_free(g_main_camera);
    cstrl_renderer_free_render_data(g_cursor_render_data);
    cstrl_audio_stop(&g_theme_source);
    cstrl_audio_unload(&g_theme_sound);
}
