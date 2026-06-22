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
    }
    return scene;
}

static scene_t old(const char *path)
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
    da_float positions;
    cstrl_da_float_init(&positions, 32);
    da_float colors;
    cstrl_da_float_init(&colors, 32);
    da_int indices;
    cstrl_da_int_init(&indices, 32);
    for (int i = 0; i < scene_internal->data->nodes_count; i++)
    {
        cgltf_node *node = &scene_internal->data->nodes[i];
        scene.models[i] = (model_t){0};
        if (node == NULL)
        {
            continue;
        }
        if (node->has_translation)
        {
            vec3 position = (vec3){node->translation[0], node->translation[1], node->translation[2]};
            scene.models[i].transform.position = position;
        }
        if (node->has_rotation)
        {
            quat rotation = (quat){node->rotation[0], node->rotation[1], node->rotation[2], node->rotation[3]};
            scene.models[i].transform.rotation = rotation;
        }
        if (node->has_scale)
        {
            vec3 scale = (vec3){node->scale[0], node->scale[1], node->scale[2]};
            scene.models[i].transform.scale = scale;
        }

        if (node->mesh == NULL)
        {
            continue;
        }
        for (int j = 0; j < node->mesh->primitives_count; j++)
        {
            cgltf_primitive primitive = node->mesh->primitives[j];
            cgltf_size indices_offset = primitive.indices->buffer_view->offset;
            cgltf_size indices_size = primitive.indices->count;
            cgltf_size indices_stride = primitive.indices->stride;
            for (int k = 0; k < indices_size; k++)
            {
                unsigned short *index =
                    &primitive.indices->buffer_view->buffer->data[indices_offset + k * indices_stride];
                // log_debug("Index: %hu", *index);
                cstrl_da_int_push_back(&indices, *index);
            }
            for (int k = 0; k < primitive.attributes_count; k++)
            {
                cgltf_attribute attribute = primitive.attributes[k];
                switch (attribute.type)
                {
                case cgltf_attribute_type_position: {
                    float *data = attribute.data->buffer_view->buffer->data;
                    for (int l = 0; l < attribute.data->count / 3; l++)
                    {
                        cgltf_size stride = attribute.data->buffer_view->stride * l;
                        cgltf_size offset = attribute.data->offset / sizeof(float);
                        // log_debug("Position: %f %f %f", data[l * 3 + stride + offset],
                        //           data[l * 3 + 1 + stride + offset], data[l * 3 + 2 + stride + offset]);
                        cstrl_da_float_push_back(&positions, data[l * 3 + stride + offset]);
                        cstrl_da_float_push_back(&positions, data[l * 3 + stride + 1 + offset]);
                        cstrl_da_float_push_back(&positions, data[l * 3 + stride + 2 + offset]);
                        cstrl_da_float_push_back(&colors, 0.4f);
                        cstrl_da_float_push_back(&colors, 0.7f);
                        cstrl_da_float_push_back(&colors, 0.2f);
                        cstrl_da_float_push_back(&colors, 1.0f);
                    }
                    break;
                }
                case cgltf_attribute_type_color:
                    break;
                default:
                    break;
                }
            }
        }
    }

    cstrl_renderer_add_positions(scene.render_datum[0], positions.array, 3, positions.size / 3, CSTRL_USAGE_DYNAMIC);
    cstrl_renderer_add_colors(scene.render_datum[0], colors.array, CSTRL_USAGE_STATIC);
    cstrl_renderer_add_indices(scene.render_datum[0], indices.array, indices.size, CSTRL_USAGE_STATIC);

    cstrl_da_float_free(&positions);
    cstrl_da_float_free(&colors);
    cstrl_da_int_free(&indices);

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
