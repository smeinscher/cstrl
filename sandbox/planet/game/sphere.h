#ifndef SPHERE_H
#define SPHERE_H

#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_types.h"
void generate_sphere_lat_long(float *positions, int *indices, float *uvs, float *normals, int latitude_point_count, int longitude_point_count);

void generate_partial_terrain_face_mesh(da_float *positions, da_int *indices, int resolution, vec2 size, vec3 position);

void generate_sphere_cube(float *positions, int *indices, float *uvs, float *normals, int resolution);

#endif
