#include "gameplay.h"
#include "../entities/ball.h"
#include "../entities/cup.h"
#include "../entities/player.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>

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

bool g_simulation;

static int g_mouse_x = 0;
static int g_mouse_y = 0;

static cstrl_shader *g_default_shader;

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
static bool g_cleared_balls = false;
static bool g_reset_game_update = false;
static bool g_reset_game_render = false;
static bool g_overtime_init_update = false;
static bool g_overtime_init_render = false;
static bool g_mouse_in_shot_area = false;
static bool g_paused = false;
static bool g_transition = false;
static bool g_rerack = false;

static int g_tick_counter = 0;

void gameplay_key_callback(cstrl_platform_state *state, int key, int scancode, int action, int mods)
{
    if (key == CSTRL_KEY_ESCAPE)
    {
        cstrl_platform_set_should_exit(true);
    }
    else if (key == CSTRL_KEY_R && action == CSTRL_ACTION_PRESS)
    {
        if (g_players.first_turn && g_players.human[g_players.current_player_turn])
        {
            int reracks_left = g_players.current_player_turn == PLAYER1_TURN ? g_players.team1_reracks_remaining
                                                                             : g_players.team2_reracks_remaining;
            if (reracks_left > 0 && g_players.human[g_players.current_player_turn])
            {
                if (cups_rerack(&g_cups, g_players.current_player_turn == PLAYER1_TURN ? 0 : 1))
                {
                    g_rerack = true;
                    if (g_players.current_player_turn == PLAYER1_TURN)
                    {
                        g_players.team1_reracks_remaining--;
                    }
                    else
                    {
                        g_players.team2_reracks_remaining--;
                    }
                }
            }
        }
    }
}

void gameplay_mouse_position_callback(cstrl_platform_state *state, int xpos, int ypos)
{
    g_mouse_x = xpos / 4;
    g_mouse_y = ypos / 4;
}

void gameplay_mouse_button_callback(cstrl_platform_state *state, int button, int action, int mods)
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

bool gameplay_show_cursor()
{
    return !g_players.human[g_players.current_player_turn] || !g_mouse_in_shot_area ||
           g_players.current_turn_state >= STARTED_SHOT;
}

void gameplay_get_mouse_position(int *mouse_x, int *mouse_y)
{
    *mouse_x = g_mouse_x;
    *mouse_y = g_mouse_y;
}

void gameplay_init(int base_screen_x, int base_screen_y, cstrl_shader *default_shader, bool simulation)
{
    g_default_shader = default_shader;
    g_background_render_data = cstrl_renderer_create_render_data();
    float x0 = 0.0f;
    float y0 = 0.0f;
    float x1 = base_screen_x;
    float y1 = base_screen_y;
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

    players_init(&g_players, true, 0);
    balls_init(&g_balls);

    float player_size_x = 17.0f;
    float player_size_y = 31.5f;
    float playerx0 = 30.0f;
    float playery0 = (float)base_screen_y / 2.0f - player_size_y / 2.0f;
    float playerx1 = playerx0 + player_size_x;
    float playery1 = (float)base_screen_y / 2.0f + player_size_y / 2.0f;
    playerx0 = 290.0f;
    playerx1 = playerx0 - player_size_x;

    g_team1_start = (vec2){290.0f, playery0 + player_size_y / 2.0f};
    g_team2_start = (vec2){30.0f, playery0 + player_size_y / 2.0f};

    g_mouse_x = base_screen_x / 2;
    g_mouse_y = base_screen_y / 2;
}

static void shoot_ball(bool human, int team)
{
    if (!human)
    {
        int selected_cup;
        do
        {
            selected_cup = rand() % 10 + (team * 10);
        } while (cstrl_da_int_find_first(&g_cups.freed, selected_cup) != CSTRL_DA_INT_ITEM_NOT_FOUND);
        vec2 target_position = g_cups.position[selected_cup];
        float t = (float)g_players.stats[g_players.current_player_turn].focus / 100.0f;
        float scale = (1.0f - t) * 6.5f + t * 3.0f;
        vec2 target_error = cstrl_vec2_mult_scalar(
            (vec2){(float)(rand() % 1000 - 500) / 500.0f, (float)(rand() % 1000 - 500) / 500.0f}, scale);
        balls_shoot(&g_balls, target_position, team == 0 ? g_team1_start : g_team2_start, target_error,
                    INITIAL_BALL_SPEED, team);
    }
    else
    {
        balls_shoot(&g_balls, g_target_position, team == 0 ? g_team1_start : g_team2_start,
                    (vec2){0.0f, g_target_error * 12.5}, INITIAL_BALL_SPEED, team);
    }
}

static void eye_to_eye_stage_update()
{
    switch (g_players.current_turn_state)
    {
    case AIM_TARGET:
    case AIM_METER:
        break;
    case STARTED_SHOT:
        if (g_players.current_player_turn == PLAYER1_TURN)
        {
            shoot_ball(g_players.human[0], 0);
            shoot_ball(g_players.human[2], 1);
        }
        else
        {
            shoot_ball(g_players.human[1], 0);
            shoot_ball(g_players.human[3], 1);
        }
        players_advance_turn_state(&g_players);
        break;
    case SHOOTING:
        balls_update(&g_balls, &g_cups, &g_players);
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
        }
        break;
    case TURN_END:
        g_tick_counter++;
        if (g_tick_counter > 60)
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
            balls_clear(&g_balls);
            g_cleared_balls = false;
            g_tick_counter = 0;
        }
        break;
    }
}

static void main_game_stage_update()
{
    switch (g_players.current_turn_state)
    {
    case AIM_TARGET:
    case AIM_METER:
        break;
    case STARTED_SHOT:
        if (g_players.first_turn && !g_players.human[g_players.current_player_turn] &&
            (g_players.current_player_turn == PLAYER1_TURN || g_players.current_player_turn == PLAYER3_TURN))
        {
            int reracks_left = g_players.current_player_turn == PLAYER1_TURN ? g_players.team1_reracks_remaining
                                                                             : g_players.team2_reracks_remaining;
            if (reracks_left > 0 && cups_rerack(&g_cups, g_players.current_player_turn == PLAYER1_TURN ? 0 : 1))
            {
                if (g_players.current_player_turn == PLAYER1_TURN)
                {
                    g_players.team1_reracks_remaining--;
                }
                else
                {
                    g_players.team2_reracks_remaining--;
                }
            }
        }
        shoot_ball(g_players.human[g_players.current_player_turn],
                   g_players.current_player_turn < PLAYER3_TURN ? 0 : 1);
        players_advance_turn_state(&g_players);
        break;
    case SHOOTING:
        balls_update(&g_balls, &g_cups, &g_players);
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
            g_cleared_balls = false;
        }
        break;
    case TURN_END:
        g_tick_counter++;
        if (g_tick_counter > 60)
        {
            players_advance_turn_state(&g_players);
            balls_clear(&g_balls);
            g_cleared_balls = false;
            g_tick_counter = 0;
        }
        break;
    }
}

void gameplay_update()
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
            eye_to_eye_stage_update();
            break;
        case MAIN_GAME_STAGE:
        case REBUTTAL_ATTEMPT1_STAGE:
        case REBUTTAL_ATTEMPT2_STAGE:
        case OVERTIME_STAGE:
            main_game_stage_update();
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
        balls_clear(&g_balls);
        g_cleared_balls = false;

        g_overtime_init_update = false;
    }
    if (g_reset_game_update)
    {
        printf("Resetting Update...\n");
        cups_free(&g_cups);
        cups_init(&g_cups, false);
        players_reset(&g_players);
        g_cleared_target_and_meter = false;
        balls_clear(&g_balls);
        g_cleared_balls = false;
        g_tick_counter = 0.0f;

        g_reset_game_update = false;
    }
}

static void main_game_stage_render()
{
    if (!g_cleared_balls)
    {
        float ball_positions[12 * MAX_BALLS] = {0};
        cstrl_renderer_modify_positions(g_ball_render_data, ball_positions, 0, 12 * MAX_BALLS);
        g_cleared_balls = true;
    }
    if (g_players.human[g_players.current_player_turn])
    {
        if (g_players.current_turn_state == AIM_TARGET)
        {
            float target_positions[12] = {0};
            if (g_mouse_in_shot_area)
            {
                float t = (float)g_players.stats[g_players.current_player_turn].focus / 100.0f;
                float distance = (1.0f - t) * 4.0f + t * 3.0f;
                float speed = (1.0f - t) * 20.0f + t * 12.0f;
                float target_offset_x = (float)cos(cstrl_platform_get_absolute_time() * speed) * distance;
                float target_offset_y = (float)sin(cstrl_platform_get_absolute_time() * speed) * distance;
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
                float t = (float)g_players.stats[g_players.current_player_turn].focus / 100.0f;
                float speed = (1.0f - t) * 14.0f + t * 8.0f;
                g_target_error = (float)sin(cstrl_platform_get_absolute_time() * speed);
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
            float ball_positions[12] = {0};
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

    cstrl_use_shader(*g_default_shader);
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
}

void gameplay_render()
{
    main_game_stage_render();
    if (g_rerack || g_reset_game_render || g_overtime_init_render)
    {
        float cup_positions[240] = {0};
        for (int i = 0; i < 20; i++)
        {
            if (!g_cups.active[i] || (g_overtime_init_render && (i < 4 || (i > 9 && i < 14))))
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
    }
    if (g_overtime_init_render)
    {

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

void gameplay_shutdown()
{
    cstrl_renderer_free_render_data(g_background_render_data);
    cstrl_renderer_free_render_data(g_cup_render_data);
    cstrl_renderer_free_render_data(g_ball_render_data);
    cstrl_renderer_free_render_data(g_target_render_data);
    cstrl_renderer_free_render_data(g_meter_render_data);
    cstrl_renderer_free_render_data(g_meter_bar_render_data);
}
