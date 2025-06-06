//
// Created by 12105 on 12/13/2024.
//

#define _CRT_SECURE_NO_WARNINGS
#include "cstrl/cstrl_math.h"
#include "cstrl/cstrl_platform.h"
#include "cstrl/cstrl_util.h"
#if defined(CSTRL_RENDER_API_VULKAN)
#include "cstrl/cstrl_renderer.h"
#include "log.c/log.h"
#include "vulkan_platform.h"
#include <stb/stb_image.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#ifdef NDEBUG
#define CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS 0
#else
#define CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS 1
#endif

#define MAX_FRAMES_IN_FLIGHT 2

static VkInstance g_instance;
static VkDebugUtilsMessengerEXT g_debug_messenger;

static const char *g_validation_layers[] = {"VK_LAYER_KHRONOS_validation"};
static int g_validation_layer_size = 1;

static VkPhysicalDevice g_physical_device = VK_NULL_HANDLE;

static VkDevice g_device;

static VkQueue g_graphics_queue;

static VkQueue g_present_queue;

static const char *g_device_extensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
static int g_device_extensions_size = 1;

static VkSwapchainKHR g_swap_chain;

static VkImage *g_swap_chain_images;
static uint32_t g_swap_chain_image_count;

static VkFormat g_swap_chain_image_format;
static VkExtent2D g_swap_chain_extent;

static VkImageView *g_swap_chain_image_views;

static VkRenderPass g_render_pass;
static VkDescriptorSetLayout g_descriptor_set_layout;
static VkPipelineLayout g_pipeline_layout;

static VkPipeline g_graphics_pipeline;

static VkFramebuffer *g_swap_chain_framebuffers;

static VkCommandPool g_command_pool;

static VkCommandBuffer g_command_buffers[MAX_FRAMES_IN_FLIGHT];

static VkSemaphore g_image_available_semaphores[MAX_FRAMES_IN_FLIGHT];
static VkSemaphore g_render_finished_semaphores[MAX_FRAMES_IN_FLIGHT];
static VkFence g_in_flight_fences[MAX_FRAMES_IN_FLIGHT];

static uint32_t g_current_frame = 0;

static bool g_framebuffer_resized = false;

static VkBuffer g_vertex_buffer;
static VkDeviceMemory g_vertex_buffer_memory;
static VkBuffer g_index_buffer;
static VkDeviceMemory g_index_buffer_memory;

static VkBuffer g_uniform_buffers[MAX_FRAMES_IN_FLIGHT];
static VkDeviceMemory g_uniform_buffers_memory[MAX_FRAMES_IN_FLIGHT];
static void *g_uniform_buffers_mapped[MAX_FRAMES_IN_FLIGHT];

static VkDescriptorPool g_descriptor_pool;

static VkDescriptorSet g_descriptor_sets[MAX_FRAMES_IN_FLIGHT];

typedef struct vertex
{
    vec2 pos;
    vec3 color;
    vec2 uv;
} vertex_t;

const vertex_t g_vertices[] = {{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                               {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                               {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                               {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}};
const uint16_t g_indices[] = {0, 1, 2, 2, 3, 0};

typedef struct uniform_buffer_object
{
    mat4 model;
    mat4 view;
    mat4 projection;
} uniform_buffer_object;

static VkImage g_texture_image;
static VkDeviceMemory g_texture_image_memory;

static VkImageView g_texture_image_view;

static VkSampler g_texture_sampler;

static VkVertexInputBindingDescription get_binding_description()
{
    VkVertexInputBindingDescription binding_description = {0};
    binding_description.binding = 0;
    binding_description.stride = sizeof(vertex_t);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return binding_description;
}

static VkVertexInputAttributeDescription *get_attribute_descriptions()
{
    VkVertexInputAttributeDescription *attribute_description = malloc(3 * sizeof(VkVertexInputAttributeDescription));
    attribute_description[0].binding = 0;
    attribute_description[0].location = 0;
    attribute_description[0].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_description[0].offset = offsetof(vertex_t, pos);
    attribute_description[1].binding = 0;
    attribute_description[1].location = 1;
    attribute_description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_description[1].offset = offsetof(vertex_t, color);
    attribute_description[2].binding = 0;
    attribute_description[2].location = 2;
    attribute_description[2].format = VK_FORMAT_R32G32_SFLOAT;
    attribute_description[2].offset = offsetof(vertex_t, uv);

    return attribute_description;
}

static bool check_validation_layer_support()
{
    uint32_t layer_count;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    VkLayerProperties available_layers[layer_count];
    vkEnumerateInstanceLayerProperties(&layer_count, available_layers);

    for (int i = 0; i < g_validation_layer_size; i++)
    {
        bool layer_found = false;
        for (int j = 0; j < layer_count; j++)
        {
            if (strcmp(available_layers[j].layerName, g_validation_layers[i]) == 0)
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

static const char **get_required_extensions(uint32_t *extension_count)
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

static void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT *create_info)
{
    create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    create_info->messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    create_info->messageSeverity =
        /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |*/
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    create_info->pfnUserCallback = debug_callback;
    create_info->pUserData = NULL;
}

static void setup_debug_messenger()
{
    VkDebugUtilsMessengerCreateInfoEXT create_info = {0};
    populate_debug_messenger_create_info(&create_info);

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_instance, "vkCreateDebugUtilsMessengerEXT");
    if (vkCreateDebugUtilsMessengerEXT == NULL)
    {
        log_error("Failed to setup debug messenger");
    }
    else
    {
        vkCreateDebugUtilsMessengerEXT(g_instance, &create_info, NULL, &g_debug_messenger);
    }
}
#endif

static VkInstance create_instance()
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
    create_info.enabledLayerCount = g_validation_layer_size;
    create_info.ppEnabledLayerNames = g_validation_layers;

    VkDebugUtilsMessengerCreateInfoEXT debug_create_info = {0};
    populate_debug_messenger_create_info(&debug_create_info);
    create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debug_create_info;
#else
    create_info.enabledLayerCount = 0;
#endif

    const char **extensions = cstrl_vulkan_get_required_instance_extensions();

    create_info.enabledExtensionCount = CSTRL_VULKAN_REQUIRED_INSTANCE_EXTENSIONS_COUNT;
    create_info.ppEnabledExtensionNames = extensions;

    VkInstance instance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&create_info, NULL, &instance);
    if (result != VK_SUCCESS)
    {
        log_fatal("Failed to create instance; vkCreateInstance failed (%d)", result);
        return VK_NULL_HANDLE;
    }

    return instance;
}

typedef struct queue_family_indices
{
    int graphics_family;
    int present_family;
} queue_family_indices;

static bool queue_family_indices_is_complete(queue_family_indices indices)
{
    return indices.graphics_family != -1 && indices.present_family != -1;
}

static queue_family_indices find_queue_families(VkPhysicalDevice device)
{
    queue_family_indices indices;
    indices.graphics_family = -1;
    indices.present_family = -1;

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, NULL);

    VkQueueFamilyProperties queue_families[queue_family_count];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families);

    VkSurfaceKHR surface = cstrl_vulkan_platform_get_surface();
    for (int i = 0; i < queue_family_count; i++)
    {
        if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
        {
            indices.graphics_family = i;
        }
        VkBool32 present_support = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present_support);
        if (present_support)
        {
            indices.present_family = i;
        }
        if (queue_family_indices_is_complete(indices))
        {
            break;
        }
    }
    return indices;
}

static bool check_device_extension_support(VkPhysicalDevice device)
{
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);

    VkExtensionProperties available_extensions[extension_count];
    vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, available_extensions);

    for (int i = 0; i < g_device_extensions_size; i++)
    {
        bool extension_found = false;
        for (int j = 0; j < extension_count; j++)
        {
            if (strcmp(available_extensions[j].extensionName, g_device_extensions[i]) == 0)
            {
                extension_found = true;
                break;
            }
        }

        if (!extension_found)
        {
            return false;
        }
    }

    return true;
}

typedef struct swap_chain_support_details
{
    VkSurfaceCapabilitiesKHR capabilites;
    VkSurfaceFormatKHR *formats;
    int formats_size;
    VkPresentModeKHR *present_modes;
    int present_modes_size;
} swap_chain_support_details;

static swap_chain_support_details query_swap_chain_support(VkPhysicalDevice device)
{
    swap_chain_support_details details;
    VkSurfaceKHR surface = cstrl_vulkan_platform_get_surface();

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilites);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, NULL);

    if (format_count != 0)
    {
        details.formats = malloc(format_count * sizeof(VkSurfaceFormatKHR));
        details.formats_size = format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats);
    }
    else
    {
        details.formats = NULL;
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, NULL);

    if (present_mode_count != 0)
    {
        details.present_modes = malloc(present_mode_count * sizeof(VkPresentModeKHR));
        details.present_modes_size = present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &present_mode_count, details.present_modes);
    }
    else
    {
        details.present_modes = NULL;
    }

    return details;
}

static bool is_device_suitable(VkPhysicalDevice device)
{
    queue_family_indices indices = find_queue_families(device);

    bool extensions_supported = check_device_extension_support(device);

    bool swap_chain_adequate = false;

    if (extensions_supported)
    {
        swap_chain_support_details swap_chain_support = query_swap_chain_support(device);
        swap_chain_adequate = swap_chain_support.formats && swap_chain_support.present_modes;
        free(swap_chain_support.formats);
        free(swap_chain_support.present_modes);
    }

    VkPhysicalDeviceFeatures supported_features;
    vkGetPhysicalDeviceFeatures(device, &supported_features);

    return queue_family_indices_is_complete(indices) && extensions_supported && swap_chain_adequate &&
           supported_features.samplerAnisotropy;
}

static void pick_physical_device()
{
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(g_instance, &device_count, NULL);
    if (device_count == 0)
    {
        log_error("Failed to find physical device with Vulkan support");
        return;
    }
    VkPhysicalDevice devices[device_count];
    vkEnumeratePhysicalDevices(g_instance, &device_count, devices);
    for (int i = 0; i < device_count; i++)
    {
        if (is_device_suitable(devices[i]))
        {
            g_physical_device = devices[i];
            break;
        }
    }

    if (g_physical_device == VK_NULL_HANDLE)
    {
        log_error("Failed to find a suitable physical device");
    }
}

static void create_logical_device()
{
    queue_family_indices indices = find_queue_families(g_physical_device);

    int total_families = 2;
    VkDeviceQueueCreateInfo queue_create_infos[total_families];
    da_int unique_queue_families;
    cstrl_da_int_init(&unique_queue_families, total_families);
    cstrl_da_int_push_back(&unique_queue_families, indices.graphics_family);
    if (indices.graphics_family != indices.present_family)
    {
        cstrl_da_int_push_back(&unique_queue_families, indices.present_family);
    }
    float queue_priority = 1.0f;
    for (int i = 0; i < unique_queue_families.size; i++)
    {
        queue_create_infos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_infos[i].queueFamilyIndex = unique_queue_families.array[i];
        queue_create_infos[i].queueCount = 1;
        queue_create_infos[i].pQueuePriorities = &queue_priority;
        queue_create_infos[i].flags = 0;
        queue_create_infos[i].pNext = NULL;
    }

    VkPhysicalDeviceFeatures device_features = {0};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.queueCreateInfoCount = unique_queue_families.size;
    create_info.pQueueCreateInfos = queue_create_infos;

    create_info.pEnabledFeatures = &device_features;

    create_info.enabledExtensionCount = g_device_extensions_size;
    create_info.ppEnabledExtensionNames = g_device_extensions;

#ifdef CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS
    create_info.enabledLayerCount = g_validation_layer_size;
    create_info.ppEnabledLayerNames = g_validation_layers;
#else
    create_info.enabledLayerCount = 0;
#endif

    if (vkCreateDevice(g_physical_device, &create_info, NULL, &g_device) != VK_SUCCESS)
    {
        log_error("Failed to create logical device");
        return;
    }

    vkGetDeviceQueue(g_device, indices.graphics_family, 0, &g_graphics_queue);
    vkGetDeviceQueue(g_device, indices.present_family, 0, &g_present_queue);

    cstrl_da_int_free(&unique_queue_families);
}

static VkSurfaceFormatKHR choose_swap_surface_format(VkSurfaceFormatKHR *available_formats, int available_formats_size)
{
    for (int i = 0; i < available_formats_size; i++)
    {
        if (available_formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            available_formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return available_formats[i];
        }
    }

    return available_formats[0];
}

static VkPresentModeKHR choose_swap_present_mode(VkPresentModeKHR *available_present_modes,
                                                 int available_present_modes_size)
{
    for (int i = 0; i < available_present_modes_size; i++)
    {
        if (available_present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return available_present_modes[i];
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

static VkExtent2D choose_swap_extent(VkSurfaceCapabilitiesKHR *capabilities)
{
    if (capabilities->currentExtent.width != UINT32_MAX)
    {
        return capabilities->currentExtent;
    }
    else
    {
        int width, height;
        // TODO: get framebuffer size or something
        // glfwGetFramebufferSize(window, &width, &height)
        width = 800;
        height = 600;

        VkExtent2D actual_extent = {(uint32_t)width, (uint32_t)height};

        return actual_extent;
    }
}

static void create_swap_chain()
{
    swap_chain_support_details swap_chain_support = query_swap_chain_support(g_physical_device);

    VkSurfaceFormatKHR surface_format =
        choose_swap_surface_format(swap_chain_support.formats, swap_chain_support.formats_size);
    VkPresentModeKHR present_mode =
        choose_swap_present_mode(swap_chain_support.present_modes, swap_chain_support.present_modes_size);
    VkExtent2D extent = choose_swap_extent(&swap_chain_support.capabilites);

    uint32_t image_count = swap_chain_support.capabilites.minImageCount + 1;
    if (swap_chain_support.capabilites.maxImageCount > 0 && image_count > swap_chain_support.capabilites.maxImageCount)
    {
        image_count = swap_chain_support.capabilites.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = cstrl_vulkan_platform_get_surface();
    create_info.minImageCount = image_count;
    create_info.imageFormat = surface_format.format;
    create_info.imageColorSpace = surface_format.colorSpace;
    create_info.imageExtent = extent;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    queue_family_indices indices = find_queue_families(g_physical_device);
    uint32_t queue_family_indices[] = {indices.graphics_family, indices.present_family};

    if (indices.graphics_family != indices.present_family)
    {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else
    {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        create_info.queueFamilyIndexCount = 0;
        create_info.pQueueFamilyIndices = NULL;
    }

    create_info.preTransform = swap_chain_support.capabilites.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = present_mode;
    create_info.clipped = VK_TRUE;
    create_info.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(g_device, &create_info, NULL, &g_swap_chain) != VK_SUCCESS)
    {
        log_error("Failed to create swap chain");
    }

    vkGetSwapchainImagesKHR(g_device, g_swap_chain, &g_swap_chain_image_count, NULL);
    g_swap_chain_images = malloc(g_swap_chain_image_count * sizeof(VkImage));
    vkGetSwapchainImagesKHR(g_device, g_swap_chain, &g_swap_chain_image_count, g_swap_chain_images);

    g_swap_chain_image_format = surface_format.format;
    g_swap_chain_extent = extent;

    free(swap_chain_support.formats);
    free(swap_chain_support.present_modes);
}

static VkImageView create_image_view(VkImage image, VkFormat format)
{
    VkImageViewCreateInfo view_info = {0};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    if (vkCreateImageView(g_device, &view_info, NULL, &image_view))
    {
        log_error("Failed to create texture image view");
    }

    return image_view;
}

static void create_image_views()
{
    g_swap_chain_image_views = malloc(g_swap_chain_image_count * sizeof(VkImageView));

    for (int i = 0; i < g_swap_chain_image_count; i++)
    {
        g_swap_chain_image_views[i] = create_image_view(g_swap_chain_images[i], g_swap_chain_image_format);
    }
}

static void create_render_pass()
{
    VkAttachmentDescription color_attachment = {0};
    color_attachment.format = g_swap_chain_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref = {0};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_attachment_ref;

    VkRenderPassCreateInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = 1;
    render_pass_info.pAttachments = &color_attachment;
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;

    VkSubpassDependency dependency = {0};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    if (vkCreateRenderPass(g_device, &render_pass_info, NULL, &g_render_pass) != VK_SUCCESS)
    {
        log_error("Failed to create render pass");
    }
}

static VkShaderModule create_shader_module(const char *shader_code, long file_size)
{
    VkShaderModuleCreateInfo create_info = {0};
    create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    create_info.codeSize = file_size;
    create_info.pCode = (uint32_t *)shader_code;

    VkShaderModule shader_module;
    if (vkCreateShaderModule(g_device, &create_info, NULL, &shader_module) != VK_SUCCESS)
    {
        log_error("Failed to create shader module");
        return VK_NULL_HANDLE;
    }

    return shader_module;
}

static void create_graphics_pipeline()
{
    long vertex_shader_file_size;
    char *vertex_shader_code =
        cstrl_read_file("resources/shaders/vulkan-tutorial/texture_mapping/vert.spv", &vertex_shader_file_size);
    long fragment_shader_file_size;
    char *fragment_shader_code =
        cstrl_read_file("resources/shaders/vulkan-tutorial/texture_mapping/frag.spv", &fragment_shader_file_size);

    VkShaderModule vertex_shader_module = create_shader_module(vertex_shader_code, vertex_shader_file_size);
    VkShaderModule fragment_shader_module = create_shader_module(fragment_shader_code, fragment_shader_file_size);

    free(vertex_shader_code);
    free(fragment_shader_code);

    VkPipelineShaderStageCreateInfo vertex_shader_stage_info = {0};
    vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertex_shader_stage_info.module = vertex_shader_module;
    vertex_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo fragment_shader_stage_info = {0};
    fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragment_shader_stage_info.module = fragment_shader_module;
    fragment_shader_stage_info.pName = "main";

    VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_stage_info, fragment_shader_stage_info};

    VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state = {0};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = 2;
    dynamic_state.pDynamicStates = dynamic_states;

    VkVertexInputBindingDescription binding_description = get_binding_description();
    VkVertexInputAttributeDescription *attribute_description = get_attribute_descriptions();

    VkPipelineVertexInputStateCreateInfo vertex_input_info = {0};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 1;
    vertex_input_info.pVertexBindingDescriptions = &binding_description;
    vertex_input_info.vertexAttributeDescriptionCount = 3;
    vertex_input_info.pVertexAttributeDescriptions = attribute_description;

    VkPipelineInputAssemblyStateCreateInfo input_assembly = {0};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)g_swap_chain_extent.width;
    viewport.height = (float)g_swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = g_swap_chain_extent;

    VkPipelineViewportStateCreateInfo viewport_state = {0};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo rasterizer = {0};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f;
    rasterizer.depthBiasClamp = 0.0f;
    rasterizer.depthBiasSlopeFactor = 0.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

    VkPipelineMultisampleStateCreateInfo multisampling = {0};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = NULL;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {0};
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo color_blending = {0};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY;
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f;
    color_blending.blendConstants[1] = 0.0f;
    color_blending.blendConstants[2] = 0.0f;
    color_blending.blendConstants[3] = 0.0f;

    VkPipelineLayoutCreateInfo pipeline_layout_info = {0};
    pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_info.setLayoutCount = 1;
    pipeline_layout_info.pSetLayouts = &g_descriptor_set_layout;
    pipeline_layout_info.pushConstantRangeCount = 0;
    pipeline_layout_info.pPushConstantRanges = NULL;

    if (vkCreatePipelineLayout(g_device, &pipeline_layout_info, NULL, &g_pipeline_layout) != VK_SUCCESS)
    {
        log_error("Failed to create pipeline layout");
    }

    VkGraphicsPipelineCreateInfo pipeline_info = {0};
    pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_info.stageCount = 2;
    pipeline_info.pStages = shader_stages;
    pipeline_info.pVertexInputState = &vertex_input_info;
    pipeline_info.pInputAssemblyState = &input_assembly;
    pipeline_info.pViewportState = &viewport_state;
    pipeline_info.pRasterizationState = &rasterizer;
    pipeline_info.pMultisampleState = &multisampling;
    pipeline_info.pDepthStencilState = NULL;
    pipeline_info.pColorBlendState = &color_blending;
    pipeline_info.pDynamicState = &dynamic_state;
    pipeline_info.layout = g_pipeline_layout;
    pipeline_info.renderPass = g_render_pass;
    pipeline_info.subpass = 0;
    pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_info.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(g_device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &g_graphics_pipeline) !=
        VK_SUCCESS)
    {
        log_error("Failed to create graphics pipeline");
    }

    vkDestroyShaderModule(g_device, vertex_shader_module, NULL);
    vkDestroyShaderModule(g_device, fragment_shader_module, NULL);
    // TODO: see if this causes issues
    free(attribute_description);
}

static void create_framebuffers()
{
    g_swap_chain_framebuffers = malloc(g_swap_chain_image_count * sizeof(VkFramebuffer));

    for (size_t i = 0; i < g_swap_chain_image_count; i++)
    {
        VkImageView attachments[] = {g_swap_chain_image_views[i]};

        VkFramebufferCreateInfo framebuffer_info = {0};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = g_render_pass;
        framebuffer_info.attachmentCount = 1;
        framebuffer_info.pAttachments = attachments;
        framebuffer_info.width = g_swap_chain_extent.width;
        framebuffer_info.height = g_swap_chain_extent.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(g_device, &framebuffer_info, NULL, &g_swap_chain_framebuffers[i]) != VK_SUCCESS)
        {
            log_error("Failed to create framebuffer");
        }
    }
}

static void create_command_pool()
{
    queue_family_indices queue_family_indices = find_queue_families(g_physical_device);

    VkCommandPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pool_info.queueFamilyIndex = queue_family_indices.graphics_family;

    if (vkCreateCommandPool(g_device, &pool_info, NULL, &g_command_pool) != VK_SUCCESS)
    {
        log_error("Failed to create command pool");
    }
}

static void create_command_buffers()
{
    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = g_command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(g_device, &alloc_info, g_command_buffers) != VK_SUCCESS)
    {
        log_error("Failed to allocate command buffers");
    }
}

static uint32_t find_memory_type(uint32_t type_filter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(g_physical_device, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++)
    {
        if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }
    log_error("Failed to find suitable memory type");
    return UINT32_MAX;
}

static void create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
                          VkBuffer *buffer, VkDeviceMemory *buffer_memory)
{
    VkBufferCreateInfo buffer_info = {0};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = size;
    buffer_info.usage = usage;
    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(g_device, &buffer_info, NULL, buffer) != VK_SUCCESS)
    {
        log_error("Failed to create vertex buffer");
    }

    VkMemoryRequirements mem_requirements;
    vkGetBufferMemoryRequirements(g_device, *buffer, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(g_device, &alloc_info, NULL, buffer_memory) != VK_SUCCESS)
    {
        log_error("Failed to allocate vertex buffer memory");
    }

    vkBindBufferMemory(g_device, *buffer, *buffer_memory, 0);
}

static VkCommandBuffer begin_single_time_commands()
{
    VkCommandBufferAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandPool = g_command_pool;
    alloc_info.commandBufferCount = 1;

    VkCommandBuffer command_buffer;
    vkAllocateCommandBuffers(g_device, &alloc_info, &command_buffer);

    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(command_buffer, &begin_info);

    return command_buffer;
}

static void end_single_time_commands(VkCommandBuffer command_buffer)
{
    vkEndCommandBuffer(command_buffer);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffer;

    vkQueueSubmit(g_graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(g_graphics_queue);

    vkFreeCommandBuffers(g_device, g_command_pool, 1, &command_buffer);
}

static void copy_buffer(VkBuffer src_buffer, VkBuffer dst_buffer, VkDeviceSize size)
{
    VkCommandBuffer command_buffer = begin_single_time_commands();

    VkBufferCopy copy_region = {0};
    copy_region.size = size;
    vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &copy_region);

    end_single_time_commands(command_buffer);
}

static void transition_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout)
{
    VkCommandBuffer command_buffer = begin_single_time_commands();

    VkImageMemoryBarrier barrier = {0};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags source_stage;
    VkPipelineStageFlags destination_stage;
    if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        source_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destination_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
             new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        source_stage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destination_stage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        log_error("Unsupported layout transition");
    }

    vkCmdPipelineBarrier(command_buffer, source_stage, destination_stage, 0, 0, NULL, 0, NULL, 1, &barrier);

    end_single_time_commands(command_buffer);
}

static void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer command_buffer = begin_single_time_commands();

    VkBufferImageCopy region = {0};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = (VkOffset3D){0, 0, 0};
    region.imageExtent = (VkExtent3D){width, height, 1};

    vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    end_single_time_commands(command_buffer);
}

static void create_vertex_buffer()
{
    VkDeviceSize buffer_size = sizeof(vertex_t) * 4;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer,
                  &staging_buffer_memory);

    void *data;
    vkMapMemory(g_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, g_vertices, (size_t)buffer_size);
    vkUnmapMemory(g_device, staging_buffer_memory);

    create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_vertex_buffer,
                  &g_vertex_buffer_memory);

    copy_buffer(staging_buffer, g_vertex_buffer, buffer_size);

    vkDestroyBuffer(g_device, staging_buffer, NULL);
    vkFreeMemory(g_device, staging_buffer_memory, NULL);
}

void create_index_buffer()
{
    VkDeviceSize buffer_size = sizeof(g_indices[0]) * 6;

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;
    create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer,
                  &staging_buffer_memory);

    void *data;
    vkMapMemory(g_device, staging_buffer_memory, 0, buffer_size, 0, &data);
    memcpy(data, g_indices, (size_t)buffer_size);
    vkUnmapMemory(g_device, staging_buffer_memory);

    create_buffer(buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &g_index_buffer, &g_index_buffer_memory);

    copy_buffer(staging_buffer, g_index_buffer, buffer_size);

    vkDestroyBuffer(g_device, staging_buffer, NULL);
    vkFreeMemory(g_device, staging_buffer_memory, NULL);
}

static void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index)
{
    VkCommandBufferBeginInfo begin_info = {0};
    begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begin_info.flags = 0;
    begin_info.pInheritanceInfo = NULL;

    if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS)
    {
        log_error("Failed to begin recording command buffer");
    }

    VkRenderPassBeginInfo render_pass_info = {0};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass = g_render_pass;
    render_pass_info.framebuffer = g_swap_chain_framebuffers[image_index];
    render_pass_info.renderArea.offset = (VkOffset2D){0, 0};
    render_pass_info.renderArea.extent = g_swap_chain_extent;

    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_info.clearValueCount = 1;
    render_pass_info.pClearValues = &clear_color;

    vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_graphics_pipeline);

    VkBuffer vertex_buffers[] = {g_vertex_buffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, vertex_buffers, offsets);

    vkCmdBindIndexBuffer(command_buffer, g_index_buffer, 0, VK_INDEX_TYPE_UINT16);

    VkViewport viewport = {0};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = g_swap_chain_extent.width;
    viewport.height = g_swap_chain_extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(command_buffer, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset = (VkOffset2D){0, 0};
    scissor.extent = g_swap_chain_extent;
    vkCmdSetScissor(command_buffer, 0, 1, &scissor);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, g_pipeline_layout, 0, 1,
                            &g_descriptor_sets[g_current_frame], 0, NULL);

    vkCmdDrawIndexed(command_buffer, 6, 1, 0, 0, 0);

    vkCmdEndRenderPass(command_buffer);

    if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
    {
        log_error("Failed to record command buffer");
    }
}

static void create_sync_objects()
{
    VkSemaphoreCreateInfo semaphore_info = {0};
    semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_info = {0};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(g_device, &semaphore_info, NULL, &g_image_available_semaphores[i]) != VK_SUCCESS)
        {
            log_error("Failed to create image available semephore");
        }
        if (vkCreateSemaphore(g_device, &semaphore_info, NULL, &g_render_finished_semaphores[i]) != VK_SUCCESS)
        {
            log_error("Failed to create render finished semephore");
        }
        if (vkCreateFence(g_device, &fence_info, NULL, &g_in_flight_fences[i]) != VK_SUCCESS)
        {
            log_error("Failed to create in flight fence");
        }
    }
}

static void cleanup_swap_chain()
{
    for (size_t i = 0; i < g_swap_chain_image_count; i++)
    {
        vkDestroyFramebuffer(g_device, g_swap_chain_framebuffers[i], NULL);
    }
    free(g_swap_chain_framebuffers);
    g_swap_chain_framebuffers = NULL;
    for (size_t i = 0; i < g_swap_chain_image_count; i++)
    {
        vkDestroyImageView(g_device, g_swap_chain_image_views[i], NULL);
    }
    free(g_swap_chain_image_views);
    g_swap_chain_image_views = NULL;
    vkDestroySwapchainKHR(g_device, g_swap_chain, NULL);
}

static void recreate_swap_chain()
{
    /* vulkan-tutorial code to possibly implement */
    // int width = 0, height = 0;
    // glfwGetFramebufferSize(window, &width, &height);
    // while (width == 0 || height == 0) {
    // glfwGetFramebufferSize(window, &width, &height);
    // glfwWaitEvents();
    // }
    vkDeviceWaitIdle(g_device);

    cleanup_swap_chain();

    create_swap_chain();
    create_image_views();
    create_framebuffers();
}

static void create_descriptor_set_layout()
{
    VkDescriptorSetLayoutBinding ubo_layout_binding = {0};
    ubo_layout_binding.binding = 0;
    ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_layout_binding.descriptorCount = 1;
    ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    ubo_layout_binding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutBinding sampler_layout_binding = {0};
    sampler_layout_binding.binding = 1;
    sampler_layout_binding.descriptorCount = 1;
    sampler_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    sampler_layout_binding.pImmutableSamplers = NULL;
    sampler_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding bindings[] = {ubo_layout_binding, sampler_layout_binding};

    VkDescriptorSetLayoutCreateInfo layout_info = {0};
    layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_info.bindingCount = 2;
    layout_info.pBindings = bindings;

    if (vkCreateDescriptorSetLayout(g_device, &layout_info, NULL, &g_descriptor_set_layout) != VK_SUCCESS)
    {
        log_error("Failed to create descriptor set layout");
    }
}

void create_uniform_buffers()
{
    VkDeviceSize buffer_size = sizeof(uniform_buffer_object);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        create_buffer(buffer_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &g_uniform_buffers[i],
                      &g_uniform_buffers_memory[i]);
        vkMapMemory(g_device, g_uniform_buffers_memory[i], 0, buffer_size, 0, &g_uniform_buffers_mapped[i]);
    }
}

void create_descriptor_pool()
{
    VkDescriptorPoolSize pool_size[2] = {0};
    pool_size[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_size[0].descriptorCount = MAX_FRAMES_IN_FLIGHT;
    pool_size[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_size[1].descriptorCount = MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo pool_info = {0};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.poolSizeCount = 2;
    pool_info.pPoolSizes = pool_size;
    pool_info.maxSets = MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(g_device, &pool_info, NULL, &g_descriptor_pool) != VK_SUCCESS)
    {
        log_error("Failed to create descriptor pool");
    }
}

void create_descriptor_sets()
{
    VkDescriptorSetLayout layouts[MAX_FRAMES_IN_FLIGHT];
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        layouts[i] = g_descriptor_set_layout;
    }

    VkDescriptorSetAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.descriptorPool = g_descriptor_pool;
    alloc_info.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
    alloc_info.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(g_device, &alloc_info, g_descriptor_sets) != VK_SUCCESS)
    {
        log_error("Failed to allocate descriptor sets");
    }

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo buffer_info = {0};
        buffer_info.buffer = g_uniform_buffers[i];
        buffer_info.offset = 0;
        buffer_info.range = sizeof(uniform_buffer_object);

        VkDescriptorImageInfo image_info = {0};
        image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info.imageView = g_texture_image_view;
        image_info.sampler = g_texture_sampler;

        VkWriteDescriptorSet descriptor_writes[2] = {0};
        descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[0].dstSet = g_descriptor_sets[i];
        descriptor_writes[0].dstBinding = 0;
        descriptor_writes[0].dstArrayElement = 0;
        descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_writes[0].descriptorCount = 1;
        descriptor_writes[0].pBufferInfo = &buffer_info;
        descriptor_writes[0].pImageInfo = NULL;
        descriptor_writes[0].pTexelBufferView = NULL;

        descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[1].dstSet = g_descriptor_sets[i];
        descriptor_writes[1].dstBinding = 1;
        descriptor_writes[1].dstArrayElement = 0;
        descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[1].descriptorCount = 1;
        descriptor_writes[1].pImageInfo = &image_info;

        vkUpdateDescriptorSets(g_device, 2, descriptor_writes, 0, NULL);
    }
}

void create_image(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkImage *image, VkDeviceMemory *image_memory)
{
    VkImageCreateInfo image_info = {0};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.flags = 0;

    if (vkCreateImage(g_device, &image_info, NULL, image) != VK_SUCCESS)
    {
        log_error("Failed to create image");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(g_device, *image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {0};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = find_memory_type(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(g_device, &alloc_info, NULL, image_memory) != VK_SUCCESS)
    {
        log_error("Failed to allocate image memory");
    }

    vkBindImageMemory(g_device, *image, *image_memory, 0);
}

void create_texture_image()
{
    int tex_width, tex_height, tex_channels;
    stbi_uc *pixels =
        stbi_load("resources/textures/texture.jpg", &tex_width, &tex_height, &tex_channels, STBI_rgb_alpha);
    VkDeviceSize image_size = tex_width * tex_height * 4;

    if (!pixels)
    {
        log_error("Failed to load texture image");
    }

    VkBuffer staging_buffer;
    VkDeviceMemory staging_buffer_memory;

    create_buffer(image_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &staging_buffer,
                  &staging_buffer_memory);

    void *data;
    vkMapMemory(g_device, staging_buffer_memory, 0, image_size, 0, &data);
    memcpy(data, pixels, (size_t)image_size);
    vkUnmapMemory(g_device, staging_buffer_memory);

    stbi_image_free(pixels);

    create_image(tex_width, tex_height, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
                 VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                 &g_texture_image, &g_texture_image_memory);

    transition_image_layout(g_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copy_buffer_to_image(staging_buffer, g_texture_image, (uint32_t)tex_width, (uint32_t)tex_height);

    transition_image_layout(g_texture_image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(g_device, staging_buffer, NULL);
    vkFreeMemory(g_device, staging_buffer_memory, NULL);
}

static void create_texture_image_view()
{
    g_texture_image_view = create_image_view(g_texture_image, VK_FORMAT_R8G8B8A8_SRGB);
}

static void create_texture_sampler()
{
    VkSamplerCreateInfo sampler_info = {0};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.magFilter = VK_FILTER_LINEAR;
    sampler_info.minFilter = VK_FILTER_LINEAR;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties = {0};
    vkGetPhysicalDeviceProperties(g_physical_device, &properties);
    sampler_info.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    sampler_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    sampler_info.unnormalizedCoordinates = VK_FALSE;
    sampler_info.compareEnable = VK_FALSE;
    sampler_info.compareOp = VK_COMPARE_OP_ALWAYS;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;

    if (vkCreateSampler(g_device, &sampler_info, NULL, &g_texture_sampler) != VK_SUCCESS)
    {
        log_error("Failed to create texture sampler");
    }
}

bool cstrl_renderer_init(cstrl_platform_state *platform_state)
{
    g_instance = create_instance();
#if defined(CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS)
    setup_debug_messenger();
#endif
    if (!cstrl_vulkan_platform_init(platform_state, g_instance))
    {
        return false;
    }
    pick_physical_device();
    create_logical_device();
    create_swap_chain();
    create_image_views();
    create_render_pass();
    create_descriptor_set_layout();
    create_graphics_pipeline();
    create_framebuffers();
    create_command_pool();
    create_texture_image();
    create_texture_image_view();
    create_texture_sampler();
    create_vertex_buffer();
    create_index_buffer();
    create_uniform_buffers();
    create_descriptor_pool();
    create_descriptor_sets();
    create_command_buffers();
    create_sync_objects();
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

static void update_uniform_buffer(uint32_t current_image)
{
    double current_time = cstrl_platform_get_absolute_time();
    // double time = current_time - start_time;
    // start_time = current_time;
    uniform_buffer_object ubo = {0};
    ubo.model = cstrl_mat4_identity();
    ubo.model = cstrl_mat4_rotate(ubo.model, sin(current_time), (vec3){0.0f, 0.0f, 1.0f});
    ubo.view = cstrl_mat4_look_at((vec3){2.0f, 2.0f, 2.0f}, (vec3){0.0f, 0.0f, 0.0f}, (vec3){0.0f, 0.0f, 1.0f});
    ubo.projection = cstrl_mat4_perspective(
        cstrl_pi / 4.0f, (float)g_swap_chain_extent.width / (float)g_swap_chain_extent.height, 0.1f, 10.0f);
    ubo.projection.yy *= -1.0f;

    memcpy(g_uniform_buffers_mapped[current_image], &ubo, sizeof(ubo));
}

void cstrl_renderer_draw(cstrl_render_data *data)
{
    vkWaitForFences(g_device, 1, &g_in_flight_fences[g_current_frame], VK_TRUE, UINT64_MAX);

    uint32_t image_index;
    VkResult result =
        vkAcquireNextImageKHR(g_device, g_swap_chain, UINT64_MAX, g_image_available_semaphores[g_current_frame],
                              VK_NULL_HANDLE, &image_index);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || g_framebuffer_resized)
    {
        g_framebuffer_resized = false;
        recreate_swap_chain();
    }
    else if (result != VK_SUCCESS)
    {
        log_error("Failed to acquire swap chain image");
        return;
    }

    update_uniform_buffer(g_current_frame);

    vkResetFences(g_device, 1, &g_in_flight_fences[g_current_frame]);

    vkResetCommandBuffer(g_command_buffers[g_current_frame], 0);

    record_command_buffer(g_command_buffers[g_current_frame], image_index);

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = {g_image_available_semaphores[g_current_frame]};
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &g_command_buffers[g_current_frame];
    VkSemaphore signal_semaphores[] = {g_render_finished_semaphores[g_current_frame]};
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = signal_semaphores;

    if (vkQueueSubmit(g_graphics_queue, 1, &submit_info, g_in_flight_fences[g_current_frame]) != VK_SUCCESS)
    {
        log_error("Failed to submit draw command buffer");
    }

    VkPresentInfoKHR present_info = {0};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = signal_semaphores;

    VkSwapchainKHR swap_chains[] = {g_swap_chain};
    present_info.swapchainCount = 1;
    present_info.pSwapchains = swap_chains;
    present_info.pImageIndices = &image_index;
    present_info.pResults = NULL;

    result = vkQueuePresentKHR(g_present_queue, &present_info);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || g_framebuffer_resized)
    {
        g_framebuffer_resized = false;
        recreate_swap_chain();
    }
    else if (result != VK_SUCCESS)
    {
        log_error("Failed to acquire swap chain image");
        return;
    }

    g_current_frame = (g_current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void cstrl_renderer_shutdown(cstrl_platform_state *platform_state)
{
    vkDeviceWaitIdle(g_device);

    cleanup_swap_chain();

    vkDestroySampler(g_device, g_texture_sampler, NULL);

    vkDestroyImageView(g_device, g_texture_image_view, NULL);

    vkDestroyImage(g_device, g_texture_image, NULL);
    vkFreeMemory(g_device, g_texture_image_memory, NULL);

    vkDestroyPipeline(g_device, g_graphics_pipeline, NULL);
    vkDestroyPipelineLayout(g_device, g_pipeline_layout, NULL);
    vkDestroyRenderPass(g_device, g_render_pass, NULL);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroyBuffer(g_device, g_uniform_buffers[i], NULL);
        vkFreeMemory(g_device, g_uniform_buffers_memory[i], NULL);
    }

    vkDestroyDescriptorPool(g_device, g_descriptor_pool, NULL);
    // free(g_descriptor_sets);
    vkDestroyDescriptorSetLayout(g_device, g_descriptor_set_layout, NULL);

    vkDestroyBuffer(g_device, g_index_buffer, NULL);
    vkFreeMemory(g_device, g_index_buffer_memory, NULL);

    vkDestroyBuffer(g_device, g_vertex_buffer, NULL);
    vkFreeMemory(g_device, g_vertex_buffer_memory, NULL);

    free(g_swap_chain_images);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(g_device, g_image_available_semaphores[i], NULL);
        vkDestroySemaphore(g_device, g_render_finished_semaphores[i], NULL);
        vkDestroyFence(g_device, g_in_flight_fences[i], NULL);
    }

    vkDestroyCommandPool(g_device, g_command_pool, NULL);

    vkDestroyDevice(g_device, NULL);

#if defined(CSTRL_VULKAN_ENABLE_VALIDATION_LAYERS)
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT =
        (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(g_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (vkDestroyDebugUtilsMessengerEXT == NULL)
    {
        log_error("Failed to setup destroy debug messenger");
    }
    else
    {
        vkDestroyDebugUtilsMessengerEXT(g_instance, g_debug_messenger, NULL);
    }
#endif
    cstrl_vulkan_platform_destroy(platform_state, g_instance);
    vkDestroyInstance(g_instance, NULL);
}

void cstrl_renderer_swap_buffers(cstrl_platform_state *platform_state)
{
}

#endif
