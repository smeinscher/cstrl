//
// Created by 12105 on 11/22/2024.
//

#ifndef CSTRL_MATH_H
#define CSTRL_MATH_H

#include "cstrl_defines.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

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
        struct
        {
            float r;
            float g;
            float b;
            float a;
        };
        struct
        {
            float u0;
            float v0;
            float u1;
            float v1;
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

#define cstrl_max(a, b) (a > b ? a : b)
#define cstrl_min(a, b) (a < b ? a : b)

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

// TODO: add to unit tests
CSTRL_INLINE vec2 cstrl_vec2_negate(const vec2 v)
{
    return (vec2){-v.x, -v.y};
}

CSTRL_INLINE bool cstrl_vec2_point_inside_rect(const vec2 m, const vec2 a, const vec2 b, const vec2 d)
{
    vec2 am = cstrl_vec2_mult(a, m);
    vec2 ab = cstrl_vec2_mult(b, m);
    vec2 ad = cstrl_vec2_mult(a, d);

    float am_dot_ab = cstrl_vec2_dot(am, ab);
    float ab_dot_ab = cstrl_vec2_dot(ab, ab);
    float am_dot_ad = cstrl_vec2_dot(am, ad);
    float ad_dot_ad = cstrl_vec2_dot(ad, ad);

    return am_dot_ab > 0.0f && am_dot_ab < ab_dot_ab && am_dot_ad > 0.0f && am_dot_ad < ad_dot_ad;
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

// TODO: add to unit tests
CSTRL_INLINE vec3 cstrl_vec3_rotate_along_axis(const vec3 v, const float angle, const vec3 axis)
{
    vec3 axis_normalized = cstrl_vec3_normalize(axis);
    float cos_theta = cos(angle);
    float sin_theta = sin(angle);

    vec3 t1 = cstrl_vec3_mult_scalar(axis_normalized, (1 - cos_theta));
    vec3 t2 = cstrl_vec3_cross(axis_normalized, v);
    vec3 t3 = cstrl_vec3_cross(t1, t2);
    vec3 t4 = cstrl_vec3_add(v, t3);
    return cstrl_vec3_add(t4, cstrl_vec3_mult_scalar(cstrl_vec3_cross(axis_normalized, v), sin_theta));
}

// TODO: add to unit tests
CSTRL_INLINE vec3 cstrl_vec3_rotate_by_euler(const vec3 v, float heading, float pitch, float bank)
{
    float c1 = cosf(heading / 2.0f);
    float s1 = sinf(heading / 2.0f);
    float c2 = cosf(pitch / 2.0f);
    float s2 = sinf(pitch / 2.0f);
    float c3 = cosf(bank / 2.0f);
    float s3 = cosf(bank / 2.0f);

    float w = c1 * c2 * c3 - s1 * s2 * s3;
    float x = c1 * c2 * s3 + s1 * s2 * c3;
    float y = s1 * c2 * c3 + c1 * s2 * s3;
    float z = c1 * s2 * c3 - s1 * c2 * s3;

    float angle = 2 * acos(w);

    float norm = x * x + y * y + z * z;
    if (norm < cstrl_epsilon)
    {
        x = 1;
        y = z = 0;
    }
    else
    {
        norm = sqrtf(norm);
        x /= norm;
        y /= norm;
        z /= norm;
    }

    return cstrl_vec3_rotate_along_axis(v, angle, (vec3){x, y, z});
}

CSTRL_INLINE vec3 cstrl_vec3_rotate_by_quat(const vec3 v, const quat q)
{
    quat p = (quat){0.0f, v.x, v.y, v.z};
    quat q_conjugate = cstrl_quat_conjugate(q);
    quat p_q = cstrl_quat_mult(q, p);
    quat p_new = cstrl_quat_mult(p_q, q_conjugate);
    return (vec3){p_new.x, p_new.y, p_new.z};
}

// TODO: add to unit tests
CSTRL_INLINE vec3 cstrl_vec3_negate(const vec3 v)
{
    return (vec3){-v.x, -v.y, -v.z};
}

// TODO: add to unit tests
CSTRL_INLINE vec3 cstrl_vec3_mult_mat3(const vec3 v, const mat3 m)
{
    return (vec3){m.xx * v.x + m.yx * v.y + m.zx * v.z, m.xy * v.x + m.yy * v.y + m.zy * v.z,
                  m.xz * v.x + m.yz * v.y + m.zz * v.z};
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

// TODO: add to unit tests
CSTRL_INLINE vec4 cstrl_vec4_negate(const vec4 v)
{
    return (vec4){-v.x, -v.y, -v.z, -v.w};
}

// TODO: add to unit tests
CSTRL_INLINE vec3 cstrl_vec4_to_vec3(const vec4 v)
{
    return (vec3){v.x, v.y, v.z};
}

CSTRL_INLINE vec4 cstrl_vec4_mult_mat4(const vec4 v, const mat4 m)
{
    // Assumes column vector
    float x = v.x * m.xx + v.y * m.yx + v.z * m.zx + v.w * m.wx;
    float y = v.x * m.xy + v.y * m.yy + v.z * m.zy + v.w * m.wy;
    float z = v.x * m.xz + v.y * m.yz + v.z * m.zz + v.w * m.wz;
    float w = v.x * m.xw + v.y * m.yw + v.z * m.zw + v.w * m.ww;

    return (vec4){x, y, z, w};
}

/*
 *
 *      mat3x3 math functions
 *
 */

// TODO: add to unit tests
CSTRL_INLINE float cstrl_mat3_determinant(mat3 m)
{
    return m.xx * (m.yy * m.zz - m.zy * m.yz) - m.xy * (m.yx * m.zz - m.zx * m.yz) + m.xz * (m.yx * m.zy - m.yy * m.zx);
}

// TODO: add to unit tests
CSTRL_INLINE mat3 cstrl_mat3_adjugate(mat3 m)
{
    mat3 result;

    result.xx = m.yy * m.zz - m.yz * m.zy;
    result.yx = m.xy * m.zz - m.xz * m.zy;
    result.zx = m.xz * m.yy - m.xy * m.yz;

    result.xy = m.yx * m.zz - m.yz * m.zx;
    result.yy = m.xx * m.zz - m.xz * m.zx;
    result.zy = m.xx * m.yz - m.xz * m.yx;

    result.xz = m.zx * m.yy - m.yx * m.zy;
    result.yz = m.xx * m.zy - m.xy * m.zx;
    result.zz = m.xx * m.yy - m.xy * m.yx;

    return result;
}

// TODO: add to unit tests
CSTRL_INLINE mat3 cstrl_mat3_negate(mat3 m)
{
    for (int i = 0; i < 9; i++)
    {
        m.m[i] *= -1;
    }

    return m;
}

// TODO: add to unit tests
CSTRL_INLINE mat3 cstrl_mat3_inverse(mat3 m)
{
    float det = cstrl_mat3_determinant(m);
    mat3 adj = cstrl_mat3_adjugate(m);

    for (int i = 0; i < 9; i++)
    {
        adj.m[i] /= det;
    }

    return adj;
}

// TODO: add to unit tests
CSTRL_INLINE quat cstrl_mat3_orthogonal_to_quat(mat3 m)
{
    quat q;
    float t = m.xx + m.yy + m.zz;
    if (fabsf(1.0f + t) > cstrl_epsilon)
    {
        float r = sqrtf(1.0f + t);
        float s = 1.0f / (2.0f * r);
        q.w = r * 0.5f;
        q.x = (m.zy - m.yz) * s;
        q.y = (m.xz - m.zx) * s;
        q.z = (m.yx - m.xy) * s;
    }
    else
    {
        if (m.xx > m.yy && m.xx > m.zz)
        {
            float r = sqrtf(1 + m.xx - m.yy - m.zz);
            float s = 1.0f / (2.0f * r);
            q.w = (m.zy - m.yz) * s;
            q.x = r * 0.5f;
            q.y = (m.yx + m.xy) * s;
            q.z = (m.xz + m.zx) * s;
        }
        else if (m.yy > m.xx && m.yy > m.zz)
        {
            float r = sqrtf(1 + m.yy - m.xx - m.zz);
            float s = 1.0f / (2.0f * r);
            q.w = (m.xz - m.zx) * s;
            q.x = (m.xy - m.yx);
            q.y = r * 0.5f;
            q.z = (m.zy - m.yz) * s;
        }
        else
        {
            float r = sqrtf(1 + m.zz - m.xx - m.yy);
            float s = 1.0f / (2.0f * r);
            q.w = (m.yx - m.xy) * s;
            q.x = (m.xz + m.zx) * s;
            q.y = (m.zy + m.yz) * s;
            q.z = r * 0.5f;
        }
    }
    return q;
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
CSTRL_INLINE mat4 cstrl_mat4_add(const mat4 a, const mat4 b)
{
    mat4 m;

    for (int i = 0; i < 16; i++)
    {
        m.m[i] = a.m[i] + b.m[i];
    }

    return m;
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_add_scalar(const mat4 m, const float s)
{
    mat4 result;

    for (int i = 0; i < 16; i++)
    {
        result.m[i] = m.m[i] + s;
    }

    return result;
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_look_at(const vec3 eye, const vec3 center, const vec3 up)
{
    vec3 z = cstrl_vec3_sub(eye, center);
    z = cstrl_vec3_normalize(z);
    vec3 x = cstrl_vec3_cross(up, z);
    x = cstrl_vec3_normalize(x);
    vec3 y = cstrl_vec3_cross(z, x);
    y = cstrl_vec3_normalize(y);

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

// TODO: add to unit tests
// Rodrigues' rotation formula
CSTRL_INLINE mat4 cstrl_mat4_rotate(mat4 m, float angle, vec3 axis)
{
    mat4 result = cstrl_mat4_identity();

    mat4 k = {0};
    k.yx = -axis.z;
    k.zx = axis.y;
    k.xy = axis.z;
    k.zy = -axis.x;
    k.xz = -axis.y;
    k.yz = axis.x;

    mat4 m0 = cstrl_mat4_mult_scalar(k, sinf(angle));
    mat4 m1 = cstrl_mat4_mult(k, k);
    m1 = cstrl_mat4_mult_scalar(m1, (1 - cosf(angle)));

    result = cstrl_mat4_add(result, m0);
    result = cstrl_mat4_add(result, m1);

    return result;
}

// TODO: add to unit tests
CSTRL_INLINE float cstrl_mat4_determinant(mat4 m)
{
    return m.xw * m.yz * m.zy * m.wx - m.xz * m.yw * m.zy * m.wx - m.xw * m.yy * m.zz * m.wx +
           m.xy * m.yw * m.zz * m.wx + m.xz * m.yy * m.zw * m.wx - m.xy * m.yz * m.zw * m.wx -
           m.xw * m.yz * m.zx * m.wy + m.xz * m.yw * m.zx * m.wy + m.xw * m.yx * m.zz * m.wy -
           m.xx * m.yw * m.zz * m.wy - m.xz * m.yx * m.zw * m.wy + m.xx * m.yz * m.zw * m.wy +
           m.xw * m.yy * m.zx * m.wz - m.xy * m.yw * m.zx * m.wz - m.xw * m.yx * m.zy * m.wz +
           m.xx * m.yw * m.zy * m.wz + m.xy * m.yx * m.zw * m.wz - m.xx * m.yy * m.zw * m.wz -
           m.xz * m.yy * m.zx * m.ww + m.xy * m.yz * m.zx * m.ww + m.xz * m.yx * m.zy * m.ww -
           m.xx * m.yz * m.zy * m.ww - m.xy * m.yx * m.zz * m.ww + m.xx * m.yy * m.zz * m.ww;
}

// TODO: add to unit tests
CSTRL_INLINE float cstrl_mat4_cofactor(mat4 m, int i, int j)
{
    return powf(-1, i + j);
}

CSTRL_INLINE mat4 cstrl_mat4_affine_inverse(mat4 m)
{
    mat3 m3;

    m3.xx = m.xx;
    m3.xy = m.xy;
    m3.xz = m.xz;
    m3.yx = m.yx;
    m3.yy = m.yy;
    m3.yz = m.yz;
    m3.zx = m.zx;
    m3.zy = m.zy;
    m3.zz = m.zz;

    mat3 upper_left = cstrl_mat3_inverse(m3);

    vec3 translate = cstrl_vec3_mult_mat3((vec3){m.xw, m.yw, m.zw}, cstrl_mat3_negate(upper_left));

    return (mat4){upper_left.xx, upper_left.yx, upper_left.zx, 0.0f, upper_left.xy, upper_left.yy, upper_left.zy, 0.0f,
                  upper_left.xz, upper_left.yz, upper_left.zz, 0.0f, -translate.x,  -translate.y,  translate.z,   1.0f};
}

// TODO: add to unit tests
CSTRL_INLINE mat3 cstrl_mat4_upper_left(mat4 m)
{
    return (mat3){m.xx, m.yx, m.zx, m.xy, m.yy, m.zy, m.xz, m.yz, m.zz};
}

// TODO: add to unit tests
// From glm compute_inverse function
CSTRL_INLINE mat4 cstrl_mat4_inverse_glm(mat4 m)
{
    float coef00 = m.zz * m.ww - m.wz * m.zw;
    float coef02 = m.yz * m.ww - m.wz * m.yw;
    float coef03 = m.yz * m.zw - m.zz * m.yw;

    float coef04 = m.zy * m.ww - m.wy * m.zw;
    float coef06 = m.yy * m.ww - m.wy * m.yw;
    float coef07 = m.yy * m.zw - m.zy * m.yw;

    float coef08 = m.zy * m.wz - m.wy * m.zz;
    float coef10 = m.yy * m.wz - m.wy * m.yz;
    float coef11 = m.yy * m.zz - m.zy * m.yz;

    float coef12 = m.zx * m.ww - m.wx * m.zw;
    float coef14 = m.yx * m.ww - m.wx * m.yw;
    float coef15 = m.yx * m.zw - m.zx * m.yw;

    float coef16 = m.zx * m.wz - m.wx * m.zz;
    float coef18 = m.yx * m.wz - m.wx * m.yz;
    float coef19 = m.yx * m.zz - m.zx * m.yz;

    float coef20 = m.zx * m.wy - m.wx * m.zy;
    float coef22 = m.yx * m.wy - m.wx * m.yy;
    float coef23 = m.yx * m.zy - m.zx * m.yy;

    vec4 fac0 = {coef00, coef00, coef02, coef03};
    vec4 fac1 = {coef04, coef04, coef06, coef07};
    vec4 fac2 = {coef08, coef08, coef10, coef11};
    vec4 fac3 = {coef12, coef12, coef14, coef15};
    vec4 fac4 = {coef16, coef16, coef18, coef19};
    vec4 fac5 = {coef20, coef20, coef22, coef23};

    vec4 vec_0 = {m.yx, m.xx, m.xx, m.xx};
    vec4 vec_1 = {m.yy, m.xy, m.xy, m.xy};
    vec4 vec_2 = {m.yz, m.xz, m.xz, m.xz};
    vec4 vec_3 = {m.yw, m.xw, m.xw, m.xw};

    vec4 inv0_a = cstrl_vec4_mult(vec_1, fac0);
    vec4 inv0_b = cstrl_vec4_mult(vec_2, fac1);
    vec4 inv0_c = cstrl_vec4_mult(vec_3, fac2);
    vec4 inv0_d = cstrl_vec4_sub(inv0_a, inv0_b);
    vec4 inv0 = cstrl_vec4_add(inv0_d, inv0_c);

    vec4 inv1_a = cstrl_vec4_mult(vec_0, fac0);
    vec4 inv1_b = cstrl_vec4_mult(vec_2, fac3);
    vec4 inv1_c = cstrl_vec4_mult(vec_3, fac4);
    vec4 inv1_d = cstrl_vec4_sub(inv1_a, inv1_b);
    vec4 inv1 = cstrl_vec4_add(inv1_d, inv1_c);

    vec4 inv2_a = cstrl_vec4_mult(vec_0, fac1);
    vec4 inv2_b = cstrl_vec4_mult(vec_1, fac3);
    vec4 inv2_c = cstrl_vec4_mult(vec_3, fac5);
    vec4 inv2_d = cstrl_vec4_sub(inv0_a, inv0_b);
    vec4 inv2 = cstrl_vec4_add(inv0_d, inv0_c);

    vec4 inv3_a = cstrl_vec4_mult(vec_0, fac2);
    vec4 inv3_b = cstrl_vec4_mult(vec_1, fac4);
    vec4 inv3_c = cstrl_vec4_mult(vec_2, fac5);
    vec4 inv3_d = cstrl_vec4_sub(inv1_a, inv1_b);
    vec4 inv3 = cstrl_vec4_add(inv1_d, inv1_c);

    vec4 sign_a = {1.0f, -1.0f, 1.0f, -1.0f};
    vec4 sign_b = {-1.0f, 1.0f, -1.0f, 1.0f};

    vec4 inverse_x = cstrl_vec4_mult(inv0, sign_a);
    vec4 inverse_y = cstrl_vec4_mult(inv1, sign_b);
    vec4 inverse_z = cstrl_vec4_mult(inv2, sign_a);
    vec4 inverse_w = cstrl_vec4_mult(inv3, sign_b);

    mat4 inverse =
        (mat4){inverse_x.x, inverse_x.y, inverse_x.z, inverse_x.w, inverse_y.x, inverse_y.y, inverse_y.z, inverse_y.w,
               inverse_z.x, inverse_z.y, inverse_z.z, inverse_z.w, inverse_w.x, inverse_w.y, inverse_w.z, inverse_w.w};

    vec4 row0 = {inverse.xx, inverse.xy, inverse.xz, inverse.xw};

    vec4 dot0 = cstrl_vec4_mult(m.x, row0);

    float dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

    float one_over_determinant = 1 / dot1;

    return cstrl_mat4_mult_scalar(inverse, one_over_determinant);
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_inverse(mat4 m)
{
    mat4 result;
    result.xx = m.yz * m.zw * m.wy - m.yw * m.zz * m.wy + m.yw * m.zy * m.wz - m.yy * m.zw * m.wz - m.yz * m.zy * m.ww +
                m.yy * m.zz * m.ww;
    result.xy = m.xw * m.zz * m.wy - m.xz * m.zw * m.wy - m.xw * m.zy * m.wz + m.xy * m.zw * m.wz + m.xz * m.zy * m.ww -
                m.xy * m.zz * m.ww;
    result.xz = m.xz * m.yw * m.wy - m.xw * m.yz * m.wy + m.xw * m.yy * m.wz - m.xy * m.yw * m.wz - m.xz * m.yy * m.ww +
                m.xy * m.yz * m.ww;
    result.xw = m.xw * m.yz * m.zy - m.xz * m.yw * m.zy - m.xw * m.yy * m.zz + m.xy * m.yw * m.zz + m.xz * m.yy * m.zw -
                m.xy * m.yz * m.zw;
    result.yx = m.yw * m.zz * m.wx - m.yz * m.zw * m.wx - m.yw * m.zx * m.wz + m.yx * m.zw * m.wz + m.yz * m.zx * m.ww -
                m.yx * m.zz * m.ww;
    result.yy = m.xz * m.zw * m.wx - m.xw * m.zz * m.wx + m.xw * m.zx * m.wz - m.xx * m.zw * m.wz - m.xz * m.zx * m.ww +
                m.xx * m.zz * m.ww;
    result.yz = m.xw * m.yz * m.wx - m.xz * m.yw * m.wx - m.xw * m.yx * m.wz + m.xx * m.yw * m.wz + m.xz * m.yx * m.ww -
                m.xx * m.yz * m.ww;
    result.yw = m.xz * m.yw * m.zx - m.xw * m.yz * m.zx + m.xw * m.yx * m.zz - m.xx * m.yw * m.zz - m.xz * m.yx * m.zw +
                m.xx * m.yz * m.zw;
    result.zx = m.yy * m.zw * m.wx - m.yw * m.zy * m.wx + m.yw * m.zx * m.wy - m.yx * m.zw * m.wy - m.yy * m.zx * m.ww +
                m.yx * m.zy * m.ww;
    result.zy = m.xw * m.zy * m.wx - m.xy * m.zw * m.wx - m.xw * m.zx * m.wy + m.xx * m.zw * m.wy + m.xy * m.zx * m.ww -
                m.xx * m.zy * m.ww;
    result.zz = m.xy * m.yw * m.wx - m.xw * m.yy * m.wx + m.xw * m.yx * m.wy - m.xx * m.yw * m.wy - m.xy * m.yx * m.ww +
                m.xx * m.yy * m.ww;
    result.zw = m.xw * m.yy * m.zx - m.xy * m.yw * m.zx - m.xw * m.yx * m.zy + m.xx * m.yw * m.zy + m.xy * m.yx * m.zw -
                m.xx * m.yy * m.zw;
    result.wx = m.yz * m.zy * m.wx - m.yy * m.zz * m.wx - m.yz * m.zx * m.wy + m.yx * m.zz * m.wy + m.yy * m.zx * m.wz -
                m.yx * m.zy * m.wz;
    result.wy = m.xy * m.zz * m.wx - m.xz * m.zy * m.wx + m.xz * m.zx * m.wy - m.xx * m.zz * m.wy - m.xy * m.zx * m.wz +
                m.xx * m.zy * m.wz;
    result.wz = m.xz * m.yy * m.wx - m.xy * m.yz * m.wx - m.xz * m.yx * m.wy + m.xx * m.yz * m.wy + m.xy * m.yx * m.wz -
                m.xx * m.yy * m.wz;
    result.ww = m.xy * m.yz * m.zx - m.xz * m.yy * m.zx + m.xz * m.yx * m.zy - m.xx * m.yz * m.zy - m.xy * m.yx * m.zz +
                m.xx * m.yy * m.zz;

    float one_over_determinant = 1.0f / cstrl_mat4_determinant(m);
    for (int i = 0; i < 16; i++)
    {
        result.m[i] *= one_over_determinant;
    }
    return result;
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_transpose(mat4 m)
{
    mat4 result;

    result.xx = m.xx;
    result.xy = m.yx;
    result.xz = m.zx;
    result.xw = m.wx;
    result.yx = m.xy;
    result.yy = m.yy;
    result.yz = m.zy;
    result.yw = m.wy;
    result.zx = m.xz;
    result.zy = m.yz;
    result.zz = m.zz;
    result.zw = m.wz;
    result.wx = m.xw;
    result.wy = m.yw;
    result.wz = m.zw;
    result.ww = m.ww;

    return result;
}

// TODO: add to unit tests
CSTRL_INLINE mat4 cstrl_mat4_broken_inverse(mat4 m)
{
    int pivot_row = 0;
    int pivot_column = 0;
    while (pivot_row < 4 && pivot_column < 4)
    {
        int i_max = pivot_row;
        float max = m.m[pivot_row * 4 + pivot_column];
        for (int i = pivot_row + 1; i < 4; i++)
        {
            if (fabs(m.m[i * 4 + pivot_column]) > max)
            {
                max = fabs(m.m[i * 4 + pivot_column]);
                i_max = i;
            }
        }
        if (fabs(m.m[i_max * 4 + pivot_column]) < cstrl_epsilon)
        {
            pivot_column++;
        }
        else
        {
            vec4 tmp_h = (vec4){m.m[pivot_row], m.m[pivot_row + 1], m.m[pivot_row + 2], m.m[pivot_row + 3]};
            vec4 tmp_i_max = (vec4){m.m[i_max], m.m[i_max + 1], m.m[i_max + 2], m.m[i_max + 3]};
            m.m[pivot_row] = tmp_i_max.x;
            m.m[pivot_row + 1] = tmp_i_max.y;
            m.m[pivot_row + 2] = tmp_i_max.z;
            m.m[pivot_row + 3] = tmp_i_max.w;

            m.m[i_max] = tmp_h.x;
            m.m[i_max + 1] = tmp_h.y;
            m.m[i_max + 2] = tmp_h.z;
            m.m[i_max + 3] = tmp_h.w;

            for (int i = pivot_row + 1; i < 4; i++)
            {
                float f = m.m[i * 4 + pivot_column] / m.m[pivot_row * 4 + pivot_column];
                m.m[i * 4 + pivot_column] = 0.0f;
                for (int j = pivot_column + 1; j < 4; j++)
                {
                    m.m[i * 4 + j] -= m.m[pivot_row * 4 + j] * f;
                }
            }

            pivot_row++;
            pivot_column++;
        }
    }

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

CSTRL_INLINE vec3 cstrl_quat_to_euler_angles(quat q)
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

// TODO: add to unit tests
CSTRL_INLINE vec3 cstrl_quat_xyz(quat q)
{
    return (vec3){q.x, q.y, q.z};
}

#endif // CSTRL_MATH_H
