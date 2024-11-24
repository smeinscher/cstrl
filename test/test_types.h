//
// Created by 12105 on 11/22/2024.
//

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "log.c/log.h"

#define expect_int_to_be(expected, result)                                                                             \
    if (expected != result)                                                                                            \
    {                                                                                                                  \
        log_error("Expected: %lld; Received: %lld", expected, result);                                                 \
        return 0;                                                                                                      \
    }

#define expect_float_to_be(expected, result)                                                                           \
    if (expected != result)                                                                                            \
    {                                                                                                                  \
        log_error("Expected: %f; Received: %f", expected, result);                                                     \
        return 0;                                                                                                      \
    }

#define expect_ptr_to_be_null(ptr)                                                                                     \
    if (ptr != NULL)                                                                                                   \
    {                                                                                                                  \
        log_error("Expected pointer to be NULL");                                                                      \
        return 0;                                                                                                      \
    }

#define expect_ptr_to_not_be_null(ptr)                                                                                 \
    if (ptr == NULL)                                                                                                   \
    {                                                                                                                  \
        log_error("Expected pointer to NOT be NULL");                                                                  \
        return 0;                                                                                                      \
    }

#endif // TEST_TYPES_H
