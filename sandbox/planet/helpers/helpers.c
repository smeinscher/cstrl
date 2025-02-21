#include "helpers.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_physics.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>

bool planet_hit_check(vec3 d, float *t, vec3 origin, vec3 center, float radius)
{
    vec3 l = cstrl_vec3_sub(origin, center);
    float b = cstrl_vec3_dot(d, l);
    float c = cstrl_vec3_dot(l, l) - powf(radius, 2.0f);

    if (powf(b, 2.0) - c >= 0.0f)
    {
        *t = -b - sqrtf(powf(b, 2.0f) - c);
        return true;
    }
    *t = 0.0f;
    return false;
}

void generate_line_segments(da_float *positions, vec3 origin, vec3 destination, float granularity)
{
    if (cstrl_vec3_length(cstrl_vec3_sub(origin, destination)) < granularity)
    {
        cstrl_da_float_push_back(positions, origin.x);
        cstrl_da_float_push_back(positions, origin.y);
        cstrl_da_float_push_back(positions, origin.z);
        cstrl_da_float_push_back(positions, destination.x);
        cstrl_da_float_push_back(positions, destination.y);
        cstrl_da_float_push_back(positions, destination.z);
        return;
    }
    float dot = cstrl_vec3_dot(cstrl_vec3_normalize(origin), cstrl_vec3_normalize(destination));
    float angle = acosf(dot) * 0.5f;
    vec3 axis = cstrl_vec3_cross(origin, destination);
    if (cstrl_vec3_is_equal(axis, (vec3){0.0f, 0.0f, 0.0f}))
    {
        return;
    }
    vec3 mid_point = cstrl_vec3_rotate_along_axis(origin, angle, axis);
    generate_line_segments(positions, origin, mid_point, granularity);
    generate_line_segments(positions, mid_point, destination, granularity);
}

vec3 screen_ray_cast(vec2 screen_coords, vec2 screen_size, mat4 projection, mat4 view)
{
    float x = (2.0f * screen_coords.x) / screen_size.x - 1.0f;
    float y = 1.0f - (2.0f * screen_coords.y) / screen_size.y;

    vec4 ray_clip = {x, y, -1.0f, 1.0f};
    vec4 ray_eye = cstrl_vec4_mult_mat4(ray_clip, cstrl_mat4_inverse(projection));
    ray_eye = (vec4){ray_eye.x, ray_eye.y, -1.0f, 0.0f};

    vec4 ray_world = cstrl_vec4_mult_mat4(ray_eye, cstrl_mat4_transpose(cstrl_mat4_inverse(view)));

    return cstrl_vec3_normalize(cstrl_vec4_to_vec3(ray_world));
}

vec2 world_to_screen(vec3 world_coords, vec2 screen_size, mat4 projection, mat4 view)
{
    vec4 world_view =
        cstrl_vec4_mult_mat4((vec4){world_coords.x, world_coords.y, world_coords.z, 1.0f}, cstrl_mat4_transpose(view));
    vec4 clip_space = cstrl_vec4_mult_mat4(world_view, cstrl_mat4_transpose(projection));
    vec3 ndc_space = cstrl_vec3_div_scalar(cstrl_vec4_to_vec3(clip_space), clip_space.w);
    vec2 window_space;
    window_space.x = (ndc_space.x + 1.0f) / 2.0f * screen_size.x;
    window_space.y = (1.0f - ndc_space.y) / 2.0f * screen_size.y;
    return window_space;
}

vec3 modify_point(vec3 point, transform transform)
{
    point = cstrl_vec3_mult(point, transform.scale);
    point = cstrl_vec3_rotate_by_quat(point, transform.rotation);
    point = cstrl_vec3_add(point, transform.position);

    return point;
}

void get_points(vec3 *p0, vec3 *p1, vec3 *p2, vec3 *p3, transform transform)
{
    float x0 = -0.5f;
    float x1 = 0.5f;
    float y0 = -0.5f;
    float y1 = 0.5f;
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

vec3 get_point_on_path(vec3 origin, vec3 start_position, vec3 end_position, float t)
{
    vec3 current = cstrl_vec3_normalize(cstrl_vec3_sub(start_position, origin));
    vec3 desired = cstrl_vec3_normalize(cstrl_vec3_sub(end_position, origin));
    float rotation_angle = acosf(cstrl_vec3_dot(current, desired));
    vec3 rotation_axis = cstrl_vec3_normalize(cstrl_vec3_cross(current, desired));
    if (cstrl_vec3_is_equal(rotation_axis, (vec3){0.0f, 0.0f, 0.0f}))
    {
        rotation_axis = (vec3){1.0f, 0.0f, 0.0f};
    }
    quat rotation = cstrl_quat_angle_axis(rotation_angle * t, rotation_axis);
    float length = cstrl_vec3_length(cstrl_vec3_sub(end_position, origin));
    vec3 position = cstrl_vec3_mult_scalar(cstrl_vec3_rotate_by_quat(current, rotation), length);
    position = cstrl_vec3_add(position, origin);

    return position;
}

float get_spherical_path_length(vec3 start_position, vec3 end_position)
{
    return acos(cstrl_vec3_dot(cstrl_vec3_normalize(start_position), cstrl_vec3_normalize(end_position)));
}

void generate_cube_positions(float *positions)
{
    positions[0] = -1.0f;
    positions[1] = 1.0f;
    positions[2] = -1.0f;
    positions[3] = -1.0f;
    positions[4] = -1.0f;
    positions[5] = -1.0f;
    positions[6] = 1.0f;
    positions[7] = -1.0f;
    positions[8] = -1.0f;
    positions[9] = 1.0f;
    positions[10] = -1.0f;
    positions[11] = -1.0f;
    positions[12] = 1.0f;
    positions[13] = 1.0f;
    positions[14] = -1.0f;
    positions[15] = -1.0f;
    positions[16] = 1.0f;
    positions[17] = -1.0f;
    positions[18] = -1.0f;
    positions[19] = -1.0f;
    positions[20] = 1.0f;
    positions[21] = -1.0f;
    positions[22] = -1.0f;
    positions[23] = -1.0f;
    positions[24] = -1.0f;
    positions[25] = 1.0f;
    positions[26] = -1.0f;
    positions[27] = -1.0f;
    positions[28] = 1.0f;
    positions[29] = -1.0f;
    positions[30] = -1.0f;
    positions[31] = 1.0f;
    positions[32] = 1.0f;
    positions[33] = -1.0f;
    positions[34] = -1.0f;
    positions[35] = 1.0f;
    positions[36] = 1.0f;
    positions[37] = -1.0f;
    positions[38] = -1.0f;
    positions[39] = 1.0f;
    positions[40] = -1.0f;
    positions[41] = 1.0f;
    positions[42] = 1.0f;
    positions[43] = 1.0f;
    positions[44] = 1.0f;
    positions[45] = 1.0f;
    positions[46] = 1.0f;
    positions[47] = 1.0f;
    positions[48] = 1.0f;
    positions[49] = 1.0f;
    positions[50] = -1.0f;
    positions[51] = 1.0f;
    positions[52] = -1.0f;
    positions[53] = -1.0f;
    positions[54] = -1.0f;
    positions[55] = -1.0f;
    positions[56] = 1.0f;
    positions[57] = -1.0f;
    positions[58] = 1.0f;
    positions[59] = 1.0f;
    positions[60] = 1.0f;
    positions[61] = 1.0f;
    positions[62] = 1.0f;
    positions[63] = 1.0f;
    positions[64] = 1.0f;
    positions[65] = 1.0f;
    positions[66] = 1.0f;
    positions[67] = -1.0f;
    positions[68] = 1.0f;
    positions[69] = -1.0f;
    positions[70] = -1.0f;
    positions[71] = 1.0f;
    positions[72] = -1.0f;
    positions[73] = 1.0f;
    positions[74] = -1.0f;
    positions[75] = 1.0f;
    positions[76] = 1.0f;
    positions[77] = -1.0f;
    positions[78] = 1.0f;
    positions[79] = 1.0f;
    positions[80] = 1.0f;
    positions[81] = 1.0f;
    positions[82] = 1.0f;
    positions[83] = 1.0f;
    positions[84] = -1.0f;
    positions[85] = 1.0f;
    positions[86] = 1.0f;
    positions[87] = -1.0f;
    positions[88] = 1.0f;
    positions[89] = -1.0f;
    positions[90] = -1.0f;
    positions[91] = -1.0f;
    positions[92] = -1.0f;
    positions[93] = -1.0f;
    positions[94] = -1.0f;
    positions[95] = 1.0f;
    positions[96] = 1.0f;
    positions[97] = -1.0f;
    positions[98] = -1.0f;
    positions[99] = 1.0f;
    positions[100] = -1.0f;
    positions[101] = -1.0f;
    positions[102] = -1.0f;
    positions[103] = -1.0f;
    positions[104] = 1.0f;
    positions[105] = 1.0f;
    positions[106] = -1.0f;
    positions[107] = 1.0f;
}
