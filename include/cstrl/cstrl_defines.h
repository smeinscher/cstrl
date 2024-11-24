//
// Created by 12105 on 11/22/2024.
//

#ifndef DEFINES_H
#define DEFINES_H

#ifdef _WIN32
#define CSTRL_PLATFORM_WINDOWS
#else
#define CSTRL_PLATFORM_LINUX
#endif

#ifdef CSTRL_EXPORT
#ifdef _MSC_VER
#define CSTRL_API _declspec(dllexport)
#else
#define CSTRL_API __attribute__((visibility("default")))
#endif
#else
#ifdef _MSC_VER
#define CSTRL_API __declspec(dllimport)
#else
#define CSTRL_API
#endif
#endif

#define CSTRL_INLINE static inline

#endif // DEFINES_H
