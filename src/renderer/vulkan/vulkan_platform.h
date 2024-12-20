#ifndef CSTRL_VULKAN_PLATFORM_H
#define CSTRL_VULKAN_PLATFORM_H
#include "cstrl/cstrl_platform.h"

#include <vulkan/vulkan.h>
#include <stdbool.h>

#define CSTRL_VULKAN_REQUIRED_INSTANCE_EXTENSIONS_COUNT 3

CSTRL_API bool cstrl_vulkan_platform_init(cstrl_platform_state *platform_state, VkInstance instance);

CSTRL_API VkSurfaceKHR cstrl_vulkan_platform_get_surface();

CSTRL_API void cstrl_vulkan_platform_destroy(cstrl_platform_state *platform_state, VkInstance instance);

CSTRL_API void cstrl_vulkan_platform_swap_buffers(cstrl_platform_state *platform_state);

CSTRL_API const char** cstrl_vulkan_get_required_instance_extensions();

#endif // CSTRL_VULKAN_PLATFORM_H
