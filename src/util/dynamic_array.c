//
// Created by sterling on 7/28/24.
//

#include "log.c/log.h"

#include "cstrl/cstrl_util.h"

#include <memory.h>
#include <stdint.h>
#include <stdlib.h>

#define GROWTH_FACTOR 2

CSTRL_API void cstrl_da_int_init(da_int *da, size_t initial_size)
{
    da->size = 0;
    da->array = malloc(sizeof(int) * initial_size);
    if (da->array == NULL)
    {
        log_error("Failed to allocate dynamic int array");
        da->capacity = 0;
        return;
    }
    da->capacity = initial_size;
}

CSTRL_API void cstrl_da_float_init(da_float *da, size_t initial_size)
{
    da->size = 0;
    da->array = malloc(sizeof(float) * initial_size);
    if (da->array == NULL)
    {
        log_error("Failed to allocate dynamic float array");
        da->capacity = 0;
        return;
    }
    da->capacity = initial_size;
}

CSTRL_API void cstrl_string_init(string *str, size_t initial_size)
{
    str->size = 0;
    str->array = malloc(sizeof(char) * initial_size);
    if (str->array == NULL)
    {
        log_error("Failed to allocate dynamic char array (string)");
        str->capacity = 0;
        return;
    }
    str->capacity = initial_size;
}

CSTRL_API void cstrl_da_string_init(da_string *da, size_t initial_size)
{
    da->size = 0;
    da->array = malloc(sizeof(string) * initial_size);
    if (da->array == NULL)
    {
        log_error("Failed to allocate dynamic string array");
        da->capacity = 0;
        return;
    }
    for (int i = 0; i < initial_size; i++)
    {
        da->array[i].array = NULL;
    }
    da->capacity = initial_size;
}

CSTRL_API bool cstrl_da_int_reserve(da_int *da, size_t new_capacity)
{
    if (da == NULL || da->array == NULL)
    {
        log_error("Invalid dynamic array structure");
        return false;
    }
    if (new_capacity >= SIZE_MAX / sizeof(int))
    {
        log_error("New capacity is too large");
        return false;
    }
    int *temp = realloc(da->array, new_capacity * sizeof(int));
    if (temp == NULL)
    {
        log_error("Failed to allocate memory for dynamic int array");
        return false;
    }
    da->array = temp;
    da->capacity = new_capacity;
    return true;
}

CSTRL_API bool cstrl_da_float_reserve(da_float *da, size_t new_capacity)
{
    if (da == NULL || da->array == NULL)
    {
        log_error("Invalid dynamic array structure");
        return false;
    }
    if (new_capacity >= SIZE_MAX / sizeof(float))
    {
        log_error("New capacity is too large");
        return false;
    }
    float *temp = realloc(da->array, new_capacity * sizeof(float));
    if (temp == NULL)
    {
        log_error("Failed to allocate memory for dynamic float array");
        return false;
    }
    da->array = temp;
    da->capacity = new_capacity;
    return true;
}

CSTRL_API bool cstrl_string_reserve(string *str, size_t new_capacity)
{
    if (str == NULL || str->array == NULL)
    {
        log_error("Invalid dynamic array structure");
        return false;
    }
    if (new_capacity >= SIZE_MAX / sizeof(char))
    {
        log_error("New capacity is too large");
        return false;
    }
    char *temp = realloc(str->array, new_capacity * sizeof(char));
    if (temp == NULL)
    {
        log_error("Failed to allocate memory for dynamic char array (string)");
        return false;
    }
    str->array = temp;
    str->capacity = new_capacity;
    return true;
}

CSTRL_API bool cstrl_da_string_reserve(da_string *da, size_t new_capacity)
{
    if (da == NULL || da->array == NULL)
    {
        log_error("Invalid dynamic array structure");
        return false;
    }
    if (new_capacity >= SIZE_MAX / sizeof(string))
    {
        log_error("New capacity is too large");
        return false;
    }
    string *temp = realloc(da->array, new_capacity * sizeof(string));
    if (temp == NULL)
    {
        log_error("Failed to allocate memory for dynamic string array");
        return false;
    }
    da->array = temp;
    da->capacity = new_capacity;
    return true;
}

CSTRL_API void cstrl_da_int_push_back(da_int *da, int element)
{
    if (da->size >= da->capacity && !cstrl_da_int_reserve(da, da->capacity * GROWTH_FACTOR))
    {
        return;
    }
    da->array[da->size++] = element;
}

CSTRL_API void cstrl_da_float_push_back(da_float *da, float element)
{
    if (da->size >= da->capacity && !cstrl_da_float_reserve(da, da->capacity * GROWTH_FACTOR))
    {
        return;
    }
    da->array[da->size++] = element;
}

CSTRL_API void cstrl_string_push_back(string *str, const char *characters, size_t length)
{
    for (int i = 0; i < length; i++)
    {
        // TODO: find a way to make less calls to string_reserve if length of new string is large
        if (str->size >= str->capacity && !cstrl_string_reserve(str, str->capacity * GROWTH_FACTOR))
        {
            return;
        }
        str->array[str->size++] = characters[i];
    }
}

CSTRL_API void cstrl_da_string_push_back(da_string *da, string element)
{
    if (da->size >= da->capacity && !cstrl_da_string_reserve(da, da->capacity * GROWTH_FACTOR))
    {
        return;
    }
    cstrl_string_init(&da->array[da->size], element.capacity);
    cstrl_string_push_back(&da->array[da->size], element.array, element.size);
    da->size++;
}

// TODO: unit test
CSTRL_API void cstrl_da_int_insert(da_int *da, int element, size_t index)
{
    if ((index >= da->capacity || da->size >= da->capacity) && !cstrl_da_int_reserve(da, da->capacity * GROWTH_FACTOR))
    {
        log_error("Failed to insert into dynamic int array");
        return;
    }
    do
    {
        int temp = da->array[index];
        da->array[index] = element;
        element = temp;
        index++;
    } while (index <= da->size);
    da->size++;
}

CSTRL_API void cstrl_da_float_insert(da_float *da, float element, size_t index)
{
    log_fatal("da_float_insert not implemented");
}

CSTRL_API void cstrl_string_insert(string *str, const char *element, size_t length, size_t index)
{
    log_fatal("string_insert not implemented");
}

CSTRL_API void cstrl_da_string_insert(da_string *da, string element, size_t index)
{
    log_fatal("da_string_insert not implemented");
}

CSTRL_API void cstrl_da_int_remove(da_int *da, int index)
{
    if (index >= da->size || index < 0)
    {
        log_error("Dynamic int array remove index out of bounds");
        return;
    }
    int temp_index = 0;
    for (int i = 0; i < da->size; i++)
    {
        if (i == index)
        {
            continue;
        }
        da->array[temp_index] = da->array[i];
        temp_index++;
    }
    da->size--;
}

CSTRL_API void cstrl_da_float_remove(da_float *da, int index)
{
    if (index >= da->size)
    {
        log_error("Dynamic float array remove index out of bounds");
        return;
    }
    int temp_index = 0;
    for (int i = 0; i < da->size; i++)
    {
        if (i == index)
        {
            continue;
        }
        da->array[temp_index] = da->array[i];
        temp_index++;
    }
    da->size--;
}

CSTRL_API void cstrl_string_remove(string *str, int index)
{
    log_fatal("string_remove not implemented");
}

CSTRL_API void cstrl_da_string_remove(da_string *da, int index)
{
    if (index >= da->size)
    {
        log_error("Dynamic string array remove index out of bounds");
        return;
    }
    int temp_index = 0;
    for (int i = 0; i < da->size; i++)
    {
        if (i == index)
        {
            cstrl_string_free(&da->array[i]);
            continue;
        }
        da->array[temp_index] = da->array[i];
        temp_index++;
    }
    da->size--;
}

CSTRL_API int cstrl_da_int_find_first(da_int *da, int value)
{
    for (int i = 0; i < da->size; i++)
    {
        if (da->array[i] == value)
        {
            return i;
        }
    }

    return -1;
}

CSTRL_API int cstrl_da_int_pop_back(da_int *da)
{
    da->size--;
    return da->array[da->size];
}

CSTRL_API float cstrl_da_float_pop_back(da_float *da)
{
    da->size--;
    return da->array[da->size];
}

CSTRL_API char cstrl_string_pop_back(string *str)
{
    str->size--;
    return str->array[str->size];
}

CSTRL_API string cstrl_da_string_pop_back(da_string *da)
{
    da->size--;
    return da->array[da->size];
}

CSTRL_API int cstrl_da_int_pop_front(da_int *da)
{
    int temp = da->array[0];
    cstrl_da_int_remove(da, 0);
    return temp;
}

CSTRL_API float cstrl_da_float_pop_front(da_float *da)
{
    float temp = da->array[0];
    cstrl_da_float_remove(da, 0);
    return temp;
}

CSTRL_API string cstrl_da_string_pop_front(da_string *da)
{
    string temp = da->array[0];
    cstrl_da_string_remove(da, 0);
    return temp;
}

CSTRL_API void cstrl_da_int_clear(da_int *da)
{
    memset(da->array, 0, sizeof(int) * da->size);
    da->size = 0;
}

CSTRL_API void cstrl_da_float_clear(da_float *da)
{
    memset(da->array, 0, sizeof(float) * da->size);
    da->size = 0;
}

CSTRL_API void cstrl_string_clear(string *str)
{
    memset(str->array, 0, sizeof(char) * str->size);
    str->size = 0;
}

CSTRL_API void cstrl_da_string_clear(da_string *da)
{
    if (da->size == 0)
    {
        return;
    }
    for (int i = 0; i < da->size; i++)
    {
        cstrl_string_free(&da->array[i]);
        da->array[i].array = NULL;
    }
    da->size = 0;
}

CSTRL_API void cstrl_da_int_free(da_int *da)
{
    if (da == NULL)
    {
        return;
    }
    free(da->array);
    da->array = NULL;
    da->size = 0;
    da->capacity = 0;
}

CSTRL_API void cstrl_da_float_free(da_float *da)
{
    free(da->array);
    da->array = NULL;
    da->size = 0;
    da->capacity = 0;
}

CSTRL_API void cstrl_string_free(string *str)
{
    free(str->array);
    str->array = NULL;
    str->size = 0;
    str->capacity = 0;
}

CSTRL_API void cstrl_da_string_free(da_string *da)
{
    for (int i = 0; i < da->size; i++)
    {
        cstrl_string_free(&da->array[i]);
    }
    free(da->array);
    da->size = 0;
    da->capacity = 0;
}

CSTRL_API char *cstrl_string_to_c_str(string *str)
{
    char *temp = malloc(sizeof(char) * str->size + 1);
    for (int i = 0; i < str->size; i++)
    {
        temp[i] = str->array[i];
    }
    temp[str->size] = '\0';
    return temp;
}

static int cstrl_da_int_quick_sort_partition_internal(da_int *da, int left, int right, bool descending)
{
    int pivot = da->array[right];

    int index = left - 1;

    for (int i = left; i < right; i++)
    {
        if (!descending && da->array[i] > pivot || descending && da->array[i] < pivot)
        {
            index++;
            int temp = da->array[index];
            da->array[index] = da->array[i];
            da->array[i] = temp;
        }
    }

    int temp = da->array[index + 1];
    da->array[index + 1] = da->array[right];
    da->array[right] = temp;

    return index + 1;
}

static void cstrl_da_int_quick_sort_internal(da_int *da, int left, int right, bool descending)
{
    if (right <= left)
    {
        return;
    }
    int pivot = cstrl_da_int_quick_sort_partition_internal(da, left, right, descending);
    cstrl_da_int_quick_sort_internal(da, left, pivot - 1, descending);
    cstrl_da_int_quick_sort_internal(da, pivot + 1, right, descending);
}

CSTRL_API void cstrl_da_int_quick_sort(da_int *da, bool descending)
{
    cstrl_da_int_quick_sort_internal(da, 0, da->size - 1, descending);
}
