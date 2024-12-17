//
// Created by 12105 on 11/22/2024.
//

#include "integration_tests/integration_tests.h"
#include "log.c/log.h"
#include "manual_tests/manual_tests.h"
#include "test_manager/test_types.h"
#include "unit_tests/unit_tests.h"

#define CSTRL_TEST_RUN_UNIT_TESTS 0
#define CSTRL_TEST_RUN_INTEGRATION_TESTS 0
#define CSTRL_TEST_RUN_MANUAL_TESTS 1

int main()
{
#if CSTRL_TEST_RUN_UNIT_TESTS
    log_info("Starting unit tests");
    if (run_unit_tests() == cstrl_test_failure)
    {
        log_warn("Failed unit tests");
    }
#endif
#if CSTRL_TEST_RUN_INTEGRATION_TESTS
    log_info("Starting integration tests");
    if (run_integration_tests() == cstrl_test_failure)
    {
        log_warn("Failed integration tests");
    }
#endif
#if CSTRL_TEST_RUN_MANUAL_TESTS
    log_info("Starting manual tests");
    if (run_manual_tests() == cstrl_test_failure)
    {
        log_warn("Failed manual tests");
    }
#endif
    return 0;
}
