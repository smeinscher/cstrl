#ifndef MODEL_INTERNAL_H
#define MODEL_INTERNAL_H

/*
*
*   Track
*
*/

#include "cstrl/cstrl_math.h"

typedef struct bezier_curve_t
{
    vec3 point1;
    vec3 control1;
    vec3 point2;
    vec3 control2;
} bezier_curve_t;

vec3 cstrl_model_bezier_curve_interpolate(bezier_curve_t curve, float t);

float cstrl_model_hermite_interpolation_float(float p1, float s1, float p2, float s2, float t);

vec3 cstrl_model_hermite_interpolation_vec3(vec3 p1, vec3 s1, vec3 p2, vec3 s2, float t);

quat cstrl_model_hermite_interpolation_quat(quat p1, quat s1, quat p2, quat s2, float t);

typedef CSTRL_PACKED_ENUM{CSTRL_INTERPOLATION_CONSTANT, CSTRL_INTERPOLATION_LINEAR, CSTRL_INTERPOLATION_CUBIC} interpolation_type_t;

typedef CSTRL_PACKED_ENUM{CSTRL_FRAME_TYPE_SCALAR, CSTRL_FRAME_TYPE_VEC3, CSTRL_FRAME_TYPE_QUAT} frame_type_t;

typedef struct scalar_frame_t
{
    float value;
    float in;
    float out;
    float time;
} scalar_frame_t;

typedef struct vec3_frame_t
{
    vec3 value;
    vec3 in;
    vec3 out;
    float time;
} vec3_frame_t;

typedef struct quat_frame_t
{
    quat value;
    quat in;
    quat out;
    float time;
} quat_frame_t;

typedef struct track_t
{
    void *frames;
    int frame_count;
    interpolation_type_t interpolation_type;
    frame_type_t frame_type;
} track_t;

typedef struct transform_track_t
{
    track_t position;
    track_t rotation;
    track_t scale;
    int id;
} transform_track_t;

float cstrl_model_get_frame_time(track_t *track, int frame_index);

float cstrl_model_get_start_time(track_t *track);

float cstrl_model_get_end_time(track_t *track);

int cstrl_model_frame_index(track_t *track, float time, bool looping);

float cstrl_model_adjust_time_to_fit_track(track_t *track, float time, bool looping);

float cstrl_model_sample_scalar(track_t *track, float time, bool looping);

vec3 cstrl_model_sample_vec3(track_t *track, float time, bool looping);

quat cstrl_model_sample_quat(track_t *track, float time, bool looping);

float cstrl_model_transform_track_get_start_time(transform_track_t *transform_track);

float cstrl_model_transform_track_get_end_time(transform_track_t *transform_track);

transform_t cstrl_model_sample_track(transform_track_t *transform_track, transform_t ref, float time, bool looping);

/* 
*
*
*   Pose
*
*/

typedef struct pose_t
{
    transform_t *joints;
    int *parents;
    int count;
} pose_t;

void cstrl_model_pose_init(pose_t *pose, int count);

void cstrl_model_pose_free(pose_t *pose);

transform_t cstrl_model_pose_get_global_transform(pose_t *pose, int joint);

/*
*
*
*   Clip
*
*/

typedef struct clip_t
{
    transform_track_t *tracks;
    int count;
    const char *name;
    float start_time;
    float end_time;
    bool looping;
} clip_t;

#endif // MODEL_INTERNAL_H
