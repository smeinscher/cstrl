#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_util.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

typedef struct ray_t
{
    vec3 origin;
    vec3 direction;
} ray_t;

vec3 ray_at(ray_t ray, float t)
{
    return cstrl_vec3_add(ray.origin, cstrl_vec3_mult_scalar(ray.direction, t));
}

typedef struct hit_record_t
{
    vec3 point;
    vec3 normal;
    float t;
} hit_record_t;

typedef struct sphere
{
    vec3 center;
    float radius;
} sphere_t;

bool hit_sphere(const sphere_t sphere, const ray_t ray, float ray_tmin, float ray_tmax, hit_record_t *record)
{
    vec3 oc = cstrl_vec3_sub(sphere.center, ray.origin);
    float a = cstrl_vec3_length(ray.direction) * cstrl_vec3_length(ray.direction);
    float h = cstrl_vec3_dot(ray.direction, oc);
    float c = cstrl_vec3_length(oc) * cstrl_vec3_length(oc) - sphere.radius * sphere.radius;
    float discriminant = h * h - a * c;
    if (discriminant < 0.0f)
    {
        return false;
    }

    float sqrt_d = sqrtf(discriminant);
    float root = (h - sqrt_d) / a;
    if (root <= ray_tmin || ray_tmax <= root)
    {
        root = (h + sqrt_d) / a;
        if (root <= ray_tmin || ray_tmax <= root)
        {
            return false;
        }
    }

    record->t = root;
    record->point = ray_at(ray, record->t);
    record->normal = cstrl_vec3_div_scalar(cstrl_vec3_sub(record->point, sphere.center), sphere.radius);
    return true;
}

float hit_sphere_old(const vec3 center, float radius, ray_t ray)
{
    vec3 oc = cstrl_vec3_sub(center, ray.origin);
    float a = cstrl_vec3_length(ray.direction) * cstrl_vec3_length(ray.direction);
    float h = cstrl_vec3_dot(ray.direction, oc);
    float c = cstrl_vec3_length(oc) * cstrl_vec3_length(oc) - radius * radius;
    float discriminant = h * h - a * c;
    if (discriminant < 0.0f)
    {
        return -1.0f;
    }
    return (h - sqrtf(discriminant)) / a;
}

vec3 ray_color(ray_t ray)
{
    float t = hit_sphere((vec3){0.0f, 0.0f, -1.0f}, 0.5, ray);
    if (t > 0.0f)
    {
        vec3 n = cstrl_vec3_normalize(cstrl_vec3_sub(ray_at(ray, t), (vec3){0.0f, 0.0f, -1.0f}));
        return cstrl_vec3_mult_scalar((vec3){n.x + 1.0f, n.y + 1.0f, n.z + 1.0f}, 0.5f);
    }
    vec3 unit_direction = cstrl_vec3_normalize(ray.direction);
    float a = 0.5f * (unit_direction.y + 1.0f);
    vec3 a_start_value = cstrl_vec3_mult_scalar((vec3){1.0f, 1.0f, 1.0f}, 1.0f - a);
    vec3 a_end_value = cstrl_vec3_mult_scalar((vec3){0.5f, 0.7f, 1.0f}, a);

    return cstrl_vec3_add(a_start_value, a_end_value);
}

static void write_color(const char *file_name, const vec3 pixel_color)
{
    int rbyte = (int)(255.999f * pixel_color.x);
    int gbyte = (int)(255.999f * pixel_color.y);
    int bbyte = (int)(255.999f * pixel_color.z);

    char buffer[16];
    sprintf(buffer, "%d %d %d\n", rbyte, gbyte, bbyte);
    cstrl_append_file(file_name, buffer, strlen(buffer));
}

int ray_tracing_in_one_weekend()
{
    const char *file_name = "image.ppm";
    float aspect_ratio = 16.0f / 9.0f;
    int image_width = 400;
    int image_height = (int)((float)image_width / aspect_ratio);

    float focal_length = 1.0f;
    float viewport_height = 2.0f;
    float viewport_width = viewport_height * ((float)image_width / (float)image_height);
    vec3 camera_center = {0.0f, 0.0f, 0.0f};

    vec3 viewport_u = {viewport_width, 0.0f, 0.0f};
    vec3 viewport_v = {0.0f, -viewport_height, 0.0f};

    vec3 pixel_delta_u = cstrl_vec3_div_scalar(viewport_u, image_width);
    vec3 pixel_delta_v = cstrl_vec3_div_scalar(viewport_v, image_height);

    vec3 a = cstrl_vec3_sub(camera_center, (vec3){0.0f, 0.0f, focal_length});
    vec3 b = cstrl_vec3_add(cstrl_vec3_div_scalar(viewport_u, 2.0f), cstrl_vec3_div_scalar(viewport_v, 2.0f));
    vec3 viewport_upper_left = cstrl_vec3_sub(a, b);
    vec3 pixel00_loc =
        cstrl_vec3_add(viewport_upper_left, cstrl_vec3_mult_scalar(cstrl_vec3_add(pixel_delta_u, pixel_delta_v), 0.5f));
    char buffer[32];
    sprintf(buffer, "P3\n%d %d\n255\n", image_width, image_height);
    cstrl_write_file(file_name, buffer, strlen(buffer));

    for (int j = 0; j < image_height; j++)
    {
        for (int i = 0; i < image_width; i++)
        {
            vec3 pixel_delta =
                cstrl_vec3_add(cstrl_vec3_mult_scalar(pixel_delta_u, i), cstrl_vec3_mult_scalar(pixel_delta_v, j));
            vec3 pixel_center = cstrl_vec3_add(pixel00_loc, pixel_delta);
            vec3 ray_direction = cstrl_vec3_sub(pixel_center, camera_center);

            ray_t ray;
            ray.origin = camera_center;
            ray.direction = ray_direction;

            vec3 color = ray_color(ray);

            write_color(file_name, color);
        }
    }

    return 0;
}
