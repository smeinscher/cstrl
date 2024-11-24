//
// Created by 12105 on 11/23/2024.
//

#ifndef CSTRL_TYPES_H
#define CSTRL_TYPES_H

#include <stddef.h>

typedef struct da_int
{
    int *array;
    size_t size;
    size_t capacity;
} da_int;

typedef struct da_float
{
    float *array;
    size_t size;
    size_t capacity;
} da_float;

typedef struct da_char
{
    char *array;
    size_t size;
    size_t capacity;
} da_char;

typedef da_char string;

typedef struct da_string
{
    string *array;
    size_t size;
    size_t capacity;
} da_string;

#endif // CSTRL_TYPES_H
