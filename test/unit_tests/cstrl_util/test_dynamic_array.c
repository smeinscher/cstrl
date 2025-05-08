//
// Created by 12105 on 11/23/2024.
//

#include "test_dynamic_array.h"

#include "../../test_manager/test_types.h"
#include "cstrl/cstrl_types.h"
#include "cstrl/cstrl_util.h"

#include <limits.h>
#include <stdint.h>

int test_cstrl_da_int_init_happy_path()
{
    da_int da;
    cstrl_da_int_init(&da, 10);

    expect_ptr_to_not_be_null(da.array);
    expect_int_to_be(0, da.size);
    expect_int_to_be(10, da.capacity);

    return 1;
}

int test_cstrl_da_int_init_zero_size()
{
    da_int da;
    cstrl_da_int_init(&da, 0);

    expect_ptr_to_not_be_null(da.array);
    expect_int_to_be(0, da.size);
    expect_int_to_be(0, da.capacity);

    return 1;
}

int test_cstrl_da_int_reserve_happy_path()
{
    da_int da;
    cstrl_da_int_init(&da, 10);
    cstrl_da_int_reserve(&da, 20);

    expect_ptr_to_not_be_null(da.array);
    expect_int_to_be(0, da.size);
    expect_int_to_be(20, da.capacity);

    return 1;
}

int test_cstrl_da_int_reserve_overflow_capacity()
{
    da_int da;
    cstrl_da_int_init(&da, 10);
    bool expected = false;
    bool result = cstrl_da_int_reserve(&da, SIZE_MAX);

    expect_int_to_be(expected, result);
    expect_ptr_to_not_be_null(da.array);
    expect_int_to_be(0, da.size);
    expect_int_to_be(10, da.capacity);

    return 1;
}

int test_cstrl_da_int_push_back_happy_path()
{
    da_int da;
    cstrl_da_int_init(&da, 10);
    int expected = 42;
    cstrl_da_int_push_back(&da, expected);

    expect_int_to_be(expected, da.array[0]);

    return 1;
}

int test_cstrl_da_int_push_back_breach_capacity()
{
    da_int da;
    cstrl_da_int_init(&da, 1);
    int expected = 42;
    cstrl_da_int_push_back(&da, -1);
    cstrl_da_int_push_back(&da, expected);

    expect_int_to_be(expected, da.array[1]);

    return 1;
}
