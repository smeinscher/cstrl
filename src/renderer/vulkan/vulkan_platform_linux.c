#include "cstrl/cstrl_defines.h"
#include "cstrl/cstrl_platform.h"

#if defined(CSTRL_RENDER_API_VULKAN) && defined(CSTRL_PLATFORM_LINUX)

#include "../../platform/platform_internal.h"
#include "log.c/log.h"
#include "vulkan_platform.h"
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_xlib.h>

static VkSurfaceKHR g_surface;
static char *g_extensions[CSTRL_VULKAN_REQUIRED_INSTANCE_EXTENSIONS_COUNT];

CSTRL_API bool cstrl_vulkan_platform_init(cstrl_platform_state *platform_state, VkInstance instance)
{
    internal_state *state = (internal_state *)platform_state->internal_state;
    VkXlibSurfaceCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    create_info.dpy = state->display;
    create_info.window = state->main_window;

    if (vkCreateXlibSurfaceKHR(instance, &create_info, NULL, &g_surface))
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
}

CSTRL_API const char **cstrl_vulkan_get_required_instance_extensions()
{
    g_extensions[0] = "VK_KHR_surface";
    g_extensions[1] = "VK_KHR_xlib_surface";
    g_extensions[2] = "VK_EXT_debug_utils";

    return (const char **)g_extensions;
}

#endif
