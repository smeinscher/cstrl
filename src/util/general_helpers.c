//
// Created by sterling on 7/29/24.
//

#include "cstrl/cstrl_util.h"
#include <log.c/log.h>
#include <malloc.h>
#include <stdio.h>

CSTRL_API bool cstrl_realloc_int(int **int_ptr, size_t size)
{
    int *temp_int_ptr = realloc(*int_ptr, size * sizeof(int));
    if (temp_int_ptr == NULL)
    {
        log_error("Error reallocating memory for int_ptr");
        return false;
    }
    *int_ptr = temp_int_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_unsigned_int(unsigned int **uint_ptr, size_t size)
{
    unsigned int *temp_uint_ptr = realloc(*uint_ptr, size * sizeof(unsigned int));
    if (temp_uint_ptr == NULL)
    {
        log_error("Error reallocating memory for int_ptr");
        return false;
    }
    *uint_ptr = temp_uint_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_float(float **float_ptr, size_t size)
{
    float *temp_float_ptr = realloc(*float_ptr, size * sizeof(float));
    if (temp_float_ptr == NULL)
    {
        log_error("Error reallocating memory for float_ptr");
        return false;
    }
    *float_ptr = temp_float_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_double(double **double_ptr, size_t size)
{
    double *temp_double_ptr = realloc(*double_ptr, size * sizeof(double));
    if (temp_double_ptr == NULL)
    {
        log_error("Error reallocating memory for double_ptr");
        return false;
    }
    *double_ptr = temp_double_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_string(string **str_ptr, size_t size)
{
    string *temp_str_ptr = realloc(*str_ptr, size * sizeof(string));
    if (temp_str_ptr == NULL)
    {
        log_error("Error reallocating memory for str_ptr");
        return false;
    }
    *str_ptr = temp_str_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_transform(transform_t **trans_ptr, size_t size)
{
    transform_t *temp_trans_ptr = realloc(*trans_ptr, size * sizeof(transform_t));
    if (temp_trans_ptr == NULL)
    {
        log_error("Error reallocating memory for trans_ptr");
        return false;
    }
    *trans_ptr = temp_trans_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_vec2(vec2 **vec2_ptr, size_t size)
{
    vec2 *temp_vec2_ptr = realloc(*vec2_ptr, size * sizeof(vec2));
    if (temp_vec2_ptr == NULL)
    {
        log_error("Error reallocating memory for vec2_ptr");
        return false;
    }
    *vec2_ptr = temp_vec2_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_vec3(vec3 **vec3_ptr, size_t size)
{
    vec3 *temp_vec3_ptr = realloc(*vec3_ptr, size * sizeof(vec3));
    if (temp_vec3_ptr == NULL)
    {
        log_error("Error reallocating memory for vec3_ptr");
        return false;
    }
    *vec3_ptr = temp_vec3_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_vec4(vec4 **vec4_ptr, size_t size)
{
    vec4 *temp_vec4_ptr = realloc(*vec4_ptr, size * sizeof(vec4));
    if (temp_vec4_ptr == NULL)
    {
        log_error("Error reallocating memory for vec4_ptr");
        return false;
    }
    *vec4_ptr = temp_vec4_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_bool(bool **bool_ptr, size_t size)
{
    bool *temp_bool_ptr = realloc(*bool_ptr, size * sizeof(bool));
    if (temp_bool_ptr == NULL)
    {
        log_error("Error reallocating memory for bool_ptr");
        return false;
    }
    *bool_ptr = temp_bool_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_da_int(da_int **da_int_ptr, size_t size)
{
    da_int *temp_da_int_ptr = realloc(*da_int_ptr, size * sizeof(da_int));
    if (temp_da_int_ptr == NULL)
    {
        log_error("Error reallocating memory for da_int_ptr");
        return false;
    }
    *da_int_ptr = temp_da_int_ptr;
    return true;
}
