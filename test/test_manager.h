//
// Created by 12105 on 11/22/2024.
//

#ifndef TEST_MANAGER_H
#define TEST_MANAGER_H

typedef int (*test_func)();

int test_manager_add_suite(const char *name, const char *desc);

void test_manager_add_test(int suite_id, test_func func, const char *name, const char *desc);

void test_manager_run_tests(int suite_id);

void test_manager_log_results(int suite_id);

int test_manager_total_failed_tests();

void test_manager_log_total_failed_tests();

#endif // TEST_MANAGER_H
