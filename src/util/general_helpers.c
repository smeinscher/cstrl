//
// Created by sterling on 7/29/24.
//

#include "cstrl/cstrl_util.h"
#include <malloc.h>
#include <stdio.h>

CSTRL_API bool cstrl_realloc_int(int **int_ptr, size_t size)
{
    int *temp_int_ptr = realloc(*int_ptr, size * sizeof(int));
    if (temp_int_ptr == NULL)
    {
        printf("Error reallocating memory for int_ptr\n");
        return false;
    }
    *int_ptr = temp_int_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_float(float **float_ptr, size_t size)
{
    float *temp_float_ptr = realloc(*float_ptr, size * sizeof(float));
    if (temp_float_ptr == NULL)
    {
        printf("Error reallocating memory for float_ptr\n");
        return false;
    }
    *float_ptr = temp_float_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_string(string **str_ptr, size_t size)
{
    string *temp_str_ptr = realloc(*str_ptr, size * sizeof(string));
    if (temp_str_ptr == NULL)
    {
        printf("Error reallocating memory for str_ptr\n");
        return false;
    }
    *str_ptr = temp_str_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_transform(transform **trans_ptr, size_t size)
{
    transform *temp_trans_ptr = realloc(*trans_ptr, size * sizeof(transform));
    if (temp_trans_ptr == NULL)
    {
        printf("Error reallocating memory for trans_ptr\n");
        return false;
    }
    *trans_ptr = temp_trans_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_vec3(vec3 **vec3_ptr, size_t size)
{
    vec3 *temp_vec3_ptr = realloc(*vec3_ptr, size * sizeof(vec3));
    if (temp_vec3_ptr == NULL)
    {
        printf("Error reallocating memory for vec3_ptr\n");
        return false;
    }
    *vec3_ptr = temp_vec3_ptr;
    return true;
}

CSTRL_API bool cstrl_realloc_bool(bool **bool_ptr, size_t size)
{
    bool *temp_bool_ptr = realloc(*bool_ptr, size * sizeof(bool));
    if (temp_bool_ptr == NULL)
    {
        printf("Error reallocating memory for bool_ptr\n");
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
        printf("Error reallocating memory for da_int_ptr\n");
        return false;
    }
    return true;
}
