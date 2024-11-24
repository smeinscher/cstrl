//
// Created by 12105 on 11/22/2024.
//

#ifndef CSTRL_MATH_H
#define CSTRL_MATH_H

#include "cstrl_defines.h"

#include <math.h>

#define cstrl_pi 3.14159265359f
#define cstrl_pi_2 1.57079632679f
#define cstrl_pi_4 0.78539816399f

typedef struct vec2
{
    union {
        struct
        {
            float x;
            float y;
        };
        float v[2];
    };
} vec2;

typedef struct vec2i
{
    union {
        struct
        {
            int x;
            int y;
        };
        int v[2];
    };
} vec2i;

typedef struct vec3
{
    union {
        struct
        {
            float x;
            float y;
            float z;
        };
        float v[3];
    };
} vec3;

typedef struct vec3i
{
    union {
        struct
        {
            int x;
            int y;
            int z;
        };
        int v[3];
    };
} vec3i;

typedef struct vec4
{
    union {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
        float v[4];
    };
} vec4;

typedef struct vec4i
{
    union {
        struct
        {
            int x;
            int y;
            int z;
            int w;
        };
        int v[4];
    };
} vec4i;

// For 2D rotations
typedef struct rot
{
    union {
        struct
        {
            float c;
            float s;
        };
        float v[2];
    };
} rot;

// For 3D rotations
typedef struct quat
{
    union {
        struct
        {
            float w;
            float x;
            float y;
            float z;
        };
        float q[4];
    };
} quat;

typedef struct mat3x3
{
    union {
        struct
        {
            float xx, xy, xz;
            float yx, yy, yz;
            float zx, zy, zz;
        };
        struct
        {
            vec3 x;
            vec3 y;
            vec3 z;
        };
        float m[9];
    };
} mat3x3;
typedef mat3x3 mat3;

typedef struct mat4x4
{
    union {
        struct
        {
            float xx, xy, xz, xw;
            float yx, yy, yz, yw;
            float zx, zy, zz, zw;
            float wx, wy, wz, ww;
        };
        struct
        {
            vec4 x;
            vec4 y;
            vec4 z;
            vec4 w;
        };
        float m[16];
    };
} mat4x4;
typedef mat4x4 mat4;

/*
 *
 *      vec2 math functions
 *
 */

CSTRL_INLINE float cstrl_vec2_length(const vec2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

CSTRL_INLINE vec2 cstrl_vec2_normalize(const vec2 v)
{
    float len = cstrl_vec2_length(v);
    if (len > 0.0f)
    {
        return (vec2){v.x / len, v.y / len};
    }
    return (vec2){0.0f, 0.0f};
}

CSTRL_INLINE vec2 cstrl_vec2_add(const vec2 a, const vec2 b)
{
    return (vec2){a.x + b.x, a.y + b.y};
}

CSTRL_INLINE vec2 cstrl_vec2_sub(const vec2 a, const vec2 b)
{
    return (vec2){a.x - b.x, a.y - b.y};
}

CSTRL_INLINE vec2 cstrl_vec2_mult_scalar(const vec2 v, const float s)
{
    return (vec2){v.x * s, v.y * s};
}

CSTRL_INLINE vec2 cstrl_vec2_mult(const vec2 a, const vec2 b)
{
    return (vec2){a.x * b.x, a.y * b.y};
}

CSTRL_INLINE vec2 cstrl_vec2_div_scalar(const vec2 a, const float s)
{
    return (vec2){a.x / s, a.y / s};
}

CSTRL_INLINE vec2 cstrl_vec2_div(const vec2 a, const vec2 b)
{
    return (vec2){a.x / b.x, a.y / b.y};
}

CSTRL_INLINE float cstrl_vec2_dot(const vec2 a, const vec2 b)
{
    return a.x * b.x + a.y * b.y;
}

/*
 *
 *      vec3 math functions
 *
 */

CSTRL_INLINE float cstrl_vec3_length(const vec3 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

CSTRL_INLINE vec3 cstrl_vec3_normalize(const vec3 v)
{
    float len = cstrl_vec3_length(v);
    if (len > 0.0f)
    {
        return (vec3){v.x / len, v.y / len, v.z / len};
    }
    return (vec3){0, 0, 0};
}

CSTRL_INLINE vec3 cstrl_vec3_add(const vec3 a, const vec3 b)
{
    return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

CSTRL_INLINE vec3 cstrl_vec3_sub(const vec3 a, const vec3 b)
{
    return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
};

CSTRL_INLINE vec3 cstrl_vec3_mult_scalar(const vec3 v, const float s)
{
    return (vec3){v.x * s, v.y * s, v.z * s};
}

CSTRL_INLINE vec3 cstrl_vec3_mult(const vec3 a, const vec3 b)
{
    return (vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}

CSTRL_INLINE vec3 cstrl_vec3_div_scalar(const vec3 v, const float s)
{
    return (vec3){v.x / s, v.y / s, v.z / s};
}

CSTRL_INLINE vec3 cstrl_vec3_div(const vec3 a, const vec3 b)
{
    return (vec3){a.x / b.x, a.y / b.y, a.z / b.z};
}

CSTRL_INLINE float cstrl_vec3_dot(const vec3 a, const vec3 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

CSTRL_INLINE vec3 cstrl_vec3_cross(const vec3 a, const vec3 b)
{
    return (vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

/*
 *
 *      vec4 math functions
 *
 */

CSTRL_INLINE float cstrl_vec4_length(const vec4 v)
{
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z + v.w);
}

CSTRL_INLINE vec4 cstrl_vec4_normalize(const vec4 v)
{
    float len = cstrl_vec4_length(v);
    if (len > 0.0f)
    {
        return (vec4){v.x / len, v.y / len, v.z / len, v.w / len};
    }
    return (vec4){0.0f, 0.0f, 0.0f, 1.0f};
}

CSTRL_INLINE vec4 cstrl_vec4_add(const vec4 a, const vec4 b)
{
    return (vec4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}

CSTRL_INLINE vec4 cstrl_vec4_sub(const vec4 a, const vec4 b)
{
    return (vec4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}

CSTRL_INLINE vec4 cstrl_vec4_mult_scalar(const vec4 v, const float s)
{
    return (vec4){v.x * s, v.y * s, v.z * s, v.w * s};
}

CSTRL_INLINE vec4 cstrl_vec4_mult(const vec4 a, const vec4 b)
{
    return (vec4){a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
}

CSTRL_INLINE vec4 cstrl_vec4_div_scalar(const vec4 v, const float s)
{
    return (vec4){v.x / s, v.y / s, v.z / s, v.w / s};
}

CSTRL_INLINE vec4 cstrl_vec4_div(const vec4 a, const vec4 b)
{
    return (vec4){a.x / b.x, a.y / b.y, a.z / b.z, a.w / b.w};
}

CSTRL_INLINE float cstrl_vec4_dot(const vec4 a, const vec4 b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

#endif // CSTRL_MATH_H
