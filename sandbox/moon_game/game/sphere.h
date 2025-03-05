#ifndef SPHERE_H
#define SPHERE_H

void generate_sphere_lat_long(float *positions, int *indices, float *uvs, float *normals, int latitude_point_count, int longitude_point_count);

void generate_sphere_cube(float *positions, int *indices, float *normals, float *tangents, float *bitangents, int resolution);

#endif
