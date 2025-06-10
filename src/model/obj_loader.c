#include "cstrl/cstrl_model.h"
#include "cstrl/cstrl_renderer.h"
#include "cstrl/cstrl_util.h"
#include "log.c/log.h"
#include <fast_obj/fast_obj.h>
#include <string.h>

CSTRL_API mesh_t cstrl_model_generate_mesh_from_obj_file(const char *path)
{
    fastObjMesh *fast_obj_mesh = fast_obj_read(path);
    mesh_t mesh = {0};
    if (!fast_obj_mesh)
    {
        log_error("Failed to load file");
        return mesh;
    }
    if (fast_obj_mesh->position_count <= 1 || fast_obj_mesh->face_count == 0)
    {
        log_error("No positions and/or indices found in obj");
        return mesh;
    }
    da_float positions;
    cstrl_da_float_init(&positions, fast_obj_mesh->face_count * 6 * 3);
    da_float uvs;
    cstrl_da_float_init(&uvs, fast_obj_mesh->face_count * 6 * 3);
    da_float normals;
    cstrl_da_float_init(&normals, fast_obj_mesh->face_count * 6 * 3);
    da_float colors;
    cstrl_da_float_init(&colors, fast_obj_mesh->face_count * 6 * 4);
    da_int indices;
    cstrl_da_int_init(&indices, fast_obj_mesh->face_count * 6);
    int indices_index = 0;
    for (int i = 0; i < fast_obj_mesh->face_count; i++)
    {
        int face_vertices = fast_obj_mesh->face_vertices[i];

        int p = fast_obj_mesh->indices[indices_index].p;
        cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3]);
        cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 1]);
        cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 2]);
        p = fast_obj_mesh->indices[indices_index + 1].p;
        cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3]);
        cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 1]);
        cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 2]);
        p = fast_obj_mesh->indices[indices_index + 2].p;
        cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3]);
        cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 1]);
        cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 2]);

        int t = fast_obj_mesh->indices[indices_index].t;
        cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2]);
        cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2 + 1]);
        t = fast_obj_mesh->indices[indices_index + 1].t;
        cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2]);
        cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2 + 1]);
        t = fast_obj_mesh->indices[indices_index + 2].t;
        cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2]);
        cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2 + 1]);

        int n = fast_obj_mesh->indices[indices_index].n;
        cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3]);
        cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 1]);
        cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 2]);
        n = fast_obj_mesh->indices[indices_index + 1].n;
        cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3]);
        cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 1]);
        cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 2]);
        n = fast_obj_mesh->indices[indices_index + 2].n;
        cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3]);
        cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 1]);
        cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 2]);

        for (int j = 0; j < 3; j++)
        {
            cstrl_da_float_push_back(&colors, 1.0f);
            cstrl_da_float_push_back(&colors, 1.0f);
            cstrl_da_float_push_back(&colors, 1.0f);
            cstrl_da_float_push_back(&colors, 1.0f);
        }
        cstrl_da_int_push_back(&indices, indices_index);
        cstrl_da_int_push_back(&indices, indices_index + 1);
        cstrl_da_int_push_back(&indices, indices_index + 2);

        if (face_vertices == 4)
        {
            p = fast_obj_mesh->indices[indices_index].p;
            cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3]);
            cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 1]);
            cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 2]);
            p = fast_obj_mesh->indices[indices_index + 2].p;
            cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3]);
            cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 1]);
            cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 2]);
            p = fast_obj_mesh->indices[indices_index + 3].p;
            cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3]);
            cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 1]);
            cstrl_da_float_push_back(&positions, fast_obj_mesh->positions[p * 3 + 2]);

            t = fast_obj_mesh->indices[indices_index].t;
            cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2]);
            cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2 + 1]);
            t = fast_obj_mesh->indices[indices_index + 2].t;
            cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2]);
            cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2 + 1]);
            t = fast_obj_mesh->indices[indices_index + 3].t;
            cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2]);
            cstrl_da_float_push_back(&uvs, fast_obj_mesh->texcoords[t * 2 + 1]);

            n = fast_obj_mesh->indices[indices_index].n;
            cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3]);
            cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 1]);
            cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 2]);
            n = fast_obj_mesh->indices[indices_index + 2].n;
            cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3]);
            cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 1]);
            cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 2]);
            n = fast_obj_mesh->indices[indices_index + 3].n;
            cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3]);
            cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 1]);
            cstrl_da_float_push_back(&normals, fast_obj_mesh->normals[n * 3 + 2]);

            for (int j = 0; j < 3; j++)
            {
                cstrl_da_float_push_back(&colors, 1.0f);
                cstrl_da_float_push_back(&colors, 1.0f);
                cstrl_da_float_push_back(&colors, 1.0f);
                cstrl_da_float_push_back(&colors, 1.0f);
            }
            cstrl_da_int_push_back(&indices, indices_index);
            cstrl_da_int_push_back(&indices, indices_index + 2);
            cstrl_da_int_push_back(&indices, indices_index + 3);
        }
        indices_index += face_vertices;
    }
    mesh.positions = malloc(sizeof(float) * positions.size);
    mesh.uvs = malloc(sizeof(float) * uvs.size);
    mesh.colors = malloc(sizeof(float) * colors.size);
    mesh.normals = malloc(sizeof(float) * normals.size);
    mesh.indices = malloc(sizeof(int) * indices.size);
    if (mesh.positions == NULL || mesh.uvs == NULL || mesh.colors == NULL || mesh.normals == NULL ||
        mesh.indices == NULL)
    {
        log_error("Failed to malloc mesh attribute");
        cstrl_da_float_free(&positions);
        cstrl_da_float_free(&uvs);
        cstrl_da_float_free(&normals);
        cstrl_da_float_free(&colors);
        cstrl_da_int_free(&indices);
        return (mesh_t){0};
    }
    memcpy(mesh.positions, positions.array, sizeof(float) * positions.size);
    mesh.vertex_count = positions.size / 3;
    memcpy(mesh.uvs, uvs.array, sizeof(float) * uvs.size);
    memcpy(mesh.normals, normals.array, sizeof(float) * normals.size);
    memcpy(mesh.colors, colors.array, sizeof(float) * colors.size);
    memcpy(mesh.indices, indices.array, sizeof(int) * indices.size);
    mesh.indices_count = indices.size;
    cstrl_da_float_free(&positions);
    cstrl_da_float_free(&uvs);
    cstrl_da_float_free(&normals);
    cstrl_da_float_free(&colors);
    cstrl_da_int_free(&indices);

    if (fast_obj_mesh->texture_count > 1)
    {
        mesh.textures = cstrl_texture_generate_from_path(fast_obj_mesh->textures[1].path, CSTRL_TEXTURE_FILTER_NEAREST);
    }

    fast_obj_destroy(fast_obj_mesh);

    return mesh;
}
