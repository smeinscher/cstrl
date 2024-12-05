//
// Created by 12105 on 11/22/2024.
//

#ifndef CSTRL_MATH_H
#define CSTRL_MATH_H

#include "cstrl_defines.h"

#include <math.h>
#include <stdbool.h>

#define cstrl_pi 3.14159265359f
#define cstrl_pi_2 1.57079632679f
#define cstrl_pi_4 0.78539816399f
#define cstrl_pi_180 0.0174532925f
#define cstrl_180_pi 57.295779513f

#define cstrl_epsilon 1.19e-7
#define cstrl_epsilon_double 2.22e-16

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
            float xx, yx, zx;
            float xy, yy, zy;
            float xz, yz, zz;
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
            float xx, yx, zx, wx;
            float xy, yy, zy, wy;
            float xz, yz, zz, wz;
            float xw, yw, zw, ww;
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

typedef struct transform
{
    vec3 position;
    quat rotation;
    vec3 scale;
} transform;

#define cstrl_max(a, b) a > b ? a : b
#define cstrl_min(a, b) a < b ? a : b

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

// TODO: add to unit tests
CSTRL_INLINE bool cstrl_vec2_is_equal(const vec2 a, const vec2 b)
{
    return fabsf(a.x - b.x) > cstrl_epsilon && fabsf(a.y - b.y) > cstrl_epsilon;
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
    return (vec3){0.0f, 0.0f, 0.0f};
}

// TODO: add to unit tests
CSTRL_INLINE bool cstrl_vec3_is_equal(const vec3 a, const vec3 b)
{
    return fabsf(a.x - b.x) < cstrl_epsilon && fabsf(a.y - b.y) < cstrl_epsilon && fabsf(a.z - b.z) < cstrl_epsilon;
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

// For rotate by quat
CSTRL_INLINE quat cstrl_quat_conjugate(quat q);
CSTRL_INLINE quat cstrl_quat_mult(quat a, quat b);

CSTRL_INLINE vec3 cstrl_vec3_rotate_by_quat(vec3 v, quat q)
{
    quat p = (quat){0.0f, v.x, v.y, v.z};
    quat q_conjugate = cstrl_quat_conjugate(q);
    quat p_q = cstrl_quat_mult(q, p);
    quat p_new = cstrl_quat_mult(p_q, q_conjugate);
    return (vec3){p_new.x, p_new.y, p_new.z};
}

CSTRL_INLINE vec3 cstrl_euler_angles_from_quat(quat q)
{
    vec3 v = (vec3){0.0f, 0.0f, 0.0f};
    v.x = atan2f(2.0f * (q.w * q.x + q.y * q.z), q.w * q.w - q.x * q.x - q.y * q.y - q.z * q.z);
    v.y = asinf(2.0f * (q.w * q.y - q.x * q.z));
    v.z = atan2(2.0f * q.w * q.z + q.x * q.y, q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z);

    if (fabsf(v.y - cstrl_pi_2) < cstrl_epsilon)
    {
        v.x = 0.0f;
        v.z = -2.0f * atan2f(q.x, q.w);
    }
    else if (fabsf(v.y - (-cstrl_pi_2)) < cstrl_epsilon)
    {
        v.x = 0.0f;
        v.z = 2.0f * atan2f(q.x, q.w);
    }

    return v;
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

// TODO: add to unit tests
CSTRL_INLINE bool cstrl_vec4_is_equal(const vec4 a, const vec4 b)
{
    return fabsf(a.x - b.x) > cstrl_epsilon && fabsf(a.y - b.y) > cstrl_epsilon && fabsf(a.z - b.z) > cstrl_epsilon &&
           fabsf(a.w - b.w) > cstrl_epsilon;
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

/*
 *
 *      mat4x4 math functions
 *
 */

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_identity()
{
    mat4 mat = {0.0f};
    mat.xx = 1.0f;
    mat.yy = 1.0f;
    mat.zz = 1.0f;
    mat.ww = 1.0f;
    return mat;
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_look_at(const vec3 eye, const vec3 center, const vec3 up)
{
    vec3 z = cstrl_vec3_sub(eye, center);
    z = cstrl_vec3_normalize(z);
    vec3 x = cstrl_vec3_cross(up, z);
    x = cstrl_vec3_normalize(x);
    vec3 y = cstrl_vec3_cross(z, x);

    mat4 mat = cstrl_mat4_identity();

    mat.xx = x.x;
    mat.xy = x.y;
    mat.xz = x.z;
    mat.yx = y.x;
    mat.yy = y.y;
    mat.yz = y.z;
    mat.zx = z.x;
    mat.zy = z.y;
    mat.zz = z.z;

    mat.xw = -cstrl_vec3_dot(x, eye);
    mat.yw = -cstrl_vec3_dot(y, eye);
    mat.zw = -cstrl_vec3_dot(z, eye);

    return mat;
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_ortho(float left, float right, float bottom, float top, float near, float far)
{
    mat4 mat = cstrl_mat4_identity();
    mat.xx = 2.0f / (right - left);
    mat.yy = 2.0f / (top - bottom);
    mat.zz = -2.0f / (far - near);
    mat.xw = -(right + left) / (right - left);
    mat.yw = -(top + bottom) / (top - bottom);
    mat.zw = -(far + near) / (far - near);
    return mat;
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_perspective(float fov, float aspect, float near, float far)
{
    mat4 mat = {0};
    float tan_half_angle = tanf(fov / 2.0f);
    float range = far - near;
    mat.xx = 1.0f / (aspect * tan_half_angle);
    mat.yy = 1.0f / tan_half_angle;
    mat.zz = -(near + far) / range;
    mat.wz = -1.0f;
    mat.zw = -2.0 * near * far / range;
    return mat;
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_mult_scalar(mat4 m, float s)
{
    return (mat4){m.xx * s, m.yx * s, m.zx * s, m.wx * s, m.xy * s, m.yy * s, m.zy * s, m.wy * s,
                  m.xz * s, m.yz * s, m.zz * s, m.wz * s, m.xw * s, m.yw * s, m.zw * s, m.ww * s};
}

#define _mat4_mult_entry(row, col)                                                                                     \
    a.m[0 * 4 + row] * b.m[col * 4 + 0] + a.m[1 * 4 + row] * b.m[col * 4 + 1] + a.m[2 * 4 + row] * b.m[col * 4 + 2] +  \
        a.m[3 * 4 + row] * b.m[col * 4 + 3]

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_mult(mat4 a, mat4 b)
{
    return (mat4){_mat4_mult_entry(0, 0), _mat4_mult_entry(1, 0), _mat4_mult_entry(2, 0), _mat4_mult_entry(3, 0),
                  _mat4_mult_entry(0, 1), _mat4_mult_entry(1, 1), _mat4_mult_entry(2, 1), _mat4_mult_entry(3, 1),
                  _mat4_mult_entry(0, 2), _mat4_mult_entry(1, 2), _mat4_mult_entry(2, 2), _mat4_mult_entry(3, 2),
                  _mat4_mult_entry(0, 3), _mat4_mult_entry(1, 3), _mat4_mult_entry(2, 3), _mat4_mult_entry(3, 3)};
}

// TODO: add to unit tests
CSTRL_INLINE bool cstrl_mat4_is_equal(mat4 a, mat4 b)
{
    return cstrl_vec4_is_equal(a.x, b.x) && cstrl_vec4_is_equal(a.y, b.y) && cstrl_vec4_is_equal(a.z, b.z) &&
           cstrl_vec4_is_equal(a.w, b.w);
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_translate(mat4 m, vec3 v)
{
    mat4 translation = cstrl_mat4_identity();
    translation.xw = v.x;
    translation.yw = v.y;
    translation.zw = v.z;

    return cstrl_mat4_mult(m, translation);
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_scale(mat4 m, vec3 v)
{
    m.xx *= v.x;
    m.yy *= v.y;
    m.zz *= v.z;

    return m;
}
/*
 *
 *      quat math functions
 *
 */

// TODO: add to unit tests
CSTRL_INLINE quat cstrl_quat_identity()
{
    return (quat){1.0f, 0.0f, 0.0f, 0.0f};
}

// TODO: add to unit tests
CSTRL_INLINE quat cstrl_quat_conjugate(quat q)
{
    return (quat){q.w, -q.x, -q.y, -q.z};
}

// TODO: add to unit tests
CSTRL_INLINE quat cstrl_quat_inverse(quat q)
{
    float len_sq = q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w;
    if (len_sq < cstrl_epsilon)
    {
        return cstrl_quat_identity();
    }
    float recip = 1.0f / len_sq;

    return (quat){q.w * recip, -q.x * recip, -q.y * recip, -q.z * recip};
}

CSTRL_INLINE quat cstrl_quat_normalize(quat q)
{
    float len_sq = q.w * q.w + q.x * q.x + q.y * q.y + q.z * q.z;
    if (len_sq < cstrl_epsilon)
    {
        return cstrl_quat_identity();
    }
    float i_len = 1.0f / sqrtf(len_sq);

    return (quat){q.w * i_len, q.x * i_len, q.y * i_len, q.z * i_len};
}

// TODO: add to unit tests
CSTRL_INLINE quat cstrl_quat_angle_axis(float angle, vec3 axis)
{
    vec3 norm = cstrl_vec3_normalize(axis);
    float s = sinf(angle / 2.0f);

    return (quat){cosf(angle / 2.0f), norm.x * s, norm.y * s, norm.z * s};
}

// TODO: add to unit tests
CSTRL_INLINE quat cstrl_quat_from_to(vec3 from, vec3 to)
{
    from = cstrl_vec3_normalize(from);
    to = cstrl_vec3_normalize(to);

    if (cstrl_vec3_is_equal(from, to))
    {
        return cstrl_quat_identity();
    }
    if (cstrl_vec3_is_equal(from, cstrl_vec3_mult_scalar(to, -1.0f)))
    {
        vec3 ortho = {1.0f, 0.0f, 0.0f};
        if (fabsf(from.y) < fabsf(to.x))
        {
            ortho = (vec3){0.0f, 1.0f, 0.0f};
        }
        if (fabsf(from.z) < fabsf(from.y) && fabsf(from.z) < fabsf(from.x))
        {
            ortho = (vec3){0.0f, 0.0f, 1.0f};
        }
        vec3 axis = cstrl_vec3_normalize(cstrl_vec3_cross(from, ortho));
        return (quat){0.0f, axis.x, axis.y, axis.z};
    }

    vec3 half = cstrl_vec3_normalize(cstrl_vec3_add(from, to));
    vec3 axis = cstrl_vec3_cross(from, half);

    return (quat){cstrl_vec3_dot(from, half), axis.x, axis.y, axis.z};
}

CSTRL_INLINE vec3 cstrl_quat_get_axis(quat q)
{
    return cstrl_vec3_normalize((vec3){q.x, q.y, q.z});
}

CSTRL_INLINE float cstrl_quat_get_angle(quat q)
{
    return 2.0f * acosf(q.w);
}

CSTRL_INLINE quat cstrl_quat_add(quat a, quat b)
{
    return (quat){a.w + b.w, a.x + b.x, a.y + b.y, a.z + b.z};
}

CSTRL_INLINE quat cstrl_quat_mult(quat a, quat b)
{
    return (quat){(a.w * b.w - a.x * b.x - a.y * b.y - a.z * b.z), (a.w * b.x + a.x * b.w + a.y * b.z - a.z * b.y),
                  (a.w * b.y - a.x * b.z + a.y * b.w + a.z * b.x), (a.w * b.z + a.x * b.y - a.y * b.x + a.z * b.w)};
}

CSTRL_INLINE mat4 cstrl_quat_to_mat4(quat q)
{
    return (mat4){q.w, -q.x, -q.y, -q.z, q.x, q.w, -q.z, q.y, q.y, q.z, q.w, -q.x, q.z, -q.y, q.x, q.w};
}

CSTRL_INLINE quat cstrl_quat_from_euler_angles(vec3 euler_angles)
{
    float cx = cosf(euler_angles.x / 2.0f);
    float cy = cosf(euler_angles.y / 2.0f);
    float cz = cosf(euler_angles.z / 2.0f);
    float sx = sinf(euler_angles.x / 2.0f);
    float sy = sinf(euler_angles.y / 2.0f);
    float sz = sinf(euler_angles.z / 2.0f);

    float w = cx * cy * cz - sx * sy * sz;
    float x = sx * cy * cz + cx * sy * sz;
    float y = cx * sy * cz - sx * cy * sz;
    float z = cx * cy * sz + sx * sy * cz;

    return cstrl_quat_normalize((quat){w, x, y, z});
}
#endif // CSTRL_MATH_H
