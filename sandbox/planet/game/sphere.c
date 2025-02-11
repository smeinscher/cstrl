#include "sphere.h"
#include "cstrl/cstrl_math.h"

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

static void generate_terrain_face_mesh(float *positions, int *indices, float *uvs, float *normals, int vertex_offset,
                                       int indices_offset, int uvs_offset, int resolution, vec3 local_up)
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
            uvs[i * 2] = ((float)(uvs_offset + 1) / 6.0f) * ((float)x / resolution);
            uvs[i * 2 + 1] = (float)y / resolution;
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
}

void generate_sphere_cube(float *positions, int *indices, float *uvs, float *normals, int resolution)
{
    generate_terrain_face_mesh(positions, indices, uvs, normals, 0, 0, 0, resolution, (vec3){1.0f, 0.0f, 0.0f});
    int position_offset = resolution * resolution;
    int indices_offset = (resolution - 1) * (resolution - 1) * 6;
    generate_terrain_face_mesh(positions, indices, uvs, normals, position_offset, indices_offset, 1, resolution,
                               (vec3){-1.0f, 0.0f, 0.0f});
    position_offset += resolution * resolution;
    indices_offset += (resolution - 1) * (resolution - 1) * 6;
    generate_terrain_face_mesh(positions, indices, uvs, normals, position_offset, indices_offset, 2, resolution,
                               (vec3){0.0f, 1.0f, 0.0f});
    position_offset += resolution * resolution;
    indices_offset += (resolution - 1) * (resolution - 1) * 6;
    generate_terrain_face_mesh(positions, indices, uvs, normals, position_offset, indices_offset, 3, resolution,
                               (vec3){0.0f, -1.0f, 0.0f});
    position_offset += resolution * resolution;
    indices_offset += (resolution - 1) * (resolution - 1) * 6;
    generate_terrain_face_mesh(positions, indices, uvs, normals, position_offset, indices_offset, 4, resolution,
                               (vec3){0.0f, 0.0f, 1.0f});
    position_offset += resolution * resolution;
    indices_offset += (resolution - 1) * (resolution - 1) * 6;
    generate_terrain_face_mesh(positions, indices, uvs, normals, position_offset, indices_offset, 5, resolution,
                               (vec3){0.0f, 0.0f, -1.0f});
}
