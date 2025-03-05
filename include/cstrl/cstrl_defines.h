//
// Created by 12105 on 11/22/2024.
//

#ifndef DEFINES_H
#define DEFINES_H

#if _DEBUG
#define CSTRL_DEBUG
#else
#define CSTRL_RELEASE
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#define CSTRL_PLATFORM_WINDOWS
#elif defined(__ANDROID__)
#define CSTRL_PLATFORM_ANDROID
#elif defined(__linux__) || defined(__gnu_linux__)
#define CSTRL_PLATFORM_LINUX
#else
#error "Unknown platform"
#endif

#if defined(CSTRL_EXPORT)
#if defined(_MSC_VER)
#define CSTRL_API _declspec(dllexport)
#else
#define CSTRL_API __attribute__((visibility("default")))
#endif
#else
#if defined(_MSC_VER)
#define CSTRL_API __declspec(dllimport)
#else
#define CSTRL_API
#endif
#endif

#define CSTRL_INLINE static inline

#endif // DEFINES_H
