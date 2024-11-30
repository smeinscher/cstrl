//
// Created by sterling on 7/29/24.
//

#include "general_helpers.h"
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Coords
{
    int q;
    int r;
} Coords;

typedef struct Node
{
    Coords position;
    struct Node *parent;
    struct Node *neighbors[6];
    int g;
    int h;
    int f;
} Node;

static int g_directions[6][2] = {{0, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 0}, {1, -1}};

void realloc_int(int **int_ptr, size_t size)
{
    int *temp_int_ptr = realloc(*int_ptr, size * sizeof(int));
    if (temp_int_ptr == NULL)
    {
        printf("Error reallocating memory for int_ptr\n");
        return;
    }
    *int_ptr = temp_int_ptr;
}

void realloc_float(float **float_ptr, size_t size)
{
    float *temp_float_ptr = realloc(*float_ptr, size * sizeof(float));
    if (temp_float_ptr == NULL)
    {
        printf("Error reallocating memory for float_ptr\n");
        return;
    }
    *float_ptr = temp_float_ptr;
}

void realloc_string(string **str_ptr, size_t size)
{
    string *temp_str_ptr = realloc(*str_ptr, size * sizeof(string));
    if (temp_str_ptr == NULL)
    {
        printf("Error reallocating memory for str_ptr\n");
        return;
    }
    *str_ptr = temp_str_ptr;
}
