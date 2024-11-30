//
// Created by sterling on 7/29/24.
//

#ifndef OMEGAAPP_GENERAL_HELPERS_H
#define OMEGAAPP_GENERAL_HELPERS_H

#include "cstrl/cstrl_util.h"
#include <stdbool.h>
#include <stddef.h>

void realloc_int(int **int_ptr, size_t size);

void realloc_float(float **float_ptr, size_t size);

void realloc_string(string **str_ptr, size_t size);

#endif // OMEGAAPP_GENERAL_HELPERS_H
