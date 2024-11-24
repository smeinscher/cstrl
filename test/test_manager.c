//
// Created by 12105 on 11/22/2024.
//

#include "test_manager.h"

#include "log.c/log.h"

#include <stdio.h>

#define MAX_TESTS 64
#define MAX_SUITES 32

typedef struct Test
{
    test_func func;
    const char *name;
    const char *desc;
} Test;

typedef struct Suite
{
    int total_tests;
    int passed_tests;
    Test tests[MAX_TESTS];
    const char *name;
    const char *desc;
} Suite;

static Suite suites[MAX_SUITES];

static int total_suites = 0;

int test_manager_add_suite(const char *name, const char *desc)
{
    if (total_suites >= MAX_SUITES)
    {
        log_fatal("Too many test suites");
        return -1;
    }
    suites[total_suites].total_tests = 0;
    suites[total_suites].passed_tests = -1;
    suites[total_suites].name = name;
    suites[total_suites].desc = desc;

    return total_suites++;
}

void test_manager_add_test(const int suite_id, test_func func, const char *name, const char *desc)
{
    if (suites[suite_id].total_tests >= MAX_TESTS)
    {
        log_fatal("Too many tests");
        return;
    }
    int total_tests = suites[suite_id].total_tests;
    suites[suite_id].tests[total_tests].func = func;
    suites[suite_id].tests[total_tests].name = name;
    suites[suite_id].tests[total_tests].desc = desc;

    suites[suite_id].total_tests++;
}

void test_manager_run_tests(int suite_id)
{
    log_info("Running test suite #%d", suite_id);
    log_trace("---- %s - %s", suites[suite_id].name, suites[suite_id].desc);
    log_trace("---- %d total tests", suites[suite_id].total_tests);

    suites[suite_id].passed_tests = 0;
    for (int i = 0; i < suites[suite_id].total_tests; i++)
    {
        log_trace("---- ---- Running test #%d", i);
        log_trace("---- ---- %s - %s", suites[suite_id].tests[i].name, suites[suite_id].tests[i].desc);
        int result = suites[suite_id].tests[i].func();
        if (result > 0)
        {
            log_trace("---- ---- Test passed");
        }
        else
        {
            log_trace("---- ---- Test failed");
        }
        suites[suite_id].passed_tests += result;
    }
}

void test_manager_log_results(int suite_id)
{
    log_info("%d of %d tests passed", suites[suite_id].passed_tests, suites[suite_id].total_tests);
}

int test_manager_total_failed_tests()
{
    int failed_tests = 0;
    for (int i = 0; i < total_suites; i++)
    {
        failed_tests += suites[i].total_tests - suites[i].passed_tests;
    }
    return failed_tests;
}

void test_manager_log_total_failed_tests()
{
    log_info("Total failed tests: %d\n", test_manager_total_failed_tests());
}
