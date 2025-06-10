#ifndef CSTRL_MODEL_H
#define CSTRL_MODEL_H

#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_renderer.h"

/*
*
*   cgltf loader
*
*/

typedef struct mesh_t
{
    cstrl_texture textures;
    // int texture_count;
    float *positions;
    float *uvs;
    float *colors;
    float *normals;
    int *indices;
    int vertex_count;
    int indices_count;
} mesh_t;

typedef struct model_t
{
    transform_t transform;
    cstrl_texture texture;
    int render_data_index;
    char *name;
} model_t;

typedef struct scene_t
{
    model_t *models;
    int model_count;
    int *model_parents;
    char *name;
    void *scene_internal;
    cstrl_render_data **render_datum;
    int render_data_count;
} scene_t;

CSTRL_API mesh_t cstrl_model_generate_mesh_from_obj_file(const char *path);

CSTRL_API scene_t cstrl_model_generate_scene_from_gltf_file(const char *path);

CSTRL_API void cstrl_model_scene_free(scene_t *scene);

#endif // CSTRL_MODEL_H
