#include "sphere.h"
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_util.h"
#include <stdio.h>

void generate_sphere_lat_long(float *positions, int *indices, float *uvs, float *normals, int latitude_point_count,
                              int longitude_point_count)
{
    int planet_vertices_count = 0;
    int planet_uvs_count = 0;
    float latitude_step = cstrl_pi / latitude_point_count;
    float longitude_step = 2.0f * cstrl_pi / longitude_point_count;
    for (int i = 0; i <= latitude_point_count; i++)
    {
        float latitude_angle = cstrl_pi * 0.5f - (float)i * latitude_step;
        float xy = cosf(latitude_angle);
        float z = sinf(latitude_angle);
        for (int j = 0; j <= longitude_point_count; j++)
        {
            float longitude_angle = j * longitude_step;
            float x = xy * cosf(longitude_angle);
            float y = xy * sinf(longitude_angle);
            normals[planet_vertices_count] = x;
            positions[planet_vertices_count++] = x;
            normals[planet_vertices_count] = y;
            positions[planet_vertices_count++] = y;
            normals[planet_vertices_count] = z;
            positions[planet_vertices_count++] = z;

            float u = (float)j / longitude_point_count;
            float v = (float)i / latitude_point_count;
            uvs[planet_uvs_count++] = u;
            uvs[planet_uvs_count++] = v;
        }
    }
    int indices_count = 0;
    for (int i = 0; i < latitude_point_count; i++)
    {
        int k1 = i * (longitude_point_count + 1);
        int k2 = k1 + longitude_point_count + 1;
        for (int j = 0; j < longitude_point_count; j++)
        {
            if (i != 0)
            {
                indices[indices_count++] = k1;
                indices[indices_count++] = k2;
                indices[indices_count++] = k1 + 1;
            }
            if (i != latitude_point_count - 1)
            {
                indices[indices_count++] = k1 + 1;
                indices[indices_count++] = k2;
                indices[indices_count++] = k2 + 1;
            }

            k1++;
            k2++;
        }
    }
}

void convert_xyz_to_cube_uv(float x, float y, float z, float *u, float *v)
{
    float abs_x = fabsf(x);
    float abs_y = fabsf(y);
    float abs_z = fabsf(z);

    float max_axis, uc, vc;

    // POSITIVE X
    if (x > 0.0f && abs_x >= abs_y && abs_x >= abs_z)
    {
        // u (0 to 1) goes from +z to -z
        // v (0 to 1) goes from -y to +y
        max_axis = abs_x;
        uc = -z;
        vc = y;
        // *index = 0;
    }
    // NEGATIVE X
    if (x <= 0.0f && abs_x >= abs_y && abs_x >= abs_z)
    {
        // u (0 to 1) goes from -z to +z
        // v (0 to 1) goes from -y to +y
        max_axis = abs_x;
        uc = z;
        vc = y;
        // *index = 1;
    }
    // POSITIVE Y
    if (y > 0.0f && abs_y >= abs_x && abs_y >= abs_z)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from +z to -z
        max_axis = abs_y;
        uc = x;
        vc = -z;
        // *index = 2;
    }
    // NEGATIVE Y
    if (y <= 0.0f && abs_y >= abs_x && abs_y >= abs_z)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -z to +z
        max_axis = abs_y;
        uc = x;
        vc = z;
        // *index = 3;
    }
    // POSITIVE Z
    if (z > 0.0f && abs_z >= abs_x && abs_z >= abs_y)
    {
        // u (0 to 1) goes from -x to +x
        // v (0 to 1) goes from -y to +y
        max_axis = abs_z;
        uc = x;
        vc = y;
        // *index = 4;
    }
    // NEGATIVE Z
    if (z <= 0.0f && abs_z >= abs_x && abs_z >= abs_y)
    {
        // u (0 to 1) goes from +x to -x
        // v (0 to 1) goes from -y to +y
        max_axis = abs_z;
        uc = -x;
        vc = y;
        // *index = 5;
    }

    // Convert range from -1 to 1 to 0 to 1
    *u = 0.5f * (uc / max_axis + 1.0f);
    *v = 0.5f * (vc / max_axis + 1.0f);
}

static void generate_terrain_face_mesh(float *positions, int *indices, float *uvs, float *normals, float *tangents,
                                       float *bitangents, int vertex_offset, int indices_offset, int resolution,
                                       vec3 local_up)
{
    vec3 axis_a = {local_up.y, local_up.z, local_up.x};
    vec3 axis_b = cstrl_vec3_cross(local_up, axis_a);

    int indices_index = indices_offset;
    for (int y = 0; y < resolution; y++)
    {
        for (int x = 0; x < resolution; x++)
        {
            int i = vertex_offset + x + y * resolution;
            vec2 percent = cstrl_vec2_div_scalar((vec2){x, y}, resolution - 1);
            vec3 point_on_unit_cube_a = cstrl_vec3_mult_scalar(axis_a, (percent.x - 0.5f) * 2.0f);
            vec3 point_on_unit_cube_b = cstrl_vec3_mult_scalar(axis_b, (percent.y - 0.5f) * 2.0f);
            vec3 point_on_unit_cube =
                cstrl_vec3_add(local_up, cstrl_vec3_add(point_on_unit_cube_a, point_on_unit_cube_b));
            vec3 point_on_unit_sphere = cstrl_vec3_normalize(point_on_unit_cube);
            positions[i * 3] = point_on_unit_sphere.x;
            positions[i * 3 + 1] = point_on_unit_sphere.y;
            positions[i * 3 + 2] = point_on_unit_sphere.z;
            normals[i * 3] = point_on_unit_sphere.x;
            normals[i * 3 + 1] = point_on_unit_sphere.y;
            normals[i * 3 + 2] = point_on_unit_sphere.z;
            convert_xyz_to_cube_uv(point_on_unit_sphere.x, point_on_unit_sphere.y, point_on_unit_sphere.z, &uvs[i * 2],
                                   &uvs[i * 2 + 1]);
            if (x != resolution - 1 && y != resolution - 1)
            {
                indices[indices_index] = i;
                indices[indices_index + 1] = i + resolution + 1;
                indices[indices_index + 2] = i + resolution;
                indices[indices_index + 3] = i;
                indices[indices_index + 4] = i + 1;
                indices[indices_index + 5] = i + resolution + 1;
                indices_index += 6;
            }
        }
    }

    for (int i = 0; i < (resolution - 1) * (resolution - 1) * 6; i += 3)
    {
        int p0 = indices[indices_offset + i];
        int p1 = indices[indices_offset + i + 1];
        int p2 = indices[indices_offset + i + 2];
        vec3 v0 = {positions[p0 * 3], positions[p0 * 3 + 1], positions[p0 * 3 + 2]};
        vec3 v1 = {positions[p1 * 3], positions[p1 * 3 + 1], positions[p1 * 3 + 2]};
        vec3 v2 = {positions[p2 * 3], positions[p2 * 3 + 1], positions[p2 * 3 + 2]};

        vec2 uv0 = {uvs[p0 * 2], uvs[p0 * 2 + 1]};
        vec2 uv1 = {uvs[p1 * 2], uvs[p1 * 2 + 1]};
        vec2 uv2 = {uvs[p2 * 2], uvs[p2 * 2 + 1]};

        vec3 delta_pos_1 = cstrl_vec3_sub(v1, v0);
        vec3 delta_pos_2 = cstrl_vec3_sub(v2, v0);

        vec2 delta_uv_1 = cstrl_vec2_sub(uv1, uv0);
        vec2 delta_uv_2 = cstrl_vec2_sub(uv2, uv0);

        float r = 1.0f / (delta_uv_1.x * delta_uv_2.y - delta_uv_1.y * delta_uv_2.x);
        vec3 tangent_a = cstrl_vec3_mult_scalar(delta_pos_1, delta_uv_2.y);
        vec3 tangent_b = cstrl_vec3_mult_scalar(delta_pos_2, delta_uv_1.y);
        vec3 tangent = cstrl_vec3_normalize(cstrl_vec3_mult_scalar(cstrl_vec3_sub(tangent_a, tangent_b), r));
        vec3 bitangent_a = cstrl_vec3_mult_scalar(delta_pos_2, delta_uv_1.x);
        vec3 bitangent_b = cstrl_vec3_mult_scalar(delta_pos_1, delta_uv_2.x);
        vec3 bitangent = cstrl_vec3_normalize(cstrl_vec3_mult_scalar(cstrl_vec3_sub(bitangent_a, bitangent_b), r));

        tangents[p0 * 3] = tangent.x;
        tangents[p0 * 3 + 1] = tangent.y;
        tangents[p0 * 3 + 2] = tangent.z;
        tangents[p1 * 3] = tangent.x;
        tangents[p1 * 3 + 1] = tangent.y;
        tangents[p1 * 3 + 2] = tangent.z;
        tangents[p2 * 3] = tangent.x;
        tangents[p2 * 3 + 1] = tangent.y;
        tangents[p2 * 3 + 2] = tangent.z;

        bitangents[p0 * 3] = bitangent.x;
        bitangents[p0 * 3 + 1] = bitangent.y;
        bitangents[p0 * 3 + 2] = bitangent.z;
        bitangents[p1 * 3] = bitangent.x;
        bitangents[p1 * 3 + 1] = bitangent.y;
        bitangents[p1 * 3 + 2] = bitangent.z;
        bitangents[p2 * 3] = bitangent.x;
        bitangents[p2 * 3 + 1] = bitangent.y;
        bitangents[p2 * 3 + 2] = bitangent.z;
    }
}

void generate_sphere_cube(float *positions, int *indices, float *normals, float *tangents, float *bitangents,
                          int resolution)
{
    float uvs[resolution * resolution * 12];
    generate_terrain_face_mesh(positions, indices, uvs, normals, tangents, bitangents, 0, 0, resolution,
                               (vec3){1.0f, 0.0f, 0.0f});
    int position_offset = resolution * resolution;
    int indices_offset = (resolution - 1) * (resolution - 1) * 6;
    generate_terrain_face_mesh(positions, indices, uvs, normals, tangents, bitangents, position_offset, indices_offset,
                               resolution, (vec3){-1.0f, 0.0f, 0.0f});
    position_offset += resolution * resolution;
    indices_offset += (resolution - 1) * (resolution - 1) * 6;
    generate_terrain_face_mesh(positions, indices, uvs, normals, tangents, bitangents, position_offset, indices_offset,
                               resolution, (vec3){0.0f, 1.0f, 0.0f});
    position_offset += resolution * resolution;
    indices_offset += (resolution - 1) * (resolution - 1) * 6;
    generate_terrain_face_mesh(positions, indices, uvs, normals, tangents, bitangents, position_offset, indices_offset,
                               resolution, (vec3){0.0f, -1.0f, 0.0f});
    position_offset += resolution * resolution;
    indices_offset += (resolution - 1) * (resolution - 1) * 6;
    generate_terrain_face_mesh(positions, indices, uvs, normals, tangents, bitangents, position_offset, indices_offset,
                               resolution, (vec3){0.0f, 0.0f, 1.0f});
    position_offset += resolution * resolution;
    indices_offset += (resolution - 1) * (resolution - 1) * 6;
    generate_terrain_face_mesh(positions, indices, uvs, normals, tangents, bitangents, position_offset, indices_offset,
                               resolution, (vec3){0.0f, 0.0f, -1.0f});
}
