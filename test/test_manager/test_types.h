//
// Created by 12105 on 11/22/2024.
//

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

#include "log.c/log.h"

#define cstrl_test_success 1
#define cstrl_test_failure 0

#define cstrl_test_epsilon 0.000001
#define cstrl_test_epsilon_double 2.22e-16

#define expect_bool_to_be(expected, result)                                                                            \
    if (expected != result)                                                                                            \
    {                                                                                                                  \
        const char *false_str = "false";                                                                               \
        const char *true_str = "true";                                                                                 \
        log_error("Expected: %s; Received: %s", expected ? true_str : false_str, result ? true_str : false_str);      \
        return 0;                                                                                                      \
    }

#define expect_int_to_be(expected, result)                                                                             \
    if (expected != result)                                                                                            \
    {                                                                                                                  \
        log_error("Expected: %d; Received: %d", expected, result);                                                 \
        return 0;                                                                                                      \
    }

#define expect_float_to_be(expected, result)                                                                           \
    if (fabsf(expected - result) > cstrl_test_epsilon)                                                                 \
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
