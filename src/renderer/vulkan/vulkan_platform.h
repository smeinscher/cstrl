#ifndef CSTRL_VULKAN_PLATFORM_H
#define CSTRL_VULKAN_PLATFORM_H
#include "cstrl/cstrl_platform.h"

#include <vulkan/vulkan.h>
#include <stdbool.h>

CSTRL_API bool cstrl_vulkan_platform_init(cstrl_platform_state *platform_state, VkInstance instance);

CSTRL_API VkSurfaceKHR cstrl_vulkan_platform_get_surface();

CSTRL_API void cstrl_vulkan_platform_destroy(cstrl_platform_state *platform_state, VkInstance instance);

CSTRL_API void cstrl_vulkan_platform_swap_buffers(cstrl_platform_state *platform_state);

#endif // CSTRL_VULKAN_PLATFORM_H
