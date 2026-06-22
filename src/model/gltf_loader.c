#include "cgltf/cgltf.h"
#include "cstrl/cstrl_model.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_util.h"
#include "log.c/log.h"
#include <stdlib.h>
#include <string.h>

typedef struct scene_internal_t
{
    cgltf_data *data;
} scene_internal_t;

static cgltf_data *cstrl_model_load_gltf_file(const char *path)
{
    cgltf_options options = {0};
    cgltf_data *data = NULL;
    cgltf_result result = cgltf_parse_file(&options, path, &data);
    if (result != cgltf_result_success)
    {
        log_error("Could not load file: %s", path);
        return NULL;
    }

    result = cgltf_load_buffers(&options, data, path);
    if (result != cgltf_result_success)
    {
        cgltf_free(data);
        log_error("Could not load buffers for file: %s", path);
        return NULL;
    }

    result = cgltf_validate(data);
    if (result != cgltf_result_success)
    {
        cgltf_free(data);
        log_error("Invalid file: %s", path);
        return NULL;
    }

    return data;
}

static void cstrl_model_free_gltf_data(cgltf_data *data)
{
    cgltf_free(data);
}

CSTRL_API scene_t cstrl_model_generate_scene_from_gltf_file(const char *path)
{
    scene_t scene = {0};
    scene.scene_internal = malloc(sizeof(scene_internal_t));
    scene_internal_t *scene_internal = scene.scene_internal;
    scene_internal->data = cstrl_model_load_gltf_file(path);
    if (scene_internal->data == NULL)
    {
        log_error("Failed to load scene");
        return (scene_t){0};
    }
    scene.render_datum = malloc(sizeof(cstrl_render_data));
    scene.render_datum[0] = cstrl_renderer_create_render_data();
    scene.render_data_count = 1;
    scene.name = scene_internal->data->scene->name;
    scene.model_count = scene_internal->data->nodes_count;
    scene.models = malloc(sizeof(model_t) * scene.model_count);

    int position_index = -1;
    int uv_index = -1;
    int joints_index = -1;
    int weights_index = -1;
    int indices_index = -1;
    for (int i = 0; i < scene_internal->data->meshes[0].primitives[0].attributes_count; i++)
    {
        if (strcmp(scene_internal->data->meshes[0].primitives[0].attributes[i].name, "POSITION") == 0)
        {
            position_index = i;
            continue;
        }
        if (strcmp(scene_internal->data->meshes[0].primitives[0].attributes[i].name, "TEXCOORD_0") == 0)
        {
            uv_index = i;
            continue;
        }
        if (strcmp(scene_internal->data->meshes[0].primitives[0].attributes[i].name, "JOINTS_0") == 0)
        {
            joints_index = i;
            continue;
        }
        if (strcmp(scene_internal->data->meshes[0].primitives[0].attributes[i].name, "WEIGHTS_0") == 0)
        {
            weights_index = i;
            continue;
        }
    }
    if (scene_internal->data->meshes[0].primitives[0].indices != NULL)
    {
        indices_index = scene_internal->data->meshes[0].primitives[0].attributes_count;
    }

    for (int i = 0; i < scene_internal->data->buffer_views_count; i++)
    {
        size_t offset = scene_internal->data->buffer_views[i].offset;
        size_t size = scene_internal->data->buffer_views[i].size;
        unsigned char *buffer = scene_internal->data->buffers[0].data;
        // unsigned char *buffer = (unsigned char *)scene_internal->data->buffer_views[i].buffer;
        if (i == position_index)
        {
            float *new_buffer = malloc(size);
            memcpy(new_buffer, buffer + offset, size);
            cstrl_renderer_add_positions(scene.render_datum[0], new_buffer, 3, size / 12, CSTRL_USAGE_DYNAMIC);
            da_float colors;
            cstrl_da_float_init(&colors, size);
            for (int j = 0; j < size / 12; j++)
            {
                cstrl_da_float_push_back(&colors, 1.0f);
                cstrl_da_float_push_back(&colors, 1.0f);
                cstrl_da_float_push_back(&colors, 1.0f);
                cstrl_da_float_push_back(&colors, 1.0f);
            }
            cstrl_renderer_add_colors(scene.render_datum[0], colors.array, CSTRL_USAGE_STATIC);
            cstrl_da_float_free(&colors);
            free(new_buffer);
        }
        else if (i == uv_index)
        {
            float *new_buffer = malloc(size);
            memcpy(new_buffer, buffer + offset, size);
            cstrl_renderer_add_uvs(scene.render_datum[0], new_buffer, CSTRL_USAGE_STATIC);
            free(new_buffer);
        }
        else if (i == joints_index)
        {
            int *new_buffer = malloc(size * sizeof(int));
            for (int j = 0; j < size; j++)
            {
                new_buffer[j] = buffer[j + offset];
            }
            cstrl_renderer_add_bone_ids(scene.render_datum[0], new_buffer, CSTRL_USAGE_STATIC);
            free(new_buffer);
        }
        else if (i == weights_index)
        {
            float *new_buffer = malloc(size);
            memcpy(new_buffer, buffer + offset, size);
            cstrl_renderer_add_weights(scene.render_datum[0], new_buffer, CSTRL_USAGE_STATIC);
            free(new_buffer);
        }
        else if (i == indices_index)
        {
            int *new_buffer = malloc(size / 2 * sizeof(int));
            short *short_buffer = (short *)buffer;
            for (int j = 0; j < size / 2; j++)
            {
                new_buffer[j] = short_buffer[j + offset / 2];
            }
            cstrl_renderer_add_indices(scene.render_datum[0], new_buffer, size / 2, CSTRL_USAGE_STATIC);
            free(new_buffer);
        }
        else if (i == 6)
        {
            memcpy(scene.tmp_bone_matrices, buffer + offset, size);
        }
    }
    return scene;
}

CSTRL_API void cstrl_model_scene_free(scene_t *scene)
{
    free(scene->models);
    scene->models = NULL;
    scene_internal_t *scene_internal = scene->scene_internal;
    cstrl_model_free_gltf_data(scene_internal->data);
    scene_internal->data = NULL;
    free(scene->scene_internal);
    scene->scene_internal = NULL;
    scene->model_count = 0;
}
