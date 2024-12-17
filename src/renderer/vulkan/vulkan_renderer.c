//
// Created by 12105 on 12/13/2024.
//

#if defined(CSTRL_RENDER_API_VULKAN)
#include "cstrl/cstrl_renderer.h"
#include "log.c/log.h"
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>

#ifdef NDEBUG
#define CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS 0
#else
#define CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS 1
#endif

VkInstance g_instance;
VkDebugUtilsMessengerEXT g_debug_messenger;

const char *validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
int validation_layer_size = 1;

bool check_validation_layer_support()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    VkLayerProperties available_layers[layer_count];
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

    for (int i = 0; i < validation_layer_size; i++)
    {
        bool layer_found = false;
        for (int j = 0; j < layer_count; j++)
        {
            if (strcmp(available_layers[j].layerName, validation_layers[i]) == 0)
            {
                layer_found = true;
                break;
            }
        }

        if (!layer_found)
        {
            return false;
        }
    }

    return true;
}

const char **get_required_extensions(uint32_t *extension_count)
{
    if (vkEnumerateInstanceExtensionProperties(NULL, extension_count, NULL) != VK_SUCCESS)
    {
        log_fatal("Failed to get count; vkEnumerateInstanceExtensionProperties failed");
        return VK_NULL_HANDLE;
    }

    VkExtensionProperties extension_properties[*extension_count];
    if (vkEnumerateInstanceExtensionProperties(NULL, extension_count, extension_properties) != VK_SUCCESS)
    {
        log_fatal("Failed to get extensions; vkEnumerateInstanceExtensionProperties failed");
        return VK_NULL_HANDLE;
    }

    const char **extensions = malloc(*extension_count * sizeof(char *));
    for (int i = 0; i < *extension_count; i++)
    {
        char *name = malloc(256 * sizeof(char));
        strcpy(name, extension_properties[i].extensionName);
        extensions[i] = name;
    }

    return extensions;
}

VkInstance create_instance()
{
#if defined(CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS)
    if (!check_validation_layer_support())
    {
        log_error("Validation layers not available");
        return VK_NULL_HANDLE;
    }
#endif
    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Hello Triangle";
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "cstrl";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

#if defined(CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS)
    create_info.enabledLayerCount = validation_layer_size;
    create_info.ppEnabledLayerNames = validation_layers;
#else
    create_info.enabledLayerCount = 0;
#endif

    uint32_t extension_count = 0;
    const char **extensions = get_required_extensions(&extension_count);

    create_info.enabledExtensionCount = extension_count;
    create_info.ppEnabledExtensionNames = extensions;

    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&create_info, NULL, &instance);
    if (result != VK_SUCCESS)
    {
        log_fatal("Failed to create instance; vkCreateInstance failed (%d)", result);
        return VK_NULL_HANDLE;
    }

    free(extensions);

    return instance;
}

#if defined(CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS)
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
                                                     VkDebugUtilsMessageTypeFlagsEXT message_type,
                                                     const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
                                                     void *user_data)
{
    switch (message_severity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        log_trace("Validation layer: %s", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        log_info("Validation layer: %s", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        log_warn("Validation layer: %s", callback_data->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        log_error("Validation layer: %s", callback_data->pMessage);
        break;
    default:
        break;
    }
    return VK_FALSE;
}

void setup_debug_messenger()
{
    VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                              VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info.pfnUserCallback = debug_callback;
    create_info.pUserData = NULL;
}
#endif

bool cstrl_renderer_init(cstrl_platform_state *platform_state)
{
    g_instance = create_instance();
#if defined(CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS)
    setup_debug_messenger();
#endif
    log_trace("Using Vulkan!");

    return true;
}

void cstrl_renderer_clear(float r, float g, float b, float a)
{
}

cstrl_render_data *cstrl_renderer_create_render_data()
{
    return NULL;
}

void cstrl_renderer_free_render_data(cstrl_render_data *render_data)
{
}

void cstrl_renderer_add_positions(cstrl_render_data *render_data, float *positions, unsigned int dimensions,
                                  unsigned int vertex_count)
{
}

void cstrl_renderer_add_uvs(cstrl_render_data *render_data, float *uvs)
{
}

void cstrl_renderer_add_colors(cstrl_render_data *render_data, float *colors)
{
}

void cstrl_renderer_add_normals(cstrl_render_data *render_data, float *normals)
{
}

void cstrl_renderer_modify_render_attributes(cstrl_render_data *render_data, const float *positions, const float *uvs,
                                             const float *colors, size_t count)
{
}

void cstrl_renderer_draw(cstrl_render_data *data)
{
}

void cstrl_renderer_destroy(cstrl_platform_state *platform_state)
{
    vkDestroyInstance(g_instance, NULL);
}

void cstrl_renderer_swap_buffers(cstrl_platform_state *platform_state)
{
}

#endif
