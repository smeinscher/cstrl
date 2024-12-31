//
// Created by sterling on 7/11/24.
//

#define _CRT_SECURE_NO_WARNINGS
#include "cstrl/cstrl_util.h"

#include "log.c/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

CSTRL_API char *cstrl_read_file(const char *file_path, long *file_size)
{
    *file_size = 0;
    FILE *file = fopen(file_path, "rb");
    if (file == NULL)
    {
        log_error("Error opening file %s", file_path);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    *file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = malloc(sizeof(char) * (*file_size + 1));
    if (buffer == NULL)
    {
        log_error("Error allocating memory");
        return NULL;
    }
    fread(buffer, sizeof(char), *file_size, file);
    fclose(file);

    buffer[*file_size] = '\0';
    return buffer;
}

CSTRL_API int cstrl_write_file(const char *file_path, const char *data, const unsigned long size)
{
    FILE *output_file = fopen(file_path, "wb");
    if (output_file == NULL)
    {
        log_error("Error opening output file %s", file_path);
        return -1;
    }

    unsigned long result = fwrite(data, sizeof(char), size, output_file);
    fclose(output_file);
    if (result != size)
    {
        log_error("Error writing to output file %s", file_path);
        return -1;
    }
    return 0;
}

CSTRL_API int cstrl_copy_file(const char *file_path, const char *output_file_path)
{
    long file_size = 0;
    char *data = cstrl_read_file(file_path, &file_size);

    FILE *output_file = fopen(output_file_path, "wb");
    if (output_file == NULL)
    {
        log_error("Error opening output file %s", output_file_path);
        return -1;
    }

    unsigned long result = fwrite(data, sizeof(char), file_size, output_file);
    if (result != file_size)
    {
        log_error("Error writing to output file %s", output_file_path);
        return -1;
    }

    fclose(output_file);
    free(data);
    return 0;
}

CSTRL_API time_t cstrl_get_file_timestamp(const char *path)
{
    struct stat st = {0};
    int ierr = stat(path, &st);
    if (ierr != 0)
    {
        log_error("Error loading file timestamp");
    }
    return st.st_mtime;
}
