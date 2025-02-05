#include "sphere.h"
#include "cstrl/cstrl_math.h"

void generate_sphere(float *positions, int *indices, float *uvs, float *normals, int latitude_point_count,
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
