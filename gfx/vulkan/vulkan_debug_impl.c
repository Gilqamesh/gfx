static bool vk__check_if_validation_layer_is_available(const char* validation_layer_name);
static VKAPI_ATTR VkBool32 VKAPI_CALL vk__debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
    VkDebugUtilsMessageTypeFlagsEXT             message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void*                                       user_data
);
static const char* vk__debug_message_severity__to_str(VkDebugUtilsMessageSeverityFlagBitsEXT severity);
static const char* vk__debug_message_type__to_str(VkDebugUtilsMessageTypeFlagsEXT type);
static VkResult vk__create_debug_utils_messenger_ext(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* messenger_create_info,
    const VkAllocationCallbacks*              allocator,
    VkDebugUtilsMessengerEXT*                 debug_messenger
);
static void vk__destroy_debug_utils_messenger_ext(
    VkInstance                                instance,
    VkDebugUtilsMessengerEXT                  debug_messenger,
    const VkAllocationCallbacks*              allocator
);
static void vk__populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT* messenger_create_info);
static bool vk__setup_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT* messenger_create_info);

static bool vk__check_if_validation_layer_is_available(const char* validation_layer_name) {
    uint32_t vk_layer_properties_count = 0;
    vkEnumerateInstanceLayerProperties(&vk_layer_properties_count, 0);
    VkLayerProperties* vk_layer_properties = malloc(vk_layer_properties_count * sizeof(*vk_layer_properties));
    vkEnumerateInstanceLayerProperties(&vk_layer_properties_count, vk_layer_properties);
    bool layer_found = false;
    debug__lock();

    debug__writeln("validation layers:");
    for (uint32_t vk_layer_properties_index = 0; vk_layer_properties_index < vk_layer_properties_count; ++vk_layer_properties_index) {
        debug__writeln("%s", vk_layer_properties[vk_layer_properties_index].layerName);
        if (strcmp(vk_layer_properties[vk_layer_properties_index].layerName, validation_layer_name) == 0) {
            layer_found = true;
            break ;
        }
    }
    free(vk_layer_properties);
    debug__flush(DEBUG_MODULE_VULKAN, DEBUG_WARN);

    debug__unlock();

    return layer_found;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL vk__debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT      message_severity,
    VkDebugUtilsMessageTypeFlagsEXT             message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void*                                       user_data
) {
    (void) user_data;

    const char* message_severity_str        = vk__debug_message_severity__to_str(message_severity);
    const uint32_t message_severity_str_len = strlen(message_severity_str);
    const char* message_type_str            = vk__debug_message_type__to_str(message_type);
    const uint32_t message_type_str_len     = strlen(message_type_str);
    const uint32_t message_str_len          = strlen(callback_data->pMessage);
    uint32_t max_str_len = MAX(MAX(message_severity_str_len, message_type_str_len), message_str_len);
    for (uint32_t vk_obj_index = 0; vk_obj_index < callback_data->objectCount; ++vk_obj_index) {
        if (callback_data->pObjects[vk_obj_index].pObjectName) {
            max_str_len = MAX(max_str_len, strlen(callback_data->pObjects[vk_obj_index].pObjectName));
        }
    }

    debug__lock();

    debug__writeln("severity:   %-*.*s", max_str_len, max_str_len, message_severity_str);
    debug__writeln("type:       %-*.*s", max_str_len, max_str_len, message_type_str);
    debug__writeln("message:    %-*.*s", max_str_len, max_str_len, callback_data->pMessage);
    debug__writeln("vk objects:");
    for (uint32_t vk_obj_index = 0; vk_obj_index < callback_data->objectCount; ++vk_obj_index) {
        debug__writeln("    %-*.*s", callback_data->pObjects[vk_obj_index].pObjectName);
    }
    debug__flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR);

    debug__unlock();

    return VK_FALSE;
}

static const char* vk__debug_message_severity__to_str(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
    switch (severity) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT: return "Diagnostic";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:    return "Informational";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT: return "Warning";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:   return "Error";
    default: ASSERT(false);
    }
}

static const char* vk__debug_message_type__to_str(VkDebugUtilsMessageTypeFlagsEXT type) {
    switch (type) {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:     return "Some event has happened that is unrelated to the specification or performance";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:  return "Something has happened that violates the specification or indicates a possible mistake";
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT: return "Potential non-optimal use of Vulkan";
    default: ASSERT(false);
    }
}

static VkResult vk__create_debug_utils_messenger_ext(
    VkInstance                                instance,
    const VkDebugUtilsMessengerCreateInfoEXT* messenger_create_info,
    const VkAllocationCallbacks*              allocator,
    VkDebugUtilsMessengerEXT*                 debug_messenger
) {
    const char* fn_str = "vkCreateDebugUtilsMessengerEXT";
    PFN_vkCreateDebugUtilsMessengerEXT fn = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, fn_str);
    if (!fn) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    } else {
        return fn(instance, messenger_create_info, allocator, debug_messenger);
    }
}

static void vk__destroy_debug_utils_messenger_ext(
    VkInstance                                instance,
    VkDebugUtilsMessengerEXT                  debug_messenger,
    const VkAllocationCallbacks*              allocator
) {
    const char* fn_str = "vkDestroyDebugUtilsMessengerEXT";
    PFN_vkDestroyDebugUtilsMessengerEXT fn = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, fn_str);
    if (fn) {
        fn(instance, debug_messenger, allocator);
    }
}

static void vk__populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT* messenger_create_info) {
    memset(messenger_create_info, 0, sizeof(*messenger_create_info));
    messenger_create_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    messenger_create_info->messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        // VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    messenger_create_info->messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    messenger_create_info->pfnUserCallback = &vk__debug_callback;
}

static bool vk__setup_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT* messenger_create_info) {
    if (vk__create_debug_utils_messenger_ext(vk.instance, messenger_create_info, 0, &vk.debug_messenger) != VK_SUCCESS) {
        debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR, "failed to create debug messenger");
        return false;
    }

    return true;
}
