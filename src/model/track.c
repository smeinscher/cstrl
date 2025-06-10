#include "cstrl/cstrl_math.h"
#include "log.c/log.h"
#include "model_internal.h"

vec3 cstrl_model_bezier_curve_interpolate(bezier_curve_t curve, float t)
{
    vec3 a = cstrl_vec3_lerp(curve.point1, curve.control1, t);
    vec3 b = cstrl_vec3_lerp(curve.control2, curve.point2, t);
    vec3 c = cstrl_vec3_lerp(curve.control1, curve.control2, t);
    vec3 d = cstrl_vec3_lerp(a, c, t);
    vec3 e = cstrl_vec3_lerp(c, b, t);
    vec3 r = cstrl_vec3_lerp(d, e, t);

    return r;
}

float cstrl_model_hermite_interpolation_float(float p1, float s1, float p2, float s2, float t)
{
    return p1 * ((1.0f + 2.0f * t) * ((1.0f - t) * (1.0f - t))) + s1 * (t * ((1.0f - t) * (1.0f - t))) +
           p2 * ((t * t) * (3.0f - 2.0f * t)) + s2 * ((t * t) * (t - 1.0f));
}

vec3 cstrl_model_hermite_interpolation_vec3(vec3 p1, vec3 s1, vec3 p2, vec3 s2, float t)
{
    float a_scalar_part = (1.0f + 2.0f * t) * ((1.0f - t) * (1.0f - t));
    vec3 a = cstrl_vec3_mult_scalar(p1, a_scalar_part);
    float b_scalar_part = t * ((1.0f - t) * (1.0f - t));
    vec3 b = cstrl_vec3_mult_scalar(s1, b_scalar_part);
    float c_scalar_part = (t * t) * (3.0f - 2.0f * t);
    vec3 c = cstrl_vec3_mult_scalar(p2, c_scalar_part);
    float d_scalar_part = (t * t) * (t - 1.0f);
    vec3 d = cstrl_vec3_mult_scalar(s2, d_scalar_part);

    vec3 ab = cstrl_vec3_add(a, b);
    vec3 abc = cstrl_vec3_add(ab, c);

    return cstrl_vec3_add(abc, d);
}

quat cstrl_model_hermite_interpolation_quat(quat p1, quat s1, quat p2, quat s2, float t)
{
    float a_scalar_part = (1.0f + 2.0f * t) * ((1.0f - t) * (1.0f - t));
    quat a = cstrl_quat_mult_scalar(p1, a_scalar_part);
    float b_scalar_part = t * ((1.0f - t) * (1.0f - t));
    quat b = cstrl_quat_mult_scalar(s1, b_scalar_part);
    float c_scalar_part = (t * t) * (3.0f - 2.0f * t);
    quat c = cstrl_quat_mult_scalar(p2, c_scalar_part);
    float d_scalar_part = (t * t) * (t - 1.0f);
    quat d = cstrl_quat_mult_scalar(s2, d_scalar_part);

    quat ab = cstrl_quat_add(a, b);
    quat abc = cstrl_quat_add(ab, c);

    return cstrl_quat_add(abc, d);
}

float cstrl_model_get_frame_time(track_t *track, int frame_index)
{
    if (frame_index >= 0 && frame_index < track->frame_count)
    {
        switch (track->frame_type)
        {
        case CSTRL_FRAME_TYPE_SCALAR: {
            scalar_frame_t *frame = (scalar_frame_t *)&track->frames[frame_index];
            return frame->time;
        }
        case CSTRL_FRAME_TYPE_VEC3: {
            vec3_frame_t *frame = (vec3_frame_t *)&track->frames[frame_index];
            return frame->time;
        }
        case CSTRL_FRAME_TYPE_QUAT: {
            quat_frame_t *frame = (quat_frame_t *)&track->frames[frame_index];
            return frame->time;
        }
        }
    }
    return 0.0f;
}

float cstrl_model_get_start_time(track_t *track)
{
    return cstrl_model_get_frame_time(track, 0);
}

float cstrl_model_get_end_time(track_t *track)
{
    return cstrl_model_get_frame_time(track, track->frame_count - 1);
}

int cstrl_model_frame_index(track_t *track, float time, bool looping)
{
    if (track->frame_count <= 1)
    {
        return -1;
    }

    if (looping)
    {
        float start_time = cstrl_model_get_start_time(track);
        float end_time = cstrl_model_get_end_time(track);
        float duration = end_time - start_time;

        if (duration < 0.0f)
        {
            log_warn("Frame has negative duration");
            duration = fabsf(duration);
        }

        time = fmodf(time - start_time, end_time - start_time);
        if (time < 0.0f)
        {
            time += end_time - start_time;
        }
        time += start_time;
    }
    else
    {
        if (time <= cstrl_model_get_start_time(track))
        {
            return 0;
        }
        if (time >= cstrl_model_get_frame_time(track, track->frame_count - 2))
        {
            return track->frame_count - 2;
        }
    }

    for (int i = track->frame_count; i >= 0; i--)
    {
        if (time >= cstrl_model_get_frame_time(track, i))
        {
            return i;
        }
    }

    return -1;
}

float cstrl_model_adjust_time_to_fit_track(track_t *track, float time, bool looping)
{
    if (track->frame_count <= 1)
    {
        return 0.0f;
    }

    float start_time = cstrl_model_get_start_time(track);
    float end_time = cstrl_model_get_end_time(track);
    float duration = end_time - start_time;
    if (duration <= 0.0f)
    {
        return 0.0f;
    }

    if (looping)
    {
        time = fmodf(time - start_time, end_time - start_time);
        if (time < 0.0f)
        {
            time += end_time - start_time;
        }
        time += start_time;
    }
    else
    {
        if (time < start_time)
        {
            time = start_time;
        }
        else if (time > end_time)
        {
            time = end_time;
        }
    }

    return time;
}

float cstrl_model_sample_scalar(track_t *track, float time, bool looping)
{
    switch (track->interpolation_type)
    {
    case CSTRL_INTERPOLATION_CONSTANT: {
        int frame_index = cstrl_model_frame_index(track, time, looping);
        if (frame_index < 0 || frame_index >= track->frame_count)
        {
            return 0.0f;
        }
        scalar_frame_t *frame = (scalar_frame_t *)&track->frames[frame_index];
        return frame->value;
    }
    case CSTRL_INTERPOLATION_LINEAR: {
        int frame_index = cstrl_model_frame_index(track, time, looping);
        if (frame_index < 0 || frame_index >= track->frame_count)
        {
            return 0.0f;
        }

        float track_time = cstrl_model_adjust_time_to_fit_track(track, time, looping);
        float frame_delta =
            cstrl_model_get_frame_time(track, frame_index + 1) - cstrl_model_get_frame_time(track, frame_index);
        if (frame_delta <= 0.0f)
        {
            return 0.0f;
        }

        float t = (track_time - cstrl_model_get_frame_time(track, frame_index)) / frame_delta;

        scalar_frame_t *frame_start = (scalar_frame_t *)&track->frames[frame_index];
        scalar_frame_t *frame_end = (scalar_frame_t *)&track->frames[frame_index + 1];

        return cstrl_scalar_interpolation(frame_start->value, frame_end->value, t);
    }
    case CSTRL_INTERPOLATION_CUBIC: {
        int frame_index = cstrl_model_frame_index(track, time, looping);
        if (frame_index < 0 || frame_index >= track->frame_count)
        {
            return 0.0f;
        }

        float track_time = cstrl_model_adjust_time_to_fit_track(track, time, looping);
        float frame_delta =
            cstrl_model_get_frame_time(track, frame_index + 1) - cstrl_model_get_frame_time(track, frame_index);
        if (frame_delta <= 0.0f)
        {
            return 0.0f;
        }

        float t = (track_time - cstrl_model_get_frame_time(track, frame_index)) / frame_delta;

        scalar_frame_t *frame_start = (scalar_frame_t *)&track->frames[frame_index];
        scalar_frame_t *frame_end = (scalar_frame_t *)&track->frames[frame_index + 1];

        float p1 = frame_start->value;
        float s1 = frame_start->out;
        s1 *= frame_delta;

        float p2 = frame_end->value;
        float s2 = frame_end->in;
        s2 *= frame_delta;

        return cstrl_model_hermite_interpolation_float(p1, s1, p2, s2, t);
    }
    }
    // Should be unreachable
    log_warn("Invalid interpolation type for track");
    return 0.0;
}

vec3 cstrl_model_sample_vec3(track_t *track, float time, bool looping)
{
    switch (track->interpolation_type)
    {
    case CSTRL_INTERPOLATION_CONSTANT: {
        int frame_index = cstrl_model_frame_index(track, time, looping);
        if (frame_index < 0 || frame_index >= track->frame_count)
        {
            return (vec3){0};
        }
        vec3_frame_t *frame = (vec3_frame_t *)&track->frames[frame_index];
        return frame->value;
    }
    case CSTRL_INTERPOLATION_LINEAR: {
        int frame_index = cstrl_model_frame_index(track, time, looping);
        if (frame_index < 0 || frame_index >= track->frame_count)
        {
            return (vec3){0};
        }

        float track_time = cstrl_model_adjust_time_to_fit_track(track, time, looping);
        float frame_delta =
            cstrl_model_get_frame_time(track, frame_index + 1) - cstrl_model_get_frame_time(track, frame_index);
        if (frame_delta <= 0.0f)
        {
            return (vec3){0};
        }

        float t = (track_time - cstrl_model_get_frame_time(track, frame_index)) / frame_delta;

        vec3_frame_t *frame_start = (vec3_frame_t *)&track->frames[frame_index];
        vec3_frame_t *frame_end = (vec3_frame_t *)&track->frames[frame_index + 1];

        return cstrl_vec3_lerp(frame_start->value, frame_end->value, t);
    }
    case CSTRL_INTERPOLATION_CUBIC: {
        int frame_index = cstrl_model_frame_index(track, time, looping);
        if (frame_index < 0 || frame_index >= track->frame_count)
        {
            return (vec3){0};
        }

        float track_time = cstrl_model_adjust_time_to_fit_track(track, time, looping);
        float frame_delta =
            cstrl_model_get_frame_time(track, frame_index + 1) - cstrl_model_get_frame_time(track, frame_index);
        if (frame_delta <= 0.0f)
        {
            return (vec3){0};
        }

        float t = (track_time - cstrl_model_get_frame_time(track, frame_index)) / frame_delta;

        vec3_frame_t *frame_start = (vec3_frame_t *)&track->frames[frame_index];
        vec3_frame_t *frame_end = (vec3_frame_t *)&track->frames[frame_index + 1];

        vec3 p1 = frame_start->value;
        vec3 s1 = frame_start->out;
        cstrl_vec3_mult_scalar(s1, frame_delta);

        vec3 p2 = frame_end->value;
        vec3 s2 = frame_end->in;
        cstrl_vec3_mult_scalar(s2, frame_delta);

        return cstrl_model_hermite_interpolation_vec3(p1, s1, p2, s2, t);
    }
    }
    // Should be unreachable
    log_warn("Invalid interpolation type for track");
    return (vec3){0};
}

quat cstrl_model_sample_quat(track_t *track, float time, bool looping)
{
    switch (track->interpolation_type)
    {
    case CSTRL_INTERPOLATION_CONSTANT: {
        int frame_index = cstrl_model_frame_index(track, time, looping);
        if (frame_index < 0 || frame_index >= track->frame_count)
        {
            return (quat){0};
        }
        quat_frame_t *frame = (quat_frame_t *)&track->frames[frame_index];
        return frame->value;
    }
    case CSTRL_INTERPOLATION_LINEAR: {
        int frame_index = cstrl_model_frame_index(track, time, looping);
        if (frame_index < 0 || frame_index >= track->frame_count)
        {
            return (quat){0};
        }

        float track_time = cstrl_model_adjust_time_to_fit_track(track, time, looping);
        float frame_delta =
            cstrl_model_get_frame_time(track, frame_index + 1) - cstrl_model_get_frame_time(track, frame_index);
        if (frame_delta <= 0.0f)
        {
            return (quat){0};
        }

        float t = (track_time - cstrl_model_get_frame_time(track, frame_index)) / frame_delta;

        quat_frame_t *frame_start = (quat_frame_t *)&track->frames[frame_index];
        quat_frame_t *frame_end = (quat_frame_t *)&track->frames[frame_index + 1];

        return cstrl_quat_interpolate(frame_start->value, frame_end->value, t);
    }
    case CSTRL_INTERPOLATION_CUBIC: {
        int frame_index = cstrl_model_frame_index(track, time, looping);
        if (frame_index < 0 || frame_index >= track->frame_count)
        {
            return (quat){0};
        }

        float track_time = cstrl_model_adjust_time_to_fit_track(track, time, looping);
        float frame_delta =
            cstrl_model_get_frame_time(track, frame_index + 1) - cstrl_model_get_frame_time(track, frame_index);
        if (frame_delta <= 0.0f)
        {
            return (quat){0};
        }

        float t = (track_time - cstrl_model_get_frame_time(track, frame_index)) / frame_delta;

        quat_frame_t *frame_start = (quat_frame_t *)&track->frames[frame_index];
        quat_frame_t *frame_end = (quat_frame_t *)&track->frames[frame_index + 1];

        quat p1 = frame_start->value;
        quat s1 = frame_start->out;
        cstrl_quat_mult_scalar(s1, frame_delta);

        quat p2 = frame_end->value;
        quat s2 = frame_end->in;
        cstrl_quat_mult_scalar(s2, frame_delta);

        return cstrl_model_hermite_interpolation_quat(p1, s1, p2, s2, t);
    }
    }
    // Should be unreachable
    log_warn("Invalid interpolation type for track");
    return (quat){0};
}

float cstrl_model_transform_track_get_start_time(transform_track_t *transform_track)
{
    float result = 0.0f;
    bool is_set = false;

    if (transform_track->position.frame_count > 1)
    {
        result = cstrl_model_get_start_time(&transform_track->position);
        is_set = true;
    }
    if (transform_track->rotation.frame_count > 1)
    {
        float start = cstrl_model_get_start_time(&transform_track->rotation);
        if (start < result || !is_set)
        {
            result = start;
            is_set = true;
        }
    }
    if (transform_track->scale.frame_count > 1)
    {
        float start = cstrl_model_get_start_time(&transform_track->scale);
        if (start < result || !is_set)
        {
            result = start;
        }
    }

    return result;
}

float cstrl_model_transform_track_get_end_time(transform_track_t *transform_track)
{
    float result = 0.0f;
    bool is_set = false;

    if (transform_track->position.frame_count > 1)
    {
        result = cstrl_model_get_end_time(&transform_track->position);
        is_set = true;
    }
    if (transform_track->rotation.frame_count > 1)
    {
        float start = cstrl_model_get_end_time(&transform_track->rotation);
        if (start > result || !is_set)
        {
            result = start;
            is_set = true;
        }
    }
    if (transform_track->scale.frame_count > 1)
    {
        float start = cstrl_model_get_end_time(&transform_track->scale);
        if (start > result || !is_set)
        {
            result = start;
        }
    }

    return result;
}

transform_t cstrl_model_sample_track(transform_track_t *transform_track, transform_t ref, float time, bool looping)
{
    transform_t result = ref;
    if (transform_track->position.frame_count > 1)
    {
        result.position = cstrl_model_sample_vec3(&transform_track->position, time, looping);
    }
    if (transform_track->rotation.frame_count > 1)
    {
        result.rotation = cstrl_model_sample_quat(&transform_track->rotation, time, looping);
    }
    if (transform_track->scale.frame_count > 1)
    {
        result.scale = cstrl_model_sample_vec3(&transform_track->scale, time, looping);
    }

    return result;
}
