#include "cstrl/cstrl_defines.h"

#if defined(CSTRL_RENDER_API_VULKAN) && defined(CSTRL_PLATFORM_WINDOWS)
#define VK_USE_PLATFORM_WIN32_KHR
#include "../../platform/platform_internal.h"
#include "log.c/log.h"
#include "vulkan_platform.h"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_win32.h>

static VkSurfaceKHR g_surface;

CSTRL_API bool cstrl_vulkan_platform_init(cstrl_platform_state *platform_state, VkInstance instance)
{
    internal_state *state = (internal_state *)platform_state->internal_state;
    VkWin32SurfaceCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = state->hwnd;
    create_info.hinstance = state->h_instance;

    if (vkCreateWin32SurfaceKHR(instance, &create_info, NULL, &g_surface))
    {
        log_error("Failed to create window surface");
        return false;
    }
    return true;
}

CSTRL_API VkSurfaceKHR cstrl_vulkan_platform_get_surface()
{
    return g_surface;
}

CSTRL_API void cstrl_vulkan_platform_destroy(cstrl_platform_state *platform_state, VkInstance instance)
{
    vkDestroySurfaceKHR(instance, g_surface, NULL);
}

CSTRL_API void cstrl_vulkan_platform_swap_buffers(cstrl_platform_state *platform_state)
{
    internal_state *state = (internal_state *)platform_state->internal_state;
    HDC dc = GetDC(state->hwnd);
    SwapBuffers(dc);
}

#endif
