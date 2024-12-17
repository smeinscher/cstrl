//
// Created by sterling on 7/29/24.
//

#include "cstrl/cstrl_util.h"
#include <malloc.h>
#include <stdio.h>

CSTRL_API void cstrl_realloc_int(int **int_ptr, size_t size)
{
    int *temp_int_ptr = realloc(*int_ptr, size * sizeof(int));
    if (temp_int_ptr == NULL)
    {
        printf("Error reallocating memory for int_ptr\n");
        return;
    }
    *int_ptr = temp_int_ptr;
}

CSTRL_API void cstrl_realloc_float(float **float_ptr, size_t size)
{
    float *temp_float_ptr = realloc(*float_ptr, size * sizeof(float));
    if (temp_float_ptr == NULL)
    {
        printf("Error reallocating memory for float_ptr\n");
        return;
    }
    *float_ptr = temp_float_ptr;
}

CSTRL_API void cstrl_realloc_string(string **str_ptr, size_t size)
{
    string *temp_str_ptr = realloc(*str_ptr, size * sizeof(string));
    if (temp_str_ptr == NULL)
    {
        printf("Error reallocating memory for str_ptr\n");
        return;
    }
    *str_ptr = temp_str_ptr;
}
