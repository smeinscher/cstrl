#ifndef CSTRL_ASSERT_H
#define CSTRL_ASSERT_H

#include "cstrl/cstrl_defines.h"
#define CSTRL_ASSERTIONS_ENABLED

#if defined(__has_builtin) && !defined(__ibmxl__)
#   if __has_builtin(__builtin_debugtrap)
#       define cstrl_debug_break() __builtin_debugtrap()
#   elif __has_builtin_(__debugbreak)
#       define cstrl_debug_break() __debug_break()
#   endif
#endif

#if !defined(cstrl_debug_break)
#   if defined(__clang__) || defined(__gcc__)
#        define cstrl_debug_break() __builtin_trap()
#    elif defined(_MSC_VER)
#        include <intrin.h>
#        define cstrl_debug_break() __debugbreak()
#    else
#        define cstrl_debug_break() asm { int 3 }
#    endif
#endif

#ifdef CSTRL_ASSERTIONS_ENABLED

CSTRL_API void log_assertion_failed(const char *expression, const char *message, const char *file, int line);

#define CSTRL_ASSERT(expr, message)                              \
    {                                                            \
        if (expr){}                                              \
        else                                                     \
        {                                                        \
            log_assertion_failed(#expr, message, __FILE__, __LINE__); \
            cstrl_debug_break();                                 \
        }                                                        \
    }

#ifdef CSTRL_DEBUG
#define CSTRL_ASSERT_DEBUG(expr, message) CSTRL_ASSERT(expr, message)
#else
#define CSTRL_ASSERT_DEBUG(expr, message)
#endif

#endif  // CSTRL_ASSERTIONS_ENABLED

#endif // CSTRL_ASSERT_H
