//
// Created by 12105 on 11/23/2024.
//

#ifndef CSTRL_UTIL_H
#define CSTRL_UTIL_H

#include "cstrl/cstrl_math.h"
#include "cstrl_defines.h"
#include "cstrl_types.h"

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

CSTRL_API void cstrl_da_int_init(da_int *da, size_t initial_size);

CSTRL_API void cstrl_da_float_init(da_float *da, size_t initial_size);

CSTRL_API void cstrl_string_init(string *str, size_t initial_size);

CSTRL_API void cstrl_da_string_init(da_string *da, size_t initial_size);

CSTRL_API bool cstrl_da_int_reserve(da_int *da, size_t new_capacity);

CSTRL_API bool cstrl_da_float_reserve(da_float *da, size_t new_capacity);

CSTRL_API bool cstrl_string_reserve(string *str, size_t new_capacity);

CSTRL_API bool cstrl_da_string_reserve(da_string *da, size_t new_capacity);

CSTRL_API void cstrl_da_int_push_back(da_int *da, int element);

CSTRL_API void cstrl_da_float_push_back(da_float *da, float element);

CSTRL_API void cstrl_string_push_back(string *str, const char *characters, size_t length);

CSTRL_API void cstrl_da_string_push_back(da_string *da, string element);

CSTRL_API void cstrl_da_int_insert(da_int *da, int element, size_t index);

CSTRL_API void cstrl_da_float_insert(da_float *da, float element, size_t index);

CSTRL_API void cstrl_string_insert(string *str, const char *element, size_t length, size_t index);

CSTRL_API void cstrl_da_string_insert(da_string *da, string element, size_t index);

CSTRL_API void cstrl_da_int_remove(da_int *da, int index);

CSTRL_API void cstrl_da_float_remove(da_float *da, int index);

CSTRL_API void cstrl_string_remove(string *str, int index);

CSTRL_API void cstrl_da_string_remove(da_string *da, int index);

CSTRL_API int cstrl_da_int_find_first(da_int *da, int value);

CSTRL_API int cstrl_da_int_pop_back(da_int *da);

CSTRL_API float cstrl_da_float_pop_back(da_float *da);

CSTRL_API char cstrl_string_pop_back(string *str);

CSTRL_API string cstrl_da_string_pop_back(da_string *da);

CSTRL_API int cstrl_da_int_pop_front(da_int *da);

CSTRL_API float cstrl_da_float_pop_front(da_float *da);

CSTRL_API string cstrl_da_string_pop_front(da_string *da);

CSTRL_API void cstrl_da_int_clear(da_int *da);

CSTRL_API void cstrl_da_float_clear(da_float *da);

CSTRL_API void cstrl_string_clear(string *str);

CSTRL_API void cstrl_da_string_clear(da_string *da);

CSTRL_API void cstrl_da_int_free(da_int *da);

CSTRL_API void cstrl_da_float_free(da_float *da);

CSTRL_API void cstrl_string_free(string *str);

CSTRL_API void cstrl_da_string_free(da_string *da);

CSTRL_API char *cstrl_string_to_c_str(string *str);

CSTRL_API void cstrl_da_int_quick_sort(da_int *da, bool descending);

CSTRL_API char *cstrl_read_file(const char *file_path, long *file_size);

CSTRL_API int cstrl_write_file(const char *file_path, const char *data, unsigned long size);

CSTRL_API int cstrl_append_file(const char *file_path, const char *data, unsigned long size);

CSTRL_API int cstrl_copy_file(const char *file_path, const char *output_file_path);

CSTRL_API time_t cstrl_get_file_timestamp(const char *path);

CSTRL_API bool cstrl_realloc_int(int **int_ptr, size_t size);

CSTRL_API bool cstrl_realloc_unsigned_int(unsigned int **uint_ptr, size_t size);

CSTRL_API bool cstrl_realloc_float(float **float_ptr, size_t size);

CSTRL_API bool cstrl_realloc_string(string **str_ptr, size_t size);

CSTRL_API bool cstrl_realloc_transform(transform **trans_ptr, size_t size);

CSTRL_API bool cstrl_realloc_vec3(vec3 **vec3_ptr, size_t size);

CSTRL_API bool cstrl_realloc_bool(bool **bool_ptr, size_t size);

CSTRL_API bool cstrl_realloc_da_int(da_int **da_int_ptr, size_t size);

#endif // CSTRL_UTIL_H
