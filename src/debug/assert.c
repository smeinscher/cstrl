#include "cstrl/cstrl_assert.h"
#include "log.c/log.h"

CSTRL_API void log_assertion_failed(const char *expression, const char *message, const char *file, int line)
{
    log_fatal("Assertion Failed: %s, message: %s; (%s, %d)", expression, message, file, line);
}
