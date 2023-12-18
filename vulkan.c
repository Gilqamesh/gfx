#include "vulkan.h"

#include "debug.h"
#include "helper_macros.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <string.h>

#include "vulkan_internal.c"

static const char* vk_validation_layers[] = { "VK_LAYER_KHRONOS_validation" };

bool vk__init() {
    memset(&vk, 0, sizeof(vk));

    vk__get_required_extensions();

    /**
     * Optional, but provides useful info to the driver in order to optimize our specific application
    */
    VkApplicationInfo app_info = { 0 };
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = "Todo: fill in application name here, whatever this is";
    app_info.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.pEngineName        = "Todo: fill in engine name here, whatever this is";
    app_info.engineVersion      = VK_MAKE_API_VERSION(0, 1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_0;

    /**
     * Mandatory, tells the driver, which global (applies to entire program, not just to a specific device)
     * extensions and validation layers we want to use
    */
    VkInstanceCreateInfo create_info = { 0 };
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;
    create_info.enabledExtensionCount = vk.required_extensions_top;
    create_info.ppEnabledExtensionNames = vk.required_extensions;

    const uint32_t vk_validation_layers_count = ARRAY_SIZE(vk_validation_layers);
    if (vk__check_if_validation_layer_is_available(vk_validation_layers[0])) {
        create_info.enabledLayerCount = vk_validation_layers_count;
        create_info.ppEnabledLayerNames = vk_validation_layers;
    } else {
#if defined(DEBUG)
        debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR, "VK_LAYER_KHRONOS_validation was not found in the supported validation layers in debug build");
        return false;
#endif
    }

    VkResult create_instance_result = vkCreateInstance(&create_info, 0, &vk.instance);
    if (create_instance_result != VK_SUCCESS) {
        debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR, "failed to create an instance");
        return false;
    }

    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "instance successfully created");

#if defined(DEBUG)
    // create debug messenger callback
    VkDebugUtilsMessengerCreateInfoEXT messenger_create_info = { 0 };
    messenger_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messenger_create_info.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
    messenger_create_info.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messenger_create_info.pfnUserCallback = &vk__debug_callback;
    messenger_create_info.pUserData = 0;
    if (vk__create_debug_utils_messenger_ext(vk.instance, &messenger_create_info, 0, &vk.debug_messenger) != VK_SUCCESS) {
        debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR, "failed to create debug messenger");
        return false;
    }
#endif

    return true;
}

void vk__deinit() {
    vkDestroyInstance(vk.instance, 0);
}
