typedef struct vulkan {
    VkInstance instance;

    uint32_t     required_extensions_top;
    uint32_t     required_extensions_size;
    const char** required_extensions;

#if defined(DEBUG)
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
} vulkan_t;

static vulkan_t vk;

static bool vk__check_if_validation_layer_is_available(const char* validation_layer_name);
static bool vk__get_required_extensions();
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
    VkAllocationCallbacks*                    allocator,
    VkDebugUtilsMessengerEXT*                 debug_messenger
);

static bool vk__check_if_validation_layer_is_available(const char* validation_layer_name) {
    uint32_t vk_layer_properties_count = 0;
    vkEnumerateInstanceLayerProperties(&vk_layer_properties_count, 0);
    VkLayerProperties* vk_layer_properties = malloc(vk_layer_properties_count * sizeof(*vk_layer_properties));
    vkEnumerateInstanceLayerProperties(&vk_layer_properties_count, vk_layer_properties);
    bool layer_found = false;
    debug__write("validation layers:");
    for (uint32_t vk_layer_properties_index = 0; vk_layer_properties_index < vk_layer_properties_count; ++vk_layer_properties_index) {
        debug__write("%s", vk_layer_properties[vk_layer_properties_index].layerName);
        if (strcmp(vk_layer_properties[vk_layer_properties_index].layerName, validation_layer_name) == 0) {
            layer_found = true;
            break ;
        }
    }
    free(vk_layer_properties);
    debug__flush(DEBUG_MODULE_VULKAN, DEBUG_WARN);

    return layer_found;
}

static bool vk__get_required_extensions() {
    uint32_t glfw_extensions_count = 0;
    const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extensions_count);
    if (glfw_extensions_count == 0) {
        return false;
    }

    for (uint32_t glfw_extensions_index = 0; glfw_extensions_index < glfw_extensions_count; ++glfw_extensions_index) {
        ARRAY_ENSURE_TOP(vk.required_extensions, vk.required_extensions_top, vk.required_extensions_size);
        vk.required_extensions[vk.required_extensions_top++] = glfw_extensions[glfw_extensions_index];
    }

#if defined(DEBUG)

    // add message callback for validation layer
    ARRAY_ENSURE_TOP(vk.required_extensions, vk.required_extensions_top, vk.required_extensions_size);
    vk.required_extensions[vk.required_extensions_top++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

    // check which Vulkan extensions are not supported by GLFW
    uint32_t supported_vk_extension_count = 0;
    vkEnumerateInstanceExtensionProperties(0, &supported_vk_extension_count, 0);
    VkExtensionProperties* supported_vk_extensions = malloc(supported_vk_extension_count * sizeof(*supported_vk_extensions));
    vkEnumerateInstanceExtensionProperties(0, &supported_vk_extension_count, supported_vk_extensions);
    debug__write("unsupported Vulkan extensions by GLFW:");
    bool found_unsupported_extension = false;
    for (uint32_t supported_vk_extension_index = 0; supported_vk_extension_index < supported_vk_extension_count; ++supported_vk_extension_index) {
        bool found = false;
        for (uint32_t glfw_supported_vk_extension_index = 0; glfw_supported_vk_extension_index < glfw_extensions_count; ++glfw_supported_vk_extension_index) {
            if (strcmp(supported_vk_extensions[supported_vk_extension_index].extensionName, glfw_extensions[glfw_supported_vk_extension_index]) == 0) {
                found = true;
                break ;
            }
        }
        if (!found) {
            found_unsupported_extension = true;
            debug__write("%s", supported_vk_extensions[supported_vk_extension_index].extensionName);
        }
    }
    if (!found_unsupported_extension) {
        debug__write("---");
    }
    debug__flush(DEBUG_MODULE_VULKAN, DEBUG_WARN);
    free(supported_vk_extensions);
#endif

    return true;
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
        max_str_len = MAX(max_str_len, strlen(callback_data->pObjects[vk_obj_index].pObjectName));
    }

    debug__write("severity:   %-*.*s");
    debug__write("type:       %-*.*s");
    debug__write("message:    %-*.*s");
    debug__write("vk objects: %-*.*s");
    for (uint32_t vk_obj_index = 0; vk_obj_index < callback_data->objectCount; ++vk_obj_index) {
        debug__write("    %-*.*s", callback_data->pObjects[vk_obj_index].pObjectName);
    }
    debug__flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR);

    return VK_TRUE;
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
    VkAllocationCallbacks*                    allocator,
    VkDebugUtilsMessengerEXT*                 debug_messenger
) {
    PFN_vkCreateDebugUtilsMessengerEXT fn = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (!fn) {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    } else {
        return fn(instance, messenger_create_info, allocator, debug_messenger);
    }
}
