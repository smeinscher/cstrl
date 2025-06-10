#include "gameplay_scene.h"
#include "../entities/guy.h"
#include "../entities/hero.h"
#include "../entities/projectile.h"
#include "../random/cozy_random.h"
#include "../scenes/scene_manager.h"
#include "../ui/cozy_ui.h"
#include "cstrl/cstrl_camera.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_model.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include "scene_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TOTAL_GUY_COUNT 50
// #define COLLISION_DEBUG

static cstrl_shader g_default_shader;
static cstrl_shader g_projectile_shader;
#ifdef COLLISION_DEBUG
static cstrl_shader g_collision_shader;
#endif
static cstrl_camera *g_main_camera;
static cstrl_render_data *g_guy_render_data;
static cstrl_render_data *g_projectile_render_data;
static cstrl_render_data *g_building_render_data;
static cstrl_render_data *g_ground_render_data;
static cstrl_render_data *g_car_render_data;
#ifdef COLLISION_DEBUG
static cstrl_render_data *g_collision_render_data;
#endif
static cstrl_texture g_flower_texture;
static cstrl_texture g_guy_texture;
static cstrl_texture g_skin_type_texture;
static cstrl_texture g_ground_texture;

static mesh_t g_building_mesh;
static mesh_t g_car_mesh;

static guys_t g_guys;
static hero_t g_hero;
static projectiles_t g_projectiles;

static vec3 g_hero_prev_point0;
static vec3 g_hero_prev_point1;
static vec3 g_hero_prev_point2;
static vec3 g_hero_prev_point3;

static hero_movement_t g_hero_movement = 0;

static float *g_guy_positions;
static float *g_guy_uvs;
static float *g_guy_colors;
static int *g_guy_indices;

static int g_mouse_x;
static int g_mouse_y;

static aabb_tree_t g_aabb_tree;

static bool g_paused = true;

static int g_screen_width;
static int g_screen_height;

static float g_camera_vertical_angle = 0.0f;

static void gameplay_scene_key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    switch (key)
    {
    case CSTRL_KEY_ESCAPE:
        scene_set(MAIN_MENU_SCENE, state);
        break;
    case CSTRL_KEY_W:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_hero_movement |= MOVE_UP;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_hero_movement ^= MOVE_UP;
        }
        break;
    case CSTRL_KEY_S:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_hero_movement |= MOVE_DOWN;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_hero_movement ^= MOVE_DOWN;
        }
        break;
    case CSTRL_KEY_A:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_hero_movement |= MOVE_LEFT;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_hero_movement ^= MOVE_LEFT;
        }
        break;
    case CSTRL_KEY_D:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_hero_movement |= MOVE_RIGHT;
        }
        else if (action == CSTRL_ACTION_RELEASE)
        {
            g_hero_movement ^= MOVE_RIGHT;
        }
        break;
    case CSTRL_KEY_Q:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_camera_vertical_angle += 0.01f;
            cstrl_camera_rotate_around_point(g_main_camera, g_hero.position, g_camera_vertical_angle, 0.0f);
        }
        break;
    case CSTRL_KEY_E:
        if (action == CSTRL_ACTION_PRESS)
        {
            g_camera_vertical_angle -= 0.01f;
            cstrl_camera_rotate_around_point(g_main_camera, g_hero.position, g_camera_vertical_angle, 0.0f);
        }
        break;
    default:
        break;
    }
}

static void gameplay_scene_mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos)
{
    g_mouse_x = xpos;
    g_mouse_y = ypos;
}

static void gameplay_scene_mouse_button_callback(cstrl_platform_state *state, int button, int action, int mods)
{
    if (button == CSTRL_MOUSE_BUTTON_LEFT)
    {
        if (action == CSTRL_ACTION_PRESS && !g_paused)
        {
            float x = 1.0f - (2.0f * g_mouse_x) / g_main_camera->viewport.x;
            float y = 1.0f - (2.0f * g_mouse_y) / g_main_camera->viewport.y;

            vec4 ray_clip = {x, y, -1.0f, 1.0f};
            vec4 ray_eye = cstrl_vec4_mult_mat4(ray_clip, cstrl_mat4_inverse(g_main_camera->projection));
            ray_eye = (vec4){ray_eye.x, ray_eye.y, -1.0f, 0.0f};

            vec4 ray_world =
                cstrl_vec4_mult_mat4(ray_eye, cstrl_mat4_transpose(cstrl_mat4_inverse(g_main_camera->view)));

            ray_world = cstrl_vec4_normalize(ray_world);

            float t = g_main_camera->position.y / ray_world.y;
            vec3 intersection = {g_main_camera->position.x + ray_world.x * t, 0.0f,
                                 g_main_camera->position.z - ray_world.z * t};
            vec3 position = {g_hero.position.x, 0.0f, g_hero.position.z};
            vec3 velocity = cstrl_vec3_normalize(cstrl_vec3_sub(intersection, position));
            projectiles_add(&g_projectiles, g_hero.position, velocity, (vec3){0.67f, 0.03f, 0.03f});
        }
    }
}

static void get_points(vec3 *p0, vec3 *p1, vec3 *p2, vec3 *p3, transform_t transform)
{
    float x0 = -0.5f;
    float x1 = 0.5f;
    float y1 = -0.5f;
    float y0 = 0.5f;
    float z = 0.0f;

    *p0 = cstrl_vec3_mult((vec3){x0, y0, z}, transform.scale);
    *p1 = cstrl_vec3_mult((vec3){x1, y0, z}, transform.scale);
    *p2 = cstrl_vec3_mult((vec3){x1, y1, z}, transform.scale);
    *p3 = cstrl_vec3_mult((vec3){x0, y1, z}, transform.scale);

    *p0 = cstrl_vec3_rotate_by_quat(*p0, transform.rotation);
    *p1 = cstrl_vec3_rotate_by_quat(*p1, transform.rotation);
    *p2 = cstrl_vec3_rotate_by_quat(*p2, transform.rotation);
    *p3 = cstrl_vec3_rotate_by_quat(*p3, transform.rotation);

    *p0 = cstrl_vec3_add(*p0, transform.position);
    *p1 = cstrl_vec3_add(*p1, transform.position);
    *p2 = cstrl_vec3_add(*p2, transform.position);
    *p3 = cstrl_vec3_add(*p3, transform.position);
}

#ifdef COLLISION_DEBUG
static void fill_physics_positions(da_float *positions)
{
    for (int i = 0; i < g_aabb_tree.node_count; i++)
    {
        vec3 aabb[2];
        aabb[0] = g_aabb_tree.nodes[i].aabb[0];
        aabb[1] = g_aabb_tree.nodes[i].aabb[1];

        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[0].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[0].x);
        cstrl_da_float_push_back(positions, aabb[1].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
        cstrl_da_float_push_back(positions, aabb[1].x);
        cstrl_da_float_push_back(positions, aabb[0].y);
        cstrl_da_float_push_back(positions, aabb[1].z);
    }
}
#endif

void gameplay_scene_init(cstrl_platform_state *platform_state)
{
    g_default_shader =
        cstrl_load_shaders_from_files("resources/shaders/cozy/default3D.vert", "resources/shaders/cozy/default3D.frag");

    g_projectile_shader = cstrl_load_shaders_from_files("resources/shaders/cozy/default3D_no_texture.vert",
                                                        "resources/shaders/cozy/default3D_no_texture.frag");
#ifdef COLLISION_DEBUG
    g_collision_shader =
        cstrl_load_shaders_from_files("resources/shaders/cozy/line3D.vert", "resources/shaders/cozy/line3D.frag");
#endif
    cstrl_platform_set_key_callback(platform_state, gameplay_scene_key_callback);
    cstrl_platform_set_mouse_button_callback(platform_state, gameplay_scene_mouse_button_callback);
    cstrl_platform_set_mouse_position_callback(platform_state, gameplay_scene_mouse_position_callback);
    cstrl_platform_get_window_size(platform_state, &g_screen_width, &g_screen_height);
    g_main_camera = cstrl_camera_create(g_screen_width, g_screen_height, false);
    g_main_camera->up.y = -1.0f;
    g_main_camera->position.z = 2.0f;
    g_main_camera->forward.y = 0.5f;
    g_main_camera->forward.z = -1.0f;
    g_main_camera->forward = cstrl_vec3_normalize(g_main_camera->forward);
    cstrl_camera_update(g_main_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);

    g_guy_render_data = cstrl_renderer_create_render_data();
    g_guy_positions = malloc(sizeof(float) * (TOTAL_GUY_COUNT + 1) * 36);
    if (g_guy_positions)
    {
        memset(g_guy_positions, 0, sizeof(float) * (TOTAL_GUY_COUNT + 1) * 36);
    }
    else
    {
        printf("Failed to malloc g_guy_positions\n");
    }
    g_guy_uvs = malloc(sizeof(float) * (TOTAL_GUY_COUNT + 1) * 24);
    if (g_guy_uvs)
    {
        memset(g_guy_uvs, 0, sizeof(float) * (TOTAL_GUY_COUNT + 1) * 24);
    }
    else
    {
        printf("Failed to malloc g_guy_uvs\n");
    }
    g_guy_colors = malloc(sizeof(float) * (TOTAL_GUY_COUNT + 1) * 48);
    if (g_guy_colors)
    {
        memset(g_guy_colors, 0, sizeof(float) * (TOTAL_GUY_COUNT + 1) * 48);
    }
    else
    {
        printf("Failed to malloc g_guy_colors\n");
    }
    g_guy_indices = malloc(sizeof(int) * (TOTAL_GUY_COUNT * 2 + 1) * 6);
    if (g_guy_indices)
    {
        memset(g_guy_colors, 0, sizeof(int) * (TOTAL_GUY_COUNT * 2 + 1) * 6);
    }
    else
    {
        printf("Failed to malloc g_guy_indices\n");
    }
    cstrl_renderer_add_positions(g_guy_render_data, g_guy_positions, 3, (TOTAL_GUY_COUNT * 2 + 1) * 6);
    cstrl_renderer_add_uvs(g_guy_render_data, g_guy_uvs);
    cstrl_renderer_add_colors(g_guy_render_data, g_guy_colors);
    cstrl_renderer_add_indices(g_guy_render_data, g_guy_indices, (TOTAL_GUY_COUNT * 2 + 1) * 6);
    g_guy_texture =
        cstrl_texture_generate_from_path("resources/textures/cozy_game/guys.png", CSTRL_TEXTURE_FILTER_NEAREST);

    g_projectile_render_data = cstrl_renderer_create_render_data();
    float projectile_positions[12] = {0};
    float projectile_colors[16] = {0};
    int projectile_indices[6] = {0};
    cstrl_renderer_add_positions(g_projectile_render_data, projectile_positions, 3, 4);
    cstrl_renderer_add_colors(g_projectile_render_data, projectile_colors);
    cstrl_renderer_add_indices(g_projectile_render_data, projectile_indices, 6);

    g_building_mesh = cstrl_model_generate_mesh_from_obj_file("resources/models/building.obj");
    vec3 building_aabb[2];
    for (int i = 0; i < g_building_mesh.vertex_count; i++)
    {
        g_building_mesh.positions[i * 3] += 25.0f;
        g_building_mesh.positions[i * 3] *= 0.04f;
        // g_building_mesh.positions[i * 3 + 1] -= 72.5f;
        g_building_mesh.positions[i * 3 + 1] *= -0.04f;
        g_building_mesh.positions[i * 3 + 2] -= 25.0f;
        g_building_mesh.positions[i * 3 + 2] *= 0.04f;
        float x = g_building_mesh.positions[i * 3];
        float y = g_building_mesh.positions[i * 3 + 1];
        float z = g_building_mesh.positions[i * 3 + 2];
        if (i == 0 || building_aabb[0].x > x)
        {
            building_aabb[0].x = x;
        }
        if (i == 0 || building_aabb[0].y > y)
        {
            building_aabb[0].y = y;
        }
        if (i == 0 || building_aabb[0].z > z)
        {
            building_aabb[0].z = z;
        }
        if (i == 0 || building_aabb[1].x < x)
        {
            building_aabb[1].x = x;
        }
        if (i == 0 || building_aabb[1].y < y)
        {
            building_aabb[1].y = y;
        }
        if (i == 0 || building_aabb[1].z < z)
        {
            building_aabb[1].z = z;
        }
    }
    cstrl_collision_aabb_tree_insert(&g_aabb_tree, NULL, building_aabb);

    g_building_render_data = cstrl_renderer_create_render_data();
    cstrl_renderer_add_positions(g_building_render_data, g_building_mesh.positions, 3, g_building_mesh.vertex_count);
    cstrl_renderer_add_uvs(g_building_render_data, g_building_mesh.uvs);
    cstrl_renderer_add_colors(g_building_render_data, g_building_mesh.colors);
    cstrl_renderer_add_normals(g_building_render_data, g_building_mesh.normals);

    g_car_mesh = cstrl_model_generate_mesh_from_obj_file("resources/models/car0.obj");
    vec3 car_aabb[2];
    for (int i = 0; i < g_car_mesh.vertex_count; i++)
    {
        g_car_mesh.positions[i * 3] -= 18.5f;
        g_car_mesh.positions[i * 3] *= 0.075f;
        g_car_mesh.positions[i * 3 + 1] *= -0.075f;
        g_car_mesh.positions[i * 3 + 2] -= 20.0f;
        g_car_mesh.positions[i * 3 + 2] *= 0.075f;
        float x = g_car_mesh.positions[i * 3];
        float y = g_car_mesh.positions[i * 3 + 1];
        float z = g_car_mesh.positions[i * 3 + 2];
        if (i == 0 || car_aabb[0].x > x)
        {
            car_aabb[0].x = x;
        }
        if (i == 0 || car_aabb[0].y > y)
        {
            car_aabb[0].y = y;
        }
        if (i == 0 || car_aabb[0].z > z)
        {
            car_aabb[0].z = z;
        }
        if (i == 0 || car_aabb[1].x < x)
        {
            car_aabb[1].x = x;
        }
        if (i == 0 || car_aabb[1].y < y)
        {
            car_aabb[1].y = y;
        }
        if (i == 0 || car_aabb[1].z < z)
        {
            car_aabb[1].z = z;
        }
    }
    cstrl_collision_aabb_tree_insert(&g_aabb_tree, NULL, car_aabb);

    g_car_render_data = cstrl_renderer_create_render_data();
    cstrl_renderer_add_positions(g_car_render_data, g_car_mesh.positions, 3, g_car_mesh.vertex_count);
    cstrl_renderer_add_uvs(g_car_render_data, g_car_mesh.uvs);
    cstrl_renderer_add_colors(g_car_render_data, g_car_mesh.colors);
    cstrl_renderer_add_normals(g_car_render_data, g_car_mesh.normals);
#ifdef COLLISION_DEBUG
    g_collision_render_data = cstrl_renderer_create_render_data();
    float *positions = malloc(sizeof(float) * (TOTAL_GUY_COUNT + 1) * 108);
    if (positions)
    {
        memset(positions, 0, sizeof(float) * (TOTAL_GUY_COUNT + 1) * 108);
    }
    else
    {
        printf("Failed to malloc g_collision_positions\n");
    }
    cstrl_renderer_add_positions(g_collision_render_data, positions, 3, (TOTAL_GUY_COUNT + 1) * 36);
#endif

    g_ground_render_data = cstrl_renderer_create_render_data();
    float ground_positions[12];
    vec3 point0, point1, point2, point3;
    get_points(&point0, &point1, &point2, &point3,
               (transform_t){(vec3){0}, cstrl_quat_identity(), (vec3){7.11, 4.0f, 0.0f}});
    ground_positions[0] = point0.x;
    ground_positions[1] = point0.z;
    ground_positions[2] = point0.y;
    ground_positions[3] = point1.x;
    ground_positions[4] = point1.z;
    ground_positions[5] = point1.y;
    ground_positions[6] = point2.x;
    ground_positions[7] = point2.z;
    ground_positions[8] = point2.y;
    ground_positions[9] = point3.x;
    ground_positions[10] = point3.z;
    ground_positions[11] = point3.y;
    float ground_uvs[8];
    ground_uvs[0] = 0.0f;
    ground_uvs[1] = 0.0f;
    ground_uvs[2] = 1.0f;
    ground_uvs[3] = 0.0f;
    ground_uvs[4] = 1.0f;
    ground_uvs[5] = 1.0f;
    ground_uvs[6] = 0.0f;
    ground_uvs[7] = 1.0f;
    float ground_colors[16];
    for (int i = 0; i < 16; i++)
    {
        ground_colors[i] = 1.0f;
    }
    int ground_indices[6] = {0, 1, 2, 0, 2, 3};

    cstrl_renderer_add_positions(g_ground_render_data, ground_positions, 3, 4);
    cstrl_renderer_add_uvs(g_ground_render_data, ground_uvs);
    cstrl_renderer_add_colors(g_ground_render_data, ground_colors);
    cstrl_renderer_add_indices(g_ground_render_data, ground_indices, 6);

    g_ground_texture =
        cstrl_texture_generate_from_path("resources/textures/cozy_game/city_base.png", CSTRL_TEXTURE_FILTER_NEAREST);

    vec3 base_position = cstrl_vec3_add(g_main_camera->position, g_main_camera->forward);
    guys_init(&g_guys);
    for (int i = 0; i < TOTAL_GUY_COUNT; i++)
    {
        vec3 random_color =
            (vec3){cozy_random_float(0.0f, 1.0f), cozy_random_float(0.0f, 1.0f), cozy_random_float(0.0f, 1.0f)};
        guys_add(&g_guys, &g_aabb_tree,
                 (vec3){cozy_random_float(-1.0f, 1.0f), -GUY_SIZE / 2.0f, cozy_random_float(-1.0f, 1.0f)},
                 random_color);
    }
    projectiles_init(&g_projectiles);

    g_paused = true;

    g_hero = (hero_t){0};
    g_hero.position = (vec3){0.0f, -GUY_SIZE / 2.0f, 0.0f};
    g_hero.speed = GUY_SCALE;
    quat camera_rotation = cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(g_main_camera->view));
    quat billboard_rotation = cstrl_quat_inverse(camera_rotation);
    get_points(&point0, &point1, &point2, &point3,
               (transform_t){g_hero.position, billboard_rotation, (vec3){GUY_SIZE, GUY_SIZE, GUY_SIZE}});
    g_hero_prev_point0 = point0;
    g_hero_prev_point1 = point1;
    g_hero_prev_point2 = point2;
    g_hero_prev_point3 = point3;

    int *id = malloc(sizeof(int));
    if (!id)
    {
        printf("Failed to malloc id\n");
        return;
    }
    *id = -1;
    vec3 aabb[2];
    aabb[0] = (vec3){g_hero.position.x - GUY_SIZE / 2.0f, g_hero.position.y - GUY_SIZE / 2.0f,
                     g_hero.position.z - GUY_SIZE / 2.0f};
    aabb[1] = (vec3){g_hero.position.x + GUY_SIZE / 2.0f, g_hero.position.y + GUY_SIZE / 2.0f,
                     g_hero.position.z + GUY_SIZE / 2.0f};
    g_hero.collision_index = cstrl_collision_aabb_tree_insert(&g_aabb_tree, id, aabb);
}

void gameplay_scene_update(cstrl_platform_state *platform_state)
{
    if (!g_paused)
    {
        guys_update(&g_guys, &g_aabb_tree);
        hero_update(&g_hero, &g_aabb_tree, g_hero_movement, g_main_camera);
        g_main_camera->position.x = g_hero.position.x;
        g_main_camera->position.y = g_hero.position.y - 1.0f;
        g_main_camera->position.z = g_hero.position.z + 2.0f;
        cstrl_camera_update(g_main_camera, CSTRL_CAMERA_DIRECTION_NONE, CSTRL_CAMERA_DIRECTION_NONE);
        projectiles_update(&g_projectiles, &g_aabb_tree, &g_guys);
    }
}

void gameplay_scene_render(cstrl_platform_state *platform_state)
{
    quat camera_rotation = cstrl_mat3_orthogonal_to_quat(cstrl_mat4_upper_left(g_main_camera->view));
    quat billboard_rotation = cstrl_quat_inverse(camera_rotation);
    for (int i = 0; i < g_guys.count; i++)
    {
        vec3 point0 = {0};
        vec3 point1 = {0};
        vec3 point2 = {0};
        vec3 point3 = {0};
        if (g_guys.active[i])
        {
            transform_t transform = {g_guys.position[i], billboard_rotation, (vec3){GUY_SIZE, GUY_SIZE, GUY_SIZE}};
            get_points(&point0, &point1, &point2, &point3, transform);
            if (g_guys.velocity[i].x < 0.0f)
            {
                float tmp = point0.x;
                point0.x = point1.x;
                point1.x = tmp;
                tmp = point2.x;
                point2.x = point3.x;
                point3.x = tmp;
            }
        }

        g_guy_positions[i * 24] = point0.x;
        g_guy_positions[i * 24 + 1] = point0.y;
        g_guy_positions[i * 24 + 2] = point0.z;
        g_guy_positions[i * 24 + 3] = point1.x;
        g_guy_positions[i * 24 + 4] = point1.y;
        g_guy_positions[i * 24 + 5] = point1.z;
        g_guy_positions[i * 24 + 6] = point2.x;
        g_guy_positions[i * 24 + 7] = point2.y;
        g_guy_positions[i * 24 + 8] = point2.z;
        g_guy_positions[i * 24 + 9] = point3.x;
        g_guy_positions[i * 24 + 10] = point3.y;
        g_guy_positions[i * 24 + 11] = point3.z;

        if (g_guys.animate[i])
        {
            point0 = (vec3){0};
            point1 = (vec3){0};
            point2 = (vec3){0};
            point3 = (vec3){0};
        }
        g_guy_positions[i * 24 + 12] = point0.x;
        g_guy_positions[i * 24 + 13] = point0.y;
        g_guy_positions[i * 24 + 14] = point0.z;
        g_guy_positions[i * 24 + 15] = point1.x;
        g_guy_positions[i * 24 + 16] = point1.y;
        g_guy_positions[i * 24 + 17] = point1.z;
        g_guy_positions[i * 24 + 18] = point2.x;
        g_guy_positions[i * 24 + 19] = point2.y;
        g_guy_positions[i * 24 + 20] = point2.z;
        g_guy_positions[i * 24 + 21] = point3.x;
        g_guy_positions[i * 24 + 22] = point3.y;
        g_guy_positions[i * 24 + 23] = point3.z;

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
                u0 = 0.0f;
                v0 = (float)g_guys.type[i] / GUY_TOTAL_ROWS;
                u1 = 1.0f / GUY_TOTAL_COLS;
                v1 = (float)(g_guys.type[i] + 1) / GUY_TOTAL_ROWS;
            }
            else
            {
                u0 = (float)g_guys.animation_frame[i] / GUY_TOTAL_COLS;
                v0 = (float)(GUY_TOTAL_TYPES + g_guys.type[i] % 2) / (float)GUY_TOTAL_ROWS;
                u1 = (float)(g_guys.animation_frame[i] + 1) / GUY_TOTAL_COLS;
                v1 = (float)(GUY_TOTAL_TYPES + g_guys.type[i] % 2 + 1) / (float)GUY_TOTAL_ROWS;
            }
        }
        g_guy_uvs[i * 16] = u0;
        g_guy_uvs[i * 16 + 1] = v0;
        g_guy_uvs[i * 16 + 2] = u1;
        g_guy_uvs[i * 16 + 3] = v0;
        g_guy_uvs[i * 16 + 4] = u1;
        g_guy_uvs[i * 16 + 5] = v1;
        g_guy_uvs[i * 16 + 6] = u0;
        g_guy_uvs[i * 16 + 7] = v1;

        if (g_guys.active[i])
        {
            u0 = 1.0f / GUY_TOTAL_COLS;
            v0 = (float)(g_guys.type[i] % 2) / GUY_TOTAL_ROWS;
            u1 = 2.0f / GUY_TOTAL_COLS;
            v1 = (float)(g_guys.type[i] % 2 + 1) / GUY_TOTAL_ROWS;
        }
        g_guy_uvs[i * 16 + 8] = u0;
        g_guy_uvs[i * 16 + 9] = v0;
        g_guy_uvs[i * 16 + 10] = u1;
        g_guy_uvs[i * 16 + 11] = v0;
        g_guy_uvs[i * 16 + 12] = u1;
        g_guy_uvs[i * 16 + 13] = v1;
        g_guy_uvs[i * 16 + 14] = u0;
        g_guy_uvs[i * 16 + 15] = v1;

        for (int j = 0; j < 4; j++)
        {
            g_guy_colors[i * 32 + j * 4] = 1.0f;
            g_guy_colors[i * 32 + j * 4 + 1] = 1.0f;
            g_guy_colors[i * 32 + j * 4 + 2] = 1.0f;
            g_guy_colors[i * 32 + j * 4 + 3] = 1.0f;
        }
        for (int j = 4; j < 8; j++)
        {
            if (!g_guys.animate[i])
            {
                g_guy_colors[i * 32 + j * 4] = g_guys.color[i].x;
                g_guy_colors[i * 32 + j * 4 + 1] = g_guys.color[i].y;
                g_guy_colors[i * 32 + j * 4 + 2] = g_guys.color[i].z;
                g_guy_colors[i * 32 + j * 4 + 3] = 1.0f;
            }
            else
            {
                g_guy_colors[i * 32 + j * 4] = 1.0f;
                g_guy_colors[i * 32 + j * 4 + 1] = 1.0f;
                g_guy_colors[i * 32 + j * 4 + 2] = 1.0f;
                g_guy_colors[i * 32 + j * 4 + 3] = 1.0f;
            }
        }

        g_guy_indices[i * 12] = i * 8;
        g_guy_indices[i * 12 + 1] = i * 8 + 1;
        g_guy_indices[i * 12 + 2] = i * 8 + 2;
        g_guy_indices[i * 12 + 3] = i * 8;
        g_guy_indices[i * 12 + 4] = i * 8 + 2;
        g_guy_indices[i * 12 + 5] = i * 8 + 3;
        g_guy_indices[i * 12 + 6] = i * 8 + 4;
        g_guy_indices[i * 12 + 7] = i * 8 + 5;
        g_guy_indices[i * 12 + 8] = i * 8 + 6;
        g_guy_indices[i * 12 + 9] = i * 8 + 4;
        g_guy_indices[i * 12 + 10] = i * 8 + 6;
        g_guy_indices[i * 12 + 11] = i * 8 + 7;
    }
    vec3 point0, point1, point2, point3;
    transform_t transform = {g_hero.position, billboard_rotation, (vec3){GUY_SIZE, GUY_SIZE, GUY_SIZE}};
    get_points(&point0, &point1, &point2, &point3, transform);
    float new_x0 = point0.x;
    float new_x1 = point1.x;
    float new_x2 = point2.x;
    float new_x3 = point3.x;
    point0.x = g_hero_prev_point0.x;
    point1.x = g_hero_prev_point1.x;
    point2.x = g_hero_prev_point2.x;
    point3.x = g_hero_prev_point3.x;
    if (g_hero.velocity.x < 0.0f)
    {
        point0.x = new_x1;
        point1.x = new_x0;
        point2.x = new_x3;
        point3.x = new_x2;
    }
    else if (g_hero.velocity.x > 0.0f)
    {
        point0.x = new_x0;
        point1.x = new_x1;
        point2.x = new_x2;
        point3.x = new_x3;
    }
    g_hero_prev_point0 = point0;
    g_hero_prev_point1 = point1;
    g_hero_prev_point2 = point2;
    g_hero_prev_point3 = point3;

    g_guy_positions[TOTAL_GUY_COUNT * 24] = point0.x;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 1] = point0.y;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 2] = point0.z;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 3] = point1.x;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 4] = point1.y;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 5] = point1.z;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 6] = point2.x;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 7] = point2.y;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 8] = point2.z;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 9] = point3.x;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 10] = point3.y;
    g_guy_positions[TOTAL_GUY_COUNT * 24 + 11] = point3.z;

    float u0 = (float)HERO_COL / (float)GUY_TOTAL_COLS;
    float v0 = (float)HERO_ROW / (float)GUY_TOTAL_ROWS;
    float u1 = (float)(HERO_COL + 1) / (float)GUY_TOTAL_COLS;
    float v1 = (float)(HERO_ROW + 1) / (float)GUY_TOTAL_ROWS;
    g_guy_uvs[TOTAL_GUY_COUNT * 16] = u0;
    g_guy_uvs[TOTAL_GUY_COUNT * 16 + 1] = v0;
    g_guy_uvs[TOTAL_GUY_COUNT * 16 + 2] = u1;
    g_guy_uvs[TOTAL_GUY_COUNT * 16 + 3] = v0;
    g_guy_uvs[TOTAL_GUY_COUNT * 16 + 4] = u1;
    g_guy_uvs[TOTAL_GUY_COUNT * 16 + 5] = v1;
    g_guy_uvs[TOTAL_GUY_COUNT * 16 + 6] = u0;
    g_guy_uvs[TOTAL_GUY_COUNT * 16 + 7] = v1;

    for (int i = 0; i < 4; i++)
    {
        g_guy_colors[TOTAL_GUY_COUNT * 32 + i * 4] = 1.0f;
        g_guy_colors[TOTAL_GUY_COUNT * 32 + i * 4 + 1] = 1.0f;
        g_guy_colors[TOTAL_GUY_COUNT * 32 + i * 4 + 2] = 1.0f;
        g_guy_colors[TOTAL_GUY_COUNT * 32 + i * 4 + 3] = 1.0f;
    }

    g_guy_indices[TOTAL_GUY_COUNT * 12] = TOTAL_GUY_COUNT * 8;
    g_guy_indices[TOTAL_GUY_COUNT * 12 + 1] = TOTAL_GUY_COUNT * 8 + 1;
    g_guy_indices[TOTAL_GUY_COUNT * 12 + 2] = TOTAL_GUY_COUNT * 8 + 2;
    g_guy_indices[TOTAL_GUY_COUNT * 12 + 3] = TOTAL_GUY_COUNT * 8;
    g_guy_indices[TOTAL_GUY_COUNT * 12 + 4] = TOTAL_GUY_COUNT * 8 + 2;
    g_guy_indices[TOTAL_GUY_COUNT * 12 + 5] = TOTAL_GUY_COUNT * 8 + 3;

    cstrl_renderer_modify_render_attributes(g_guy_render_data, g_guy_positions, g_guy_uvs, g_guy_colors,
                                            (TOTAL_GUY_COUNT * 2 + 1) * 4);
    cstrl_renderer_modify_indices(g_guy_render_data, g_guy_indices, 0, (TOTAL_GUY_COUNT * 2 + 1) * 6);
    float projectile_positions[g_projectiles.count * 12];
    float projectile_colors[g_projectiles.count * 24];
    int projectile_indices[g_projectiles.count * 6];
    for (int i = 0; i < g_projectiles.count; i++)
    {
        transform = (transform_t){g_projectiles.position[i], billboard_rotation,
                                  (vec3){PROJECTILE_SIZE, PROJECTILE_SIZE, PROJECTILE_SIZE}};
        point0 = (vec3){0};
        point1 = (vec3){0};
        point2 = (vec3){0};
        point3 = (vec3){0};
        if (g_projectiles.active[i])
        {
            get_points(&point0, &point1, &point2, &point3, transform);
        }
        projectile_positions[i * 12] = point0.x;
        projectile_positions[i * 12 + 1] = point0.y;
        projectile_positions[i * 12 + 2] = point0.z;
        projectile_positions[i * 12 + 3] = point1.x;
        projectile_positions[i * 12 + 4] = point1.y;
        projectile_positions[i * 12 + 5] = point1.z;
        projectile_positions[i * 12 + 6] = point2.x;
        projectile_positions[i * 12 + 7] = point2.y;
        projectile_positions[i * 12 + 8] = point2.z;
        projectile_positions[i * 12 + 9] = point3.x;
        projectile_positions[i * 12 + 10] = point3.y;
        projectile_positions[i * 12 + 11] = point3.z;

        for (int j = 0; j < 4; j++)
        {
            projectile_colors[i * 16 + j * 4] = g_projectiles.color[i].x;
            projectile_colors[i * 16 + j * 4 + 1] = g_projectiles.color[i].y;
            projectile_colors[i * 16 + j * 4 + 2] = g_projectiles.color[i].z;
            projectile_colors[i * 16 + j * 4 + 3] = 1.0f;
        }

        projectile_indices[i * 6] = i * 4;
        projectile_indices[i * 6 + 1] = i * 4 + 1;
        projectile_indices[i * 6 + 2] = i * 4 + 2;
        projectile_indices[i * 6 + 3] = i * 4;
        projectile_indices[i * 6 + 4] = i * 4 + 2;
        projectile_indices[i * 6 + 5] = i * 4 + 3;
    }
    if (g_projectiles.count > 0)
    {
        cstrl_renderer_modify_render_attributes(g_projectile_render_data, projectile_positions, NULL, projectile_colors,
                                                g_projectiles.count * 4);
        cstrl_renderer_modify_indices(g_projectile_render_data, projectile_indices, 0, g_projectiles.count * 6);
    }

    cstrl_set_uniform_mat4(g_default_shader.program, "view", g_main_camera->view);
    cstrl_set_uniform_mat4(g_default_shader.program, "projection", g_main_camera->projection);
    cstrl_set_uniform_mat4(g_projectile_shader.program, "view", g_main_camera->view);
    cstrl_set_uniform_mat4(g_projectile_shader.program, "projection", g_main_camera->projection);
    cstrl_use_shader(g_default_shader);

    cstrl_texture_bind(g_ground_texture);
    cstrl_renderer_draw_indices(g_ground_render_data);

    cstrl_texture_bind(g_guy_texture);
    cstrl_renderer_draw_indices(g_guy_render_data);

    cstrl_use_shader(g_projectile_shader);
    cstrl_renderer_draw_indices(g_projectile_render_data);

#ifdef COLLISION_DEBUG
    da_float positions;
    cstrl_da_float_init(&positions, (TOTAL_GUY_COUNT + 1) * 108);
    fill_physics_positions(&positions);
    cstrl_renderer_modify_positions(g_collision_render_data, positions.array, 0, positions.size);
    cstrl_set_uniform_mat4(g_collision_shader.program, "view", g_main_camera->view);
    cstrl_set_uniform_mat4(g_collision_shader.program, "projection", g_main_camera->projection);
    cstrl_set_uniform_4f(g_collision_shader.program, "color", 1.0f, 0.0f, 0.0f, 0.7f);
    cstrl_use_shader(g_collision_shader);
    cstrl_renderer_draw_lines(g_collision_render_data);
#endif

    cstrl_use_shader(g_default_shader);
    cstrl_texture_bind(g_building_mesh.textures);
    cstrl_renderer_draw(g_building_render_data);

    cstrl_texture_bind(g_car_mesh.textures);
    cstrl_renderer_draw(g_car_render_data);

    if (g_paused)
    {
        cozy_ui_render_gameplay(platform_state, &g_paused);
    }
}

void gameplay_scene_shutdown(cstrl_platform_state *platform_state)
{
    free(g_guy_positions);
    free(g_guy_uvs);
    free(g_guy_colors);
    cstrl_renderer_free_render_data(g_guy_render_data);
    cstrl_renderer_free_render_data(g_projectile_render_data);
}
