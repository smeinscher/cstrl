//
// Created by sterling on 7/11/24.
//

#ifndef FILE_HELPERS_H
#define FILE_HELPERS_H
#include <time.h>

char *read_file(const char *file_path, long *file_size);

int write_file(const char *file_path, const char *data, unsigned long size);

int copy_file(const char *file_path, const char *output_file_path);

time_t get_file_timestamp(const char *path);

#endif // FILE_HELPERS_H
