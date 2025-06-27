#include "challenge.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct projectile_t
{
    vec3 position;
    vec3 velocity;
} projectile_t;

typedef struct environment_t
{
    vec3 gravity;
    vec3 wind;
} environment_t;

void tick(environment_t *env, projectile_t *proj)
{
    vec3 position = cstrl_vec3_add(proj->position, proj->velocity);
    vec3 change = cstrl_vec3_add(env->gravity, env->wind);
    vec3 velocity = cstrl_vec3_add(proj->velocity, change);

    proj->position = position;
    proj->velocity = velocity;
}

int ray_tracer_challenge_chapter1()
{
    projectile_t projectile;
    projectile.position = (vec3){0.0f, 1.0f, 0.0f};
    projectile.velocity = cstrl_vec3_normalize((vec3){1.0f, 1.0f, 0.0f});

    environment_t environment;
    environment.gravity = (vec3){0.0f, -0.1f, 0.0f};
    environment.wind = (vec3){-0.01f, 0.0f, 0.0f};

    while (projectile.position.y > 0.0f)
    {
        printf("%f, %f, %f\n", projectile.position.x, projectile.position.y, projectile.position.z);
        tick(&environment, &projectile);
    }
    return 0;
}

typedef struct canvas_t
{
    int width;
    int height;
    float *pixel_colors;
} canvas_t;

void create_canvas(canvas_t *canvas, int width, int height)
{
    canvas->width = width;
    canvas->height = height;
    canvas->pixel_colors = malloc(sizeof(float) * canvas->width * canvas->height * 3);
    memset(canvas->pixel_colors, 0, sizeof(float) * canvas->width * canvas->height * 3);
}

void write_canvas(canvas_t *canvas, int x, int y, vec3 color)
{
    if (x >= canvas->width || x < 0 || y >= canvas->height || y < 0)
    {
        printf("write_canvas: invalid coords %d, %d\n", x, y);
        return;
    }
    canvas->pixel_colors[(y * canvas->width + x) * 3] = color.r;
    canvas->pixel_colors[(y * canvas->width + x) * 3 + 1] = color.g;
    canvas->pixel_colors[(y * canvas->width + x) * 3 + 2] = color.b;
}

vec3 pixel_at(canvas_t *canvas, int x, int y)
{
    if (x >= canvas->width || x < 0 || y >= canvas->height || y < 0)
    {
        printf("pixel_at: invalid coords %d, %d\n", x, y);
        return (vec3){0};
    }
    float r = canvas->pixel_colors[(y * canvas->width + x) * 3];
    float g = canvas->pixel_colors[(y * canvas->width + x) * 3 + 1];
    float b = canvas->pixel_colors[(y * canvas->width + x) * 3 + 2];
    return (vec3){r, g, b};
}

#define HEADER_SIZE 19

char *canvas_to_ppm(canvas_t *canvas)
{
    char header[HEADER_SIZE];
    int header_length = sprintf(header, "P3\n%d %d\n255\n", canvas->width, canvas->height);

    char *content = malloc(sizeof(char) * header_length + sizeof(char) * canvas->width * canvas->height * 3 * 4 + 1);
    if (content == NULL)
    {
        printf("Failed to malloc ppm content\n");
        return NULL;
    }
    memcpy(content, header, sizeof(char) * header_length);

    int total_characters = 0;
    for (int h = 0; h < canvas->height; h++)
    {
        int total_characters_row = 0;
        int total_lines = 0;
        for (int w = 0; w < canvas->width; w++)
        {
            const float r = roundf(255.0f * canvas->pixel_colors[(h * canvas->width + w) * 3]);
            const float g = roundf(255.0f * canvas->pixel_colors[(h * canvas->width + w) * 3 + 1]);
            const float b = roundf(255.0f * canvas->pixel_colors[(h * canvas->width + w) * 3 + 2]);
            char sep = ' ';
            if (total_characters_row / 65 > total_lines)
            {
                sep = '\n';
                total_lines++;
            }
            total_characters_row += sprintf(content + header_length + total_characters + total_characters_row, "%d%c",
                                            (int)cstrl_scalar_clamp(r, 0.0f, 255.0f), sep);
            sep = ' ';
            if (total_characters_row / 65 > total_lines)
            {
                sep = '\n';
                total_lines++;
            }
            total_characters_row += sprintf(content + header_length + total_characters + total_characters_row, "%d%c",
                                            (int)cstrl_scalar_clamp(g, 0.0f, 255.0f), sep);
            sep = ' ';
            if (total_characters_row / 65 > total_lines || w == canvas->width - 1)
            {
                sep = '\n';
                total_lines++;
            }
            total_characters_row += sprintf(content + header_length + total_characters + total_characters_row, "%d%c",
                                            (int)cstrl_scalar_clamp(b, 0.0f, 255.0f), sep);
        }
        total_characters += total_characters_row;
    }
    content[header_length + total_characters] = '\0';
    return content;
}

bool test_write_canvas()
{
    canvas_t canvas;
    create_canvas(&canvas, 10, 20);
    vec3 red = {1.0f, 0.0f, 0.0f};
    write_canvas(&canvas, 2, 3, red);
    vec3 result = pixel_at(&canvas, 2, 3);
    return cstrl_vec3_is_equal(red, pixel_at(&canvas, 2, 3));
}

bool test_write_canvas_to_ppm_header()
{
    canvas_t canvas;
    create_canvas(&canvas, 5, 3);
    char *buffer = canvas_to_ppm(&canvas);
    const char *expected = "P3\n5 3\n255\n";
    buffer[11] = '\0';
    bool result = strcmp(buffer, expected) == 0;
    free(buffer);
    return result;
}

bool test_write_canvas_to_ppm_content()
{
    canvas_t canvas;
    create_canvas(&canvas, 5, 3);
    vec3 c1 = {1.5f, 0.0f, 0.0f};
    vec3 c2 = {0.0f, 0.5f, 0.0f};
    vec3 c3 = {-0.5f, 0.0f, 1.0f};
    write_canvas(&canvas, 0, 0, c1);
    write_canvas(&canvas, 2, 1, c2);
    write_canvas(&canvas, 4, 2, c3);
    char *buffer = canvas_to_ppm(&canvas);
    const char *expected = "255 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n"
                           "0 0 0 0 0 0 0 128 0 0 0 0 0 0 0\n"
                           "0 0 0 0 0 0 0 0 0 0 0 0 0 0 255\n";
    bool result = strcmp(buffer + 11, expected) == 0;
    free(buffer);
    return result;
}

bool test_write_canvas_to_ppm_long_lines()
{
    canvas_t canvas;
    create_canvas(&canvas, 10, 2);
    vec3 c = {1.0f, 1.0f, 1.0f};
    for (int i = 0; i < 10; i++)
    {
        for (int j = 0; j < 2; j++)
        {
            write_canvas(&canvas, i, j, c);
        }
    }
    char *buffer = canvas_to_ppm(&canvas);
    const char *expected = "255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255\n"
                           "255 255 255 255 255 255 255 255 255 255 255 255\n"
                           "255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255 255\n"
                           "255 255 255 255 255 255 255 255 255 255 255 255\n";
    bool result = strcmp(buffer + 12, expected) == 0;
    free(buffer);
    return result;
}

bool test_write_canvas_to_ppm_end_is_newline()
{
    canvas_t canvas;
    create_canvas(&canvas, 5, 3);
    char *buffer = canvas_to_ppm(&canvas);
    int last_char_index = strlen(buffer) - 1;
    return buffer[last_char_index] == '\n';
}

int ray_tracer_challenge()
{
    if (test_write_canvas())
    {
        printf("Test Write to Canvas - Passed\n");
    }
    else
    {
        printf("Failed to write correctly to canvas\n");
        return 1;
    }
    if (test_write_canvas_to_ppm_header())
    {
        printf("Test Write Canvas Header to PPM - Passed\n");
    }
    else
    {
        printf("Failed to write header correctly to PPM\n");
    }
    if (test_write_canvas_to_ppm_content())
    {
        printf("Test Write Canvas Content to PPM - Passed\n");
    }
    else
    {
        printf("Failed to write content correctly to PPM\n");
    }
    if (test_write_canvas_to_ppm_long_lines())
    {
        printf("Test Write Canvas Long Lines to PPM - Passed\n");
    }
    else
    {
        printf("Failed to write content with long lines correctly to PPM\n");
    }
    if (test_write_canvas_to_ppm_end_is_newline())
    {
        printf("Test Write Canvas to PPM End is Newline Character - Passed\n");
    }
    else
    {
        printf("Failed to write content to PPM ending with new line\n");
    }

#if 0
    projectile_t projectile;
    projectile.position = (vec3){0.0f, 1.0f, 0.0f};
    projectile.velocity = cstrl_vec3_mult_scalar(cstrl_vec3_normalize((vec3){1.0f, 1.8f, 0.0f}), 11.25f);

    environment_t environment;
    environment.gravity = (vec3){0.0f, -0.1f, 0.0f};
    environment.wind = (vec3){-0.01f, 0.0f, 0.0f};

    canvas_t canvas;
    create_canvas(&canvas, 900, 550);
    while (projectile.position.y > 0.0f)
    {
        write_canvas(&canvas, (int)roundf(projectile.position.x),
                     (canvas.height - 1 - (int)roundf(projectile.position.y)), (vec3){1.0f, 0.0f, 0.0f});
        printf("%f, %f, %f\n", projectile.position.x, projectile.position.y, projectile.position.z);
        tick(&environment, &projectile);
    }
#endif
    canvas_t canvas;
    create_canvas(&canvas, 420, 420);
    vec4 point = {-190.0f, 0.0f, 0.0f, 1.0f};
    write_canvas(&canvas, (int)point.x + 210, (int)point.y + 210, (vec3){1.0f, 0.0f, 0.0f});

    for (int i = 0; i < 11; i++)
    {
        mat4 transform = cstrl_mat4_identity();
        transform = cstrl_mat4_rotate(transform, cstrl_pi / 6, (vec3){0.0f, 0.0f, 1.0f});
        point = cstrl_vec4_mult_mat4(point, transform);
        write_canvas(&canvas, (int)point.x + 210, (int)point.y + 210, (vec3){1.0f, 0.0f, 0.0f});
    }

    char *buffer = canvas_to_ppm(&canvas);
    if (buffer == NULL)
    {
        return 1;
    }
    cstrl_write_file("ray_tracer_challenge.ppm", buffer, strlen(buffer));
    return 0;
}
