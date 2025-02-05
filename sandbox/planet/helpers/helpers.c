#include "helpers.h"
#include "cstrl/cstrl_util.h"

bool hit_check(vec3 d, float *t, vec3 origin, vec3 center, float radius)
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

void generate_line_segments(da_float *positions, vec3 origin, vec3 destination)
{
    if (cstrl_vec3_length(cstrl_vec3_sub(origin, destination)) < 0.1f)
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
    generate_line_segments(positions, origin, mid_point);
    generate_line_segments(positions, mid_point, destination);
}

vec3 screen_ray_cast(vec2 screen_coords, vec2 screen_size, mat4 projection, mat4 view)
{
    float x = (2.0f * screen_coords.x) / screen_size.x - 1.0f;
    float y = 1.0f - (2.0f * screen_coords.y) / screen_size.y;

    vec4 ray_clip = {x, y, -1.0f, 1.0f};
    vec4 ray_eye = cstrl_vec4_mult_mat4(ray_clip, cstrl_mat4_inverse(projection));
    ray_eye = (vec4){ray_eye.x, ray_eye.y, -1.0f, 0.0f};

    vec4 ray_world = cstrl_vec4_mult_mat4(ray_eye, cstrl_mat4_transpose(cstrl_mat4_inverse(view)));

    return cstrl_vec3_normalize((vec3){ray_world.x, ray_world.y, ray_world.z});
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
