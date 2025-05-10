#include "gameplay_scene.h"
#include "../entities/butterfly.h"
#include "../entities/guy.h"
#include "../random/cozy_random.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_renderer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_GUY_COUNT 5000
static bool g_should_transition = false;

static cstrl_shader g_default_shader;
static cstrl_camera *g_main_camera;
static cstrl_render_data *g_background_render_data;
static cstrl_render_data *g_butterfly_render_data;
static cstrl_render_data *g_flower_render_data;
static cstrl_render_data *g_guy_render_data;
static cstrl_render_data *g_guy_skin_type_render_data;
static cstrl_texture g_background_texture;
static cstrl_texture g_butterfly_texture;
static cstrl_texture g_flower_texture;
static cstrl_texture g_guy_texture;
static cstrl_texture g_skin_type_texture;

static butterflies_t g_butterflies;
static guys_t g_guys;

static float *g_guy_positions;
static float *g_guy_uvs;
static float *g_guy_colors;

void gameplay_scene_init(cstrl_platform_state *platform_state)
{
    g_default_shader =
        cstrl_load_shaders_from_files("resources/shaders/default.vert", "resources/shaders/default.frag");

    int width, height;
    cstrl_platform_get_window_size(platform_state, &width, &height);
    g_main_camera = cstrl_camera_create(width, height, true);
    cstrl_camera_update(g_main_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);

    g_background_render_data = cstrl_renderer_create_render_data();
    float background_positions[20 * 12 * 12] = {0};
    for (int i = 0; i < 20 * 12; i++)
    {
        float x0 = i % 20 * 64.0f;
        float y0 = floorf((float)i / 20.0f) * 64.0f;
        float x1 = x0 + 64.0f;
        float y1 = y0 + 64.0f;
        background_positions[i * 12] = x0;
        background_positions[i * 12 + 1] = y1;
        background_positions[i * 12 + 2] = x1;
        background_positions[i * 12 + 3] = y0;
        background_positions[i * 12 + 4] = x0;
        background_positions[i * 12 + 5] = y0;
        background_positions[i * 12 + 6] = x0;
        background_positions[i * 12 + 7] = y1;
        background_positions[i * 12 + 8] = x1;
        background_positions[i * 12 + 9] = y0;
        background_positions[i * 12 + 10] = x1;
        background_positions[i * 12 + 11] = y1;
    }
    float background_uvs[20 * 12 * 12] = {0};
    for (int i = 0; i < 20 * 12; i++)
    {
        float u0 = cozy_random_int(0, 5) / 6.0f;
        float v0 = cozy_random_int(0, 1) / 2.0f;
        float u1 = u0 + 1.0f / 6.0f;
        float v1 = v0 + 1.0f / 2.0f;
        background_uvs[i * 12] = u0;
        background_uvs[i * 12 + 1] = v1;
        background_uvs[i * 12 + 2] = u1;
        background_uvs[i * 12 + 3] = v0;
        background_uvs[i * 12 + 4] = u0;
        background_uvs[i * 12 + 5] = v0;
        background_uvs[i * 12 + 6] = u0;
        background_uvs[i * 12 + 7] = v1;
        background_uvs[i * 12 + 8] = u1;
        background_uvs[i * 12 + 9] = v0;
        background_uvs[i * 12 + 10] = u1;
        background_uvs[i * 12 + 11] = v1;
    }
    float background_colors[20 * 12 * 24];
    for (int i = 0; i < 20 * 12 * 6; i++)
    {
        background_colors[i * 4] = 1.0f;
        background_colors[i * 4 + 1] = 1.0f;
        background_colors[i * 4 + 2] = 1.0f;
        background_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(g_background_render_data, background_positions, 2, 20 * 12 * 6);
    cstrl_renderer_add_uvs(g_background_render_data, background_uvs);
    cstrl_renderer_add_colors(g_background_render_data, background_colors);
    g_background_texture =
        cstrl_texture_generate_from_path("resources/textures/cozy_game/background.png", CSTRL_TEXTURE_FILTER_NEAREST);

    g_butterfly_render_data = cstrl_renderer_create_render_data();
    float butterfly_positions[12] = {0};
    float butterfly_uvs[12] = {0};
    float butterfly_colors[24] = {0};
    for (int i = 0; i < 6; i++)
    {
        butterfly_colors[i * 4] = 1.0f;
        butterfly_colors[i * 4 + 1] = 1.0f;
        butterfly_colors[i * 4 + 2] = 1.0f;
        butterfly_colors[i * 4 + 3] = 1.0f;
    }
    cstrl_renderer_add_positions(g_butterfly_render_data, butterfly_positions, 2, 6);
    cstrl_renderer_add_uvs(g_butterfly_render_data, butterfly_uvs);
    cstrl_renderer_add_colors(g_butterfly_render_data, butterfly_colors);
    g_butterfly_texture =
        cstrl_texture_generate_from_path("resources/textures/cozy_game/butterfly.png", CSTRL_TEXTURE_FILTER_NEAREST);

    g_guy_render_data = cstrl_renderer_create_render_data();

    g_guy_positions = malloc(sizeof(float) * TOTAL_GUY_COUNT * 24);
    if (g_guy_positions)
    {
        memset(g_guy_positions, 0, sizeof(float) * TOTAL_GUY_COUNT * 24);
    }
    else
    {
        printf("Failed to malloc g_guy_positions\n");
    }
    g_guy_uvs = malloc(sizeof(float) * TOTAL_GUY_COUNT * 24);
    if (g_guy_uvs)
    {
        memset(g_guy_uvs, 0, sizeof(float) * TOTAL_GUY_COUNT * 24);
    }
    else
    {
        printf("Failed to malloc g_guy_uvs\n");
    }
    g_guy_colors = malloc(sizeof(float) * TOTAL_GUY_COUNT * 48);
    if (g_guy_colors)
    {
        memset(g_guy_colors, 0, sizeof(float) * TOTAL_GUY_COUNT * 48);
    }
    else
    {
        printf("Failed to malloc g_guy_colors\n");
    }

    cstrl_renderer_add_positions(g_guy_render_data, g_guy_positions, 2, TOTAL_GUY_COUNT * 6);
    cstrl_renderer_add_uvs(g_guy_render_data, g_guy_uvs);
    cstrl_renderer_add_colors(g_guy_render_data, g_guy_colors);
    g_guy_texture =
        cstrl_texture_generate_from_path("resources/textures/cozy_game/guys.png", CSTRL_TEXTURE_FILTER_NEAREST);

    cstrl_set_uniform_mat4(g_default_shader.program, "view", g_main_camera->view);
    cstrl_set_uniform_mat4(g_default_shader.program, "projection", g_main_camera->projection);

    butterflies_init(&g_butterflies);

    butterflies_add(&g_butterflies, (vec2){cozy_random_float(0.0f, (float)width - BUTTERFLY_SIZE),
                                           cozy_random_float(0.0f, (float)height - BUTTERFLY_SIZE)});

    guys_init(&g_guys);
    for (int i = 0; i < TOTAL_GUY_COUNT; i++)
    {
        vec3 random_color =
            (vec3){cozy_random_float(0.0f, 1.0f), cozy_random_float(0.0f, 1.0f), cozy_random_float(0.0f, 1.0f)};
        guys_add(&g_guys, (vec2){cozy_random_float(0, (float)width), cozy_random_float(0, (float)height)},
                 random_color);
    }
}

bool gameplay_scene_update(cstrl_platform_state *platform_state)
{
    butterflies_update(&g_butterflies);
    guys_update(&g_guys);
    return g_should_transition;
}

void gameplay_scene_render(cstrl_platform_state *platform_state)
{
    float butterfly_positions[g_butterflies.count * 12];
    float butterfly_uvs[g_butterflies.count * 12];
    for (int i = 0; i < g_butterflies.count; i++)
    {
        if (!g_butterflies.active[i])
        {
            continue;
        }
        float x0 = g_butterflies.position[i].x - BUTTERFLY_SIZE / 2.0f;
        float y0 = g_butterflies.position[i].y - BUTTERFLY_SIZE / 2.0f;
        float x1 = g_butterflies.position[i].x + BUTTERFLY_SIZE / 2.0f;
        float y1 = g_butterflies.position[i].y + BUTTERFLY_SIZE / 2.0f;
        butterfly_positions[i * 12] = x0;
        butterfly_positions[i * 12 + 1] = y1;
        butterfly_positions[i * 12 + 2] = x1;
        butterfly_positions[i * 12 + 3] = y0;
        butterfly_positions[i * 12 + 4] = x0;
        butterfly_positions[i * 12 + 5] = y0;
        butterfly_positions[i * 12 + 6] = x0;
        butterfly_positions[i * 12 + 7] = y1;
        butterfly_positions[i * 12 + 8] = x1;
        butterfly_positions[i * 12 + 9] = y0;
        butterfly_positions[i * 12 + 10] = x1;
        butterfly_positions[i * 12 + 11] = y1;

        float u0 = (float)g_butterflies.animation_frame[i] / BUTTERFLY_ANIMATION_TOTAL_FRAMES;
        float v0 = 0.0f;
        float u1 = (float)(g_butterflies.animation_frame[i] + 1) / BUTTERFLY_ANIMATION_TOTAL_FRAMES;
        float v1 = 1.0f / BUTTERFLY_ANIMATION_TOTAL_TYPES;
        butterfly_uvs[i * 12] = u0;
        butterfly_uvs[i * 12 + 1] = v1;
        butterfly_uvs[i * 12 + 2] = u1;
        butterfly_uvs[i * 12 + 3] = v0;
        butterfly_uvs[i * 12 + 4] = u0;
        butterfly_uvs[i * 12 + 5] = v0;
        butterfly_uvs[i * 12 + 6] = u0;
        butterfly_uvs[i * 12 + 7] = v1;
        butterfly_uvs[i * 12 + 8] = u1;
        butterfly_uvs[i * 12 + 9] = v0;
        butterfly_uvs[i * 12 + 10] = u1;
        butterfly_uvs[i * 12 + 11] = v1;
    }
    cstrl_renderer_modify_render_attributes(g_butterfly_render_data, butterfly_positions, butterfly_uvs, NULL,
                                            g_butterflies.count * 6);

    for (int i = 0; i < g_guys.count; i++)
    {
        float x0 = 0.0f;
        float y0 = 0.0f;
        float x1 = 0.0f;
        float y1 = 0.0f;

        if (g_guys.active[i])
        {
            if (g_guys.velocity[i].x < 0.0f)
            {
                x0 = g_guys.position[i].x - GUY_SIZE / 2.0f;
                y0 = g_guys.position[i].y - GUY_SIZE / 2.0f;
                x1 = g_guys.position[i].x + GUY_SIZE / 2.0f;
                y1 = g_guys.position[i].y + GUY_SIZE / 2.0f;
            }
            else
            {
                x1 = g_guys.position[i].x - GUY_SIZE / 2.0f;
                y0 = g_guys.position[i].y - GUY_SIZE / 2.0f;
                x0 = g_guys.position[i].x + GUY_SIZE / 2.0f;
                y1 = g_guys.position[i].y + GUY_SIZE / 2.0f;
            }
        }

        g_guy_positions[i * 24] = x0;
        g_guy_positions[i * 24 + 1] = y1;
        g_guy_positions[i * 24 + 2] = x1;
        g_guy_positions[i * 24 + 3] = y0;
        g_guy_positions[i * 24 + 4] = x0;
        g_guy_positions[i * 24 + 5] = y0;
        g_guy_positions[i * 24 + 6] = x0;
        g_guy_positions[i * 24 + 7] = y1;
        g_guy_positions[i * 24 + 8] = x1;
        g_guy_positions[i * 24 + 9] = y0;
        g_guy_positions[i * 24 + 10] = x1;
        g_guy_positions[i * 24 + 11] = y1;

        if (g_guys.animate[i])
        {
            x0 = 0.0f;
            y0 = 0.0f;
            x1 = 0.0f;
            y1 = 0.0f;
        }
        g_guy_positions[i * 24 + 12] = x0;
        g_guy_positions[i * 24 + 13] = y1;
        g_guy_positions[i * 24 + 14] = x1;
        g_guy_positions[i * 24 + 15] = y0;
        g_guy_positions[i * 24 + 16] = x0;
        g_guy_positions[i * 24 + 17] = y0;
        g_guy_positions[i * 24 + 18] = x0;
        g_guy_positions[i * 24 + 19] = y1;
        g_guy_positions[i * 24 + 20] = x1;
        g_guy_positions[i * 24 + 21] = y0;
        g_guy_positions[i * 24 + 22] = x1;
        g_guy_positions[i * 24 + 23] = y1;

        float u0 = 0.0f;
        float v0 = 0.0f;
        float u1 = 0.0f;
        float v1 = 0.0f;
        if (g_guys.active[i])
        {
            // u0 = (float)g_guys.animation_frame[i] / GUY_ANIMATION_TOTAL_FRAMES;
            // v0 = (float)g_guys.type[i] / GUY_TOTAL_TYPES;
            // u1 = (float)(g_guys.animation_frame[i] + 1) / GUY_ANIMATION_TOTAL_FRAMES;
            // v1 = (float)(g_guys.type[i] + 1) / GUY_TOTAL_TYPES;
            if (!g_guys.animate[i])
            {
                u0 = 1.0f / GUY_TOTAL_COLS;
                v0 = (float)(g_guys.type[i] % 2) / GUY_TOTAL_ROWS;
                u1 = 2.0f / GUY_TOTAL_COLS;
                v1 = (float)(g_guys.type[i] % 2 + 1) / GUY_TOTAL_ROWS;
            }
            else
            {
                u0 = (float)g_guys.animation_frame[i] / GUY_TOTAL_COLS;
                v0 = (float)(GUY_TOTAL_TYPES + g_guys.type[i] % 2) / (float)GUY_TOTAL_ROWS;
                u1 = (float)(g_guys.animation_frame[i] + 1) / GUY_TOTAL_COLS;
                v1 = (float)(GUY_TOTAL_TYPES + g_guys.type[i] % 2 + 1) / (float)GUY_TOTAL_ROWS;
            }
        }
        g_guy_uvs[i * 24] = u0;
        g_guy_uvs[i * 24 + 1] = v1;
        g_guy_uvs[i * 24 + 2] = u1;
        g_guy_uvs[i * 24 + 3] = v0;
        g_guy_uvs[i * 24 + 4] = u0;
        g_guy_uvs[i * 24 + 5] = v0;
        g_guy_uvs[i * 24 + 6] = u0;
        g_guy_uvs[i * 24 + 7] = v1;
        g_guy_uvs[i * 24 + 8] = u1;
        g_guy_uvs[i * 24 + 9] = v0;
        g_guy_uvs[i * 24 + 10] = u1;
        g_guy_uvs[i * 24 + 11] = v1;

        if (g_guys.active[i])
        {
            u0 = 0.0f;
            v0 = (float)g_guys.type[i] / GUY_TOTAL_ROWS;
            u1 = 1.0f / GUY_TOTAL_COLS;
            v1 = (float)(g_guys.type[i] + 1) / GUY_TOTAL_ROWS;
        }
        g_guy_uvs[i * 24 + 12] = u0;
        g_guy_uvs[i * 24 + 13] = v1;
        g_guy_uvs[i * 24 + 14] = u1;
        g_guy_uvs[i * 24 + 15] = v0;
        g_guy_uvs[i * 24 + 16] = u0;
        g_guy_uvs[i * 24 + 17] = v0;
        g_guy_uvs[i * 24 + 18] = u0;
        g_guy_uvs[i * 24 + 19] = v1;
        g_guy_uvs[i * 24 + 20] = u1;
        g_guy_uvs[i * 24 + 21] = v0;
        g_guy_uvs[i * 24 + 22] = u1;
        g_guy_uvs[i * 24 + 23] = v1;

        for (int j = 0; j < 6; j++)
        {
            if (!g_guys.animate[i])
            {
                g_guy_colors[i * 48 + j * 4] = g_guys.color[i].x;
                g_guy_colors[i * 48 + j * 4 + 1] = g_guys.color[i].y;
                g_guy_colors[i * 48 + j * 4 + 2] = g_guys.color[i].z;
                g_guy_colors[i * 48 + j * 4 + 3] = 1.0f;
            }
            else
            {
                g_guy_colors[i * 48 + j * 4] = 1.0f;
                g_guy_colors[i * 48 + j * 4 + 1] = 1.0f;
                g_guy_colors[i * 48 + j * 4 + 2] = 1.0f;
                g_guy_colors[i * 48 + j * 4 + 3] = 1.0f;
            }
        }
        for (int j = 6; j < 12; j++)
        {
            g_guy_colors[i * 48 + j * 4] = 1.0f;
            g_guy_colors[i * 48 + j * 4 + 1] = 1.0f;
            g_guy_colors[i * 48 + j * 4 + 2] = 1.0f;
            g_guy_colors[i * 48 + j * 4 + 3] = 1.0f;
        }
    }
    cstrl_renderer_modify_render_attributes(g_guy_render_data, g_guy_positions, g_guy_uvs, g_guy_colors,
                                            g_guys.count * 6);

    cstrl_use_shader(g_default_shader);

    cstrl_texture_bind(g_background_texture);
    cstrl_renderer_draw(g_background_render_data);

    cstrl_texture_bind(g_guy_texture);
    cstrl_renderer_draw(g_guy_render_data);

    cstrl_texture_bind(g_butterfly_texture);
    cstrl_renderer_draw(g_butterfly_render_data);
}

void gameplay_scene_shutdown(cstrl_platform_state *platform_state)
{
    free(g_guy_positions);
    free(g_guy_uvs);
    free(g_guy_colors);
    cstrl_renderer_free_render_data(g_butterfly_render_data);
    cstrl_renderer_free_render_data(g_guy_render_data);
}
