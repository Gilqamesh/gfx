// todo: add glsl -> spir-v compiler

struct         vulkan;
struct         swapchain;
struct         queue_family;
struct         queue_families;
struct         swapchain_support_details;
struct         shader_code;
typedef struct swapchain                 swapchain_t;
typedef struct vulkan                    vulkan_t;
typedef struct queue_family              queue_family_t;
typedef struct queue_families            queue_families_t;
typedef struct swapchain_support_details swapchain_support_details_t;
typedef struct shader_code               shader_code_t;

struct swapchain {
    VkSwapchainKHR     _;

    uint32_t           images_size;
    VkImage*           images;
    VkFormat           format;
    VkExtent2D         extent;
};

struct vulkan {
    VkInstance                      instance;
    VkSurfaceKHR                    surface; // not necessary for off-screen rendering
    VkPhysicalDevice                physical_device; // implicitly destroyed with VkInstance
    VkDevice                        logical_device;
    VkQueue                         graphics_queue; // implicitly destroyed with the VkDevice
    VkQueue                         presentation_queue; // does not necessarily come from the same family as graphics_queue
    swapchain_t                     sc;
    uint32_t                        image_views_size;
    VkImageView*                    image_views;
    uint32_t                        shader_stage_creation_infos_top;
    VkPipelineShaderStageCreateInfo shader_stage_creation_infos[8];
    VkRenderPass                    render_pass;
    VkPipelineLayout                pipeline_layout;
    VkPipeline                      gfx_pipeline;
    uint32_t                        framebuffers_size;
    VkFramebuffer*                  framebuffers;
    VkCommandPool                   command_pool;
    VkCommandBuffer                 command_buffer;
    VkSemaphore                     semaphore_sc_image_available;
    VkSemaphore                     semaphore_render_finished;
    VkFence                         fence_present_finished;

    uint32_t                        required_extensions_top;
    uint32_t                        required_extensions_size;
    const char**                    required_extensions;

#if defined(DEBUG)
    VkDebugUtilsMessengerEXT        debug_messenger;
#endif
};

struct queue_family {
    uint32_t index;
    bool     exists;
};

struct queue_families {
    // note: these 2 do not necessarily overlap
    queue_family_t graphics_family;
    queue_family_t presentation_family;
};

struct swapchain_support_details {
    VkSurfaceCapabilitiesKHR capabilities;

    uint32_t                 formats_size;
    VkSurfaceFormatKHR*      formats;

    uint32_t                 present_modes_size;
    VkPresentModeKHR*        present_modes;
};

struct shader_code {
    void*    code;
    uint32_t code_size;
};

static vulkan_t vk;

#if defined(DEBUG)
static const char* vk_validation_layers[] = { "VK_LAYER_KHRONOS_validation" };
# include "vulkan_debug_impl.c"
#endif

static const char* vk_required_logical_device_extensions[] = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static bool vk__init(GLFWwindow* glfw_window);
static void vk__deinit();
static void vk__render();
static bool vk__init_create_instance();
static bool vk__init_setup_debug_messenger();
static bool vk__get_required_extensions();
static bool vk__create_surface(GLFWwindow* glfw_window);
static bool vk__pick_physical_device();
static bool vk__is_physical_device_suitable(VkPhysicalDevice device);
static queue_families_t vk__find_queue_families(VkPhysicalDevice device);
static bool vk__check_device_required_extensions(VkPhysicalDevice device);
static bool vk__create_logical_device();
static bool vk__create_swapchain(GLFWwindow* glfw_window);
static bool vk__create_image_views();
static bool vk__create_render_pass();
static bool vk__create_gfx_pipeline();
static bool vk__create_shader_module(VkShaderModule* self, shader_code_t* shader_code);
static bool vk__create_framebuffers();
static bool vk__create_command_pool();
static bool vk__create_command_buffer();
static bool vk__create_sync_objects();

static swapchain_support_details_t swapchain_support_details__create(VkPhysicalDevice physical_device);
static void swapchain_support_details__destroy(swapchain_support_details_t* self);

static bool shader_code__create_from_file(shader_code_t* self, const char* shader_file_path);
static void shader_code__destroy(shader_code_t* self);

VkSurfaceFormatKHR swapchain__pick_surface_format(VkSurfaceFormatKHR* surface_formats, uint32_t surface_formats_size);
VkPresentModeKHR swapchain__pick_present_mode(VkPresentModeKHR* present_modes, uint32_t present_modes_size);
VkExtent2D swapchain__pick_extent(VkSurfaceCapabilitiesKHR* capabilities, GLFWwindow* glfw_window);

static bool queue_families__is_complete(queue_families_t* self);

static bool cb__record(VkCommandBuffer self, uint32_t image_index);

bool vk__init(GLFWwindow* glfw_window) {
    memset(&vk, 0, sizeof(vk));

    if (!vk__init_create_instance()) {
        return false;
    }

    if (!vk__init_setup_debug_messenger()) {
        return false;
    }

    if (!vk__create_surface(glfw_window)) {
        return false;
    }

    if (!vk__pick_physical_device()) {
        return false;
    }

    if (!vk__create_logical_device()) {
        return false;
    }

    if (!vk__create_swapchain(glfw_window)) {
        return false;
    }

    if (!vk__create_image_views()) {
        return false;
    }

    if (!vk__create_render_pass()) {
        return false;
    }

    if (!vk__create_gfx_pipeline()) {
        return false;
    }

    if (!vk__create_framebuffers()) {
        return false;
    }

    if (!vk__create_command_pool()) {
        return false;
    }

    if (!vk__create_command_buffer()) {
        return false;
    }

    if (!vk__create_sync_objects()) {
        return false;
    }

    return true;
}

void vk__deinit() {
#if defined(DEBUG)
    vk__destroy_debug_utils_messenger_ext(vk.instance, vk.debug_messenger, 0);
#endif

    vkDeviceWaitIdle(vk.logical_device);

    vkDestroySemaphore(vk.logical_device, vk.semaphore_sc_image_available, 0);
    vkDestroySemaphore(vk.logical_device, vk.semaphore_render_finished, 0);
    vkDestroyFence(vk.logical_device, vk.fence_present_finished, 0);

    vkDestroyCommandPool(vk.logical_device, vk.command_pool, 0);

    for (uint32_t framebuffer_index = 0; framebuffer_index < vk.framebuffers_size; ++framebuffer_index) {
        vkDestroyFramebuffer(vk.logical_device, vk.framebuffers[framebuffer_index], 0);
    }

    vkDestroyPipeline(vk.logical_device, vk.gfx_pipeline, 0);

    vkDestroyPipelineLayout(vk.logical_device, vk.pipeline_layout, 0);

    vkDestroyRenderPass(vk.logical_device, vk.render_pass, 0);

    for (uint32_t image_view_index = 0; image_view_index < vk.image_views_size; ++image_view_index) {
        vkDestroyImageView(vk.logical_device, vk.image_views[image_view_index], 0);
    }

    vkDestroySwapchainKHR(vk.logical_device, vk.sc._, 0);

    vkDestroyDevice(vk.logical_device, 0);

    vkDestroySurfaceKHR(vk.instance, vk.surface, 0);

    vkDestroyInstance(vk.instance, 0);

    if (vk.required_extensions) {
        free(vk.required_extensions);
    }

    if (vk.sc.images) {
        free(vk.sc.images);
    }

    if (vk.image_views) {
        free(vk.image_views);
    }

    if (vk.framebuffers) {
        free(vk.framebuffers);
    }
}

void vk__render() {
    // Wait for the previous frame to finish
    vkWaitForFences(vk.logical_device, 1, &vk.fence_present_finished, VK_TRUE, UINT64_MAX);
    vkResetFences(vk.logical_device, 1, &vk.fence_present_finished);

    // Acquire image from the swapchain
    uint32_t image_index;
    if (vkAcquireNextImageKHR(vk.logical_device, vk.sc._, UINT64_MAX, vk.semaphore_sc_image_available, VK_NULL_HANDLE, &image_index) != VK_SUCCESS) {
        return ;
    }

    // Record command buffer
    vkResetCommandBuffer(vk.command_buffer, 0);
    if (!cb__record(vk.command_buffer, image_index)) {
        return ;
    }

    // Submit the recorded command buffer
    VkSubmitInfo submit_info = { 0 };
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore wait_semaphores[] = { vk.semaphore_sc_image_available };
    VkPipelineStageFlags pipeline_state_flags[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = wait_semaphores;

    // note: describe which stage(s) of the pipeline to wait for, each entry in this array corresponds to the one supplied for pWaitSemaphores
    submit_info.pWaitDstStageMask = pipeline_state_flags;

    // note: specify which command buffer to submit for execution
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &vk.command_buffer;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &vk.semaphore_render_finished;

    if (vkQueueSubmit(vk.graphics_queue, 1, &submit_info, vk.fence_present_finished) != VK_SUCCESS) {
        return ;
    }

    // Present the sc image
    VkPresentInfoKHR present_info = { 0 };
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &vk.semaphore_render_finished;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &vk.sc._;
    present_info.pImageIndices = &image_index;
    present_info.pResults = 0; // optional

    vkQueuePresentKHR(vk.graphics_queue, &present_info);
}

static bool vk__init_create_instance() {
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

#if defined(DEBUG)
    if (!vk__check_if_validation_layer_is_available(vk_validation_layers[0])) {
        debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR, "VK_LAYER_KHRONOS_validation was not found in the supported validation layers in debug build");
        return false;
    } else {
        create_info.enabledLayerCount = ARRAY_SIZE(vk_validation_layers);
        create_info.ppEnabledLayerNames = vk_validation_layers;
    }

    VkDebugUtilsMessengerCreateInfoEXT instance_create_messenger_create_info;
    vk__populate_debug_messenger_create_info(&instance_create_messenger_create_info);
    create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &instance_create_messenger_create_info;
#endif

    VkResult create_instance_result = vkCreateInstance(&create_info, 0, &vk.instance);
    if (create_instance_result != VK_SUCCESS) {
        debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR, "failed to create an instance");
        return false;
    }

    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "created vk instance ");

    return true;
}

static bool vk__init_setup_debug_messenger() {
#if defined(DEBUG)
    // create debug messenger callback
    VkDebugUtilsMessengerCreateInfoEXT app_messenger_create_info;
    vk__populate_debug_messenger_create_info(&app_messenger_create_info);
    if (!vk__setup_debug_messenger_create_info(&app_messenger_create_info)) {
        return false;
    }
#endif

    return true;
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
    debug__writeln("unsupported Vulkan extensions by GLFW:");
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
            debug__writeln("%s", supported_vk_extensions[supported_vk_extension_index].extensionName);
        }
    }
    if (!found_unsupported_extension) {
        debug__writeln("---");
    }
    debug__flush(DEBUG_MODULE_VULKAN, DEBUG_WARN);
    free(supported_vk_extensions);
#endif

    return true;
}

static bool vk__create_surface(GLFWwindow* glfw_window) {
    if (glfwCreateWindowSurface(vk.instance, glfw_window, 0, &vk.surface) != VK_SUCCESS) {
        return false;
    }

    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "created vk surface");

    return true;
}

static bool vk__pick_physical_device() {
    vk.physical_device = VK_NULL_HANDLE;

    uint32_t physical_device_count = 0;
    vkEnumeratePhysicalDevices(vk.instance, &physical_device_count, 0);
    VkPhysicalDevice* physical_devices = malloc(physical_device_count * sizeof(*physical_devices));
    vkEnumeratePhysicalDevices(vk.instance, &physical_device_count, physical_devices);
    for (uint32_t physical_device_index = 0; physical_device_index < physical_device_count; ++physical_device_index) {
        if (vk__is_physical_device_suitable(physical_devices[physical_device_index])) {
            vk.physical_device = physical_devices[physical_device_index];
            break ;
        }
    }
    free(physical_devices);

    if (vk.physical_device == VK_NULL_HANDLE) {
        debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR, "failed to find a suitable GPU");
        return false;
    }

    VkPhysicalDeviceProperties device_properties;
    vkGetPhysicalDeviceProperties(vk.physical_device, &device_properties);
    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "GPU selected as a vk physical device: %s", device_properties.deviceName);

    return true;
}

static bool vk__is_physical_device_suitable(VkPhysicalDevice physical_device) {
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures   device_features;

    vkGetPhysicalDeviceProperties(physical_device, &device_properties);
    vkGetPhysicalDeviceFeatures(physical_device, &device_features);

    // must have dedicated graphics card
    // if (device_properties.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
    //     return false;
    // }

    // must support geometry shaders
    if (device_features.geometryShader == VK_FALSE) {
        return false;
    }

    if (!vk__check_device_required_extensions(physical_device)) {
        return false;
    }

    swapchain_support_details_t swapchain_support_details = swapchain_support_details__create(physical_device);
    bool is_swapchain_adequate = false;
    if (swapchain_support_details.formats_size > 0 && swapchain_support_details.present_modes_size > 0) {
        is_swapchain_adequate = true;
    }
    swapchain_support_details__destroy(&swapchain_support_details);
    if (!is_swapchain_adequate) {
        return false;
    }

    queue_families_t queue_families = vk__find_queue_families(physical_device);
    if (!queue_families__is_complete(&queue_families)) {
        return false;
    }

    return true;
}

static queue_families_t vk__find_queue_families(VkPhysicalDevice device) {
    queue_families_t result = { 0 };

    uint32_t queue_family_properties_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_properties_count, 0);
    VkQueueFamilyProperties* queue_family_properties = malloc(queue_family_properties_count * sizeof(*queue_family_properties));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_properties_count, queue_family_properties);

    for (uint32_t queue_family_properties_index = 0; queue_family_properties_index < queue_family_properties_count; ++queue_family_properties_index) {
        if (queue_families__is_complete(&result)) {
            break ;
        }

        if (!result.graphics_family.exists && queue_family_properties[queue_family_properties_index].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            result.graphics_family.index  = queue_family_properties_index;
            result.graphics_family.exists = true;
        }

        if (!result.presentation_family.exists) {
            VkBool32 presentation_supported = VK_FALSE;
            if (vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_family_properties_index, vk.surface, &presentation_supported) == VK_SUCCESS) {
                if (presentation_supported == VK_TRUE) {
                    result.presentation_family.index  = queue_family_properties_index;
                    result.presentation_family.exists = true;
                }
            }
        }

    }

    free(queue_family_properties);

    return result;
}

static bool vk__check_device_required_extensions(VkPhysicalDevice device) {
    uint32_t device_extensions_count = 0;
    vkEnumerateDeviceExtensionProperties(device, 0, &device_extensions_count, 0);
    VkExtensionProperties* device_extension_properties = malloc(device_extensions_count * sizeof(*device_extension_properties));
    vkEnumerateDeviceExtensionProperties(device, 0, &device_extensions_count, device_extension_properties);
    bool found_all = true;
    for (uint32_t required_device_extension_index = 0; required_device_extension_index < ARRAY_SIZE(vk_required_logical_device_extensions); ++required_device_extension_index) {
        bool found = false;
        for (uint32_t device_extension_index = 0; device_extension_index < device_extensions_count; ++device_extension_index) {
            if (strcmp(device_extension_properties[device_extension_index].extensionName, vk_required_logical_device_extensions[required_device_extension_index]) == 0) {
                found = true;
                break ;
            }
        }
        if (!found) {
            found_all = false;
            break ;
        }
    }
    free(device_extension_properties);

    return found_all;
}

static bool queue_families__is_complete(queue_families_t* self) {
    // must have graphics queue family
    if (!self->graphics_family.exists) {
        return false;
    }

    // must have presentation queue family
    if (!self->presentation_family.exists) {
        return false;
    }

    return true;
}

static bool array_u32_has(uint32_t* arr, uint32_t arr_size, uint32_t value) {
    for (uint32_t arr_index = 0; arr_index < arr_size; ++arr_index) {
        if (arr[arr_index] == value) {
            return true;
        }
    }
    return false;
}

static bool vk__create_logical_device() {
    queue_families_t queue_families = vk__find_queue_families(vk.physical_device);
    ASSERT(queue_families__is_complete(&queue_families));

    const uint32_t max_number_of_unique_device_queues = 16;
    uint32_t unique_device_queue_indices[max_number_of_unique_device_queues];
    uint32_t unique_device_queue_indices_top = 0;
    unique_device_queue_indices[unique_device_queue_indices_top++] = queue_families.graphics_family.index;
    if (!array_u32_has(unique_device_queue_indices, unique_device_queue_indices_top, queue_families.presentation_family.index)) {
        unique_device_queue_indices[unique_device_queue_indices_top++] = queue_families.presentation_family.index;
    }
    
    VkDeviceQueueCreateInfo device_queue_create_infos[max_number_of_unique_device_queues];
    memset(device_queue_create_infos, 0, sizeof(device_queue_create_infos));
    for (uint32_t unique_queue_family = 0; unique_queue_family < unique_device_queue_indices_top; ++unique_queue_family) {
        VkDeviceQueueCreateInfo* device_queue_create_info = &device_queue_create_infos[unique_queue_family];
        device_queue_create_info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        device_queue_create_info->queueFamilyIndex = unique_device_queue_indices[unique_queue_family];
        device_queue_create_info->queueCount = 1;
        float queue_priority = 1.0f;
        device_queue_create_info->pQueuePriorities = &queue_priority;
    }

    VkPhysicalDeviceFeatures device_features = { 0 };

    VkDeviceCreateInfo device_create_info = { 0 };
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = unique_device_queue_indices_top;
    device_create_info.pQueueCreateInfos = device_queue_create_infos;
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = ARRAY_SIZE(vk_required_logical_device_extensions);
    device_create_info.ppEnabledExtensionNames = vk_required_logical_device_extensions;
#if defined(DEBUG)
    device_create_info.enabledLayerCount = 1;
    device_create_info.ppEnabledLayerNames = vk_validation_layers;
#endif
    if (vkCreateDevice(vk.physical_device, &device_create_info, 0, &vk.logical_device) != VK_SUCCESS) {
        debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR, "failed to create a logical device");
        return false;
    }

    vkGetDeviceQueue(vk.logical_device, queue_families.graphics_family.index, 0, &vk.graphics_queue);
    vkGetDeviceQueue(vk.logical_device, queue_families.presentation_family.index, 0, &vk.presentation_queue);

    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "created vk logical device");

    return true;
}

static swapchain_support_details_t swapchain_support_details__create(VkPhysicalDevice physical_device) {
    swapchain_support_details_t result = { 0 };

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, vk.surface, &result.capabilities);

    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk.surface, &result.formats_size, 0);
    result.formats = malloc(result.formats_size * sizeof(*result.formats));
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, vk.surface, &result.formats_size, result.formats);

    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk.surface, &result.present_modes_size, 0);
    result.present_modes = malloc(result.present_modes_size * sizeof(*result.present_modes));
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, vk.surface, &result.present_modes_size, result.present_modes);

    return result;
}

static void swapchain_support_details__destroy(swapchain_support_details_t* self) {
    if (self->formats) {
        free(self->formats);
    }
    if (self->present_modes) {
        free(self->present_modes);
    }
}

VkSurfaceFormatKHR swapchain__pick_surface_format(VkSurfaceFormatKHR* surface_formats, uint32_t surface_formats_size) {
    ASSERT(surface_formats_size > 0);
    for (uint32_t surface_format_index = 0; surface_format_index < surface_formats_size; ++surface_format_index) {
        VkSurfaceFormatKHR surface_format = surface_formats[surface_format_index];
        if (surface_format.format == VK_FORMAT_B8G8R8A8_SRGB && surface_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return surface_format;
        }
    }

    return surface_formats[0];
}

VkPresentModeKHR swapchain__pick_present_mode(VkPresentModeKHR* present_modes, uint32_t present_modes_size) {
    ASSERT(present_modes_size > 0);
    for (uint32_t present_mode_index = 0; present_mode_index < present_modes_size; ++present_mode_index) {
        VkPresentModeKHR present_mode = present_modes[present_mode_index];
        if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return present_mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D swapchain__pick_extent(VkSurfaceCapabilitiesKHR* capabilities, GLFWwindow* glfw_window) {
    VkExtent2D result = { 0 };

    if (capabilities->currentExtent.width != UINT32_MAX) {
        // note: Window manager sets this value to UINT32_MAX if it allows us to change the resolution of swapchain images
        result = capabilities->currentExtent;
    } else {
        uint32_t framebuffer_width;
        uint32_t framebuffer_height;
        glfwGetFramebufferSize(glfw_window, (int32_t*) &framebuffer_width, (int32_t*) &framebuffer_height);
        result.width = framebuffer_width;
        result.height = framebuffer_height;

        result.width = MAX(result.width, capabilities->minImageExtent.width);
        result.height = MAX(result.height, capabilities->minImageExtent.height);
        result.width = MIN(result.width, capabilities->maxImageExtent.width);
        result.height = MIN(result.height, capabilities->maxImageExtent.height);
    }

    return result;
}

static bool vk__create_swapchain(GLFWwindow* glfw_window) {
    swapchain_support_details_t swapchain_support_details = swapchain_support_details__create(vk.physical_device);

    VkSurfaceFormatKHR sc_surface_format = swapchain__pick_surface_format(swapchain_support_details.formats, swapchain_support_details.formats_size);
    VkPresentModeKHR sc_present_mode = swapchain__pick_present_mode(swapchain_support_details.present_modes, swapchain_support_details.present_modes_size);
    VkExtent2D sc_extent = swapchain__pick_extent(&swapchain_support_details.capabilities, glfw_window);

    // note: add 1 to avoid waiting on the driver before we can acquire another image
    uint32_t sc_images_count = swapchain_support_details.capabilities.minImageCount + 1;
    if (swapchain_support_details.capabilities.maxImageCount > 0 && sc_images_count > swapchain_support_details.capabilities.maxImageCount) {
        sc_images_count = swapchain_support_details.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR sc_create_info = { 0 };
    sc_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    sc_create_info.surface = vk.surface;
    sc_create_info.minImageCount = sc_images_count;
    sc_create_info.imageFormat = sc_surface_format.format;
    sc_create_info.imageColorSpace = sc_surface_format.colorSpace;
    sc_create_info.imageExtent = sc_extent;

    // note: specify number of layers each image consists of
    sc_create_info.imageArrayLayers = 1;

    // note: render directly to the image
    sc_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    queue_families_t queue_families = vk__find_queue_families(vk.physical_device);
    ASSERT(queue_families__is_complete(&queue_families));

    uint32_t queue_family_indices_top = 0;
    uint32_t queue_family_indices[16] = { 0 };
    ASSERT(queue_family_indices_top < ARRAY_SIZE(queue_family_indices));
    queue_family_indices[queue_family_indices_top++] = queue_families.graphics_family.index;
    ASSERT(queue_family_indices_top < ARRAY_SIZE(queue_family_indices));
    queue_family_indices[queue_family_indices_top++] = queue_families.presentation_family.index;

    if (queue_families.graphics_family.index != queue_families.presentation_family.index) {
        // note: images can be used across multiple queue families without explicit ownership transfers
        sc_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        sc_create_info.queueFamilyIndexCount = 2;
        ASSERT(sc_create_info.queueFamilyIndexCount <= ARRAY_SIZE(queue_family_indices));
        sc_create_info.pQueueFamilyIndices = queue_family_indices;
    } else {
        // note: image is owned by one queue family and ownership must be explicitly transferred before using it in another queue family
        sc_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    // note: could specify certain transforms to be applied on the image (like a 90 degree clockwise rotation), these transforms are in capabilities.supportedTransforms
    sc_create_info.preTransform = swapchain_support_details.capabilities.currentTransform;

    // note: ignore alpha channel during blending
    sc_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    sc_create_info.presentMode = sc_present_mode;

    // note: don't care about color of pixels that are obscured
    sc_create_info.clipped = VK_TRUE;

    sc_create_info.oldSwapchain = VK_NULL_HANDLE;

    bool sc_create_result = true;
    if (vkCreateSwapchainKHR(vk.logical_device, &sc_create_info, 0, &vk.sc._) != VK_SUCCESS) {
        sc_create_result = false;
        debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_ERROR, "failed to create swapchain");
    } else {
        debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "created vk swapchain");

        // note: retrieve sc image handles
        vkGetSwapchainImagesKHR(vk.logical_device, vk.sc._, &vk.sc.images_size, 0);
        vk.sc.images = malloc(vk.sc.images_size * sizeof(*vk.sc.images));
        vkGetSwapchainImagesKHR(vk.logical_device, vk.sc._, &vk.sc.images_size, vk.sc.images);
        vk.sc.format = sc_surface_format.format;
        vk.sc.extent = sc_extent;
    }

    swapchain_support_details__destroy(&swapchain_support_details);

    return sc_create_result;
}

static bool vk__create_image_views() {
    vk.image_views_size = vk.sc.images_size;
    vk.image_views = malloc(vk.image_views_size * sizeof(*vk.image_views));

    for (uint32_t swap_chain_image_index = 0; swap_chain_image_index < vk.sc.images_size; ++swap_chain_image_index) {
        VkImage sc_image = vk.sc.images[swap_chain_image_index];

        VkImageViewCreateInfo image_view_create_info = { 0 };
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = sc_image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = vk.sc.format;

        // note: map channels to their identity (could do all kinds of stuff here to swizzle channels)
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        // note: color target without mipmapping, single layer (could do multiple for stereographic 3D applications)
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(vk.logical_device, &image_view_create_info, 0, &vk.image_views[swap_chain_image_index]) != VK_SUCCESS) {
            return false;
        }
    }

    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "created vk image views");

    return true;
}

typedef struct shader_module {
    shader_code_t  shader_code;
    VkShaderModule vk_shader_module;
    VkDevice       logical_device;
} shader_module_t;

static bool shader_module__create_from_file(shader_module_t* self, const char* shader_file_path, VkDevice logical_device);
static void shader_module__destroy(shader_module_t* self);

static bool vk__create_render_pass() {
    VkAttachmentDescription color_attachment_description = { 0 };
    color_attachment_description.format = vk.sc.format;
    color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;

    // note: determine what to do before and after rendering with the color data in the attachment

    // note: clear at start (before drawing a new frame)
    color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

    // note: rendered contents are store in memory, can retrieve later
    color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // note: these apply to stencil data instead of color
    color_attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // note: images need to transition to specific layouts that are suitable for the operation that they're going to be involved in next

    // note: not guaranteed to be preserved, which doesn't matter, as we are clearing it anyway
    color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    // note: specify the layout to transition to when the render pass finishes, we want it to be ready for presentation after rendering
    color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Subpasses and attachment references
    VkAttachmentReference color_attachment_reference = { 0 };

    // note: index of attachment description array, which is directly referenced from the fragment shader
    //       layout (location = 0) out vec4 out_color;
    color_attachment_reference.attachment = 0;
    color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description = { 0 };

    // note: can also be compute
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass_description.colorAttachmentCount = 1;

        /**
     * subpass dependencies:
     *  - at the start of the render pass we haven't yet acquired the sc image, so we have to resolve that here
    */
    VkSubpassDependency subpass_dependency = { 0 };

    // note: VK_SUBPASS_EXTERNAL refers to the implicit subpass before or after the render pass depending on whether it is specified in srcSubpass or dstSubpass
    subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;

    // note: 0 refers to our subpass, this must always be higher than srcSubpass to prevent cyclic dependencies (except for the case of VK_SUBPASS_EXTERNAL)
    subpass_dependency.dstSubpass = 0;

    // note: specify the operations to wait on and the stages in which these operations occur
    subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.srcAccessMask = 0;

    // note: specify the operations that should wait on the above
    subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    /**
     * other types of attachments are:
     *  input attachments: attachments that are read from a shader
     *  resolve attachments: attachments used for multisampling color attachments
     *  depth-stencil attachments: attachments for depth and stencil data
     *  preserve attachments: attachments that are not used by this subpass, but for which the data must be preserved
    */
    subpass_description.pColorAttachments = &color_attachment_reference;

    // Render pass
    VkRenderPassCreateInfo render_pass_create_info = { 0 };
    render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_create_info.attachmentCount = 1;
    render_pass_create_info.pAttachments = &color_attachment_description;
    render_pass_create_info.subpassCount = 1;
    render_pass_create_info.pSubpasses = &subpass_description;
    render_pass_create_info.dependencyCount = 1;
    render_pass_create_info.pDependencies = &subpass_dependency;

    if (vkCreateRenderPass(vk.logical_device, &render_pass_create_info, 0, &vk.render_pass) != VK_SUCCESS) {
        return false;
    }

    return true;
}

static bool vk__create_gfx_pipeline() {
    const char* shader_paths[] = {
        "game/shaders/vert.spv",
        "game/shaders/frag.spv"
    };

    VkShaderStageFlagBits vk_shader_stage_flag_bits[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT
    };

    shader_module_t shader_modules[ARRAY_SIZE(shader_paths)] = { 0 };

    uint32_t created_shader_modules = 0;
    ASSERT(ARRAY_SIZE(vk.shader_stage_creation_infos) >= ARRAY_SIZE(shader_modules));
    for (uint32_t shader_modules_index = 0; shader_modules_index < ARRAY_SIZE(shader_modules); ++shader_modules_index, ++created_shader_modules) {
        shader_module_t* shader_module = &shader_modules[shader_modules_index];
        if (!shader_module__create_from_file(shader_module, shader_paths[shader_modules_index], vk.logical_device)) {
            break ;
        }

        VkPipelineShaderStageCreateInfo* shader_creation_info = &vk.shader_stage_creation_infos[vk.shader_stage_creation_infos_top++];
        memset(shader_creation_info, 0, sizeof(*shader_creation_info));
        shader_creation_info->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_creation_info->stage = vk_shader_stage_flag_bits[shader_modules_index];
        shader_creation_info->module = shader_module->vk_shader_module;
        shader_creation_info->pName = "main";

        // note: specify values for shader constants, which can leverage certain compiler optimizations
        // shader_creation_info->pSpecializationInfo;
    }
    if (created_shader_modules != ARRAY_SIZE(shader_modules)) {
        for (uint32_t shader_modules_index = 0; shader_modules_index < created_shader_modules; ++shader_modules_index) {
            shader_module_t* shader_module = &shader_modules[shader_modules_index];
            shader_module__destroy(shader_module);
        }
        return false;
    }

    // note: create dynamic states for the pipeline that can (and must) be changed without recreating the pipeline
    VkDynamicState dynamic_states[] = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };
    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = { 0 };
    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = ARRAY_SIZE(dynamic_states);
    dynamic_state_create_info.pDynamicStates = dynamic_states;

    // vertex input
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = { 0 };
    vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    // note: bindings: spacing between data, and whether the data is per-vertex or per-instance
    vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
    vertex_input_state_create_info.pVertexBindingDescriptions = 0;

    // note: attribute descriptions: type of the attributes passed to the vertex shader, which binding to load
    //                               them from, and at which offset
    vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;
    vertex_input_state_create_info.pVertexAttributeDescriptions = 0;

    // input assembly (what kind of geometry will be drawn from the vertices, and should primitive restart be enabled)
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_create_info = { 0 };
    input_assembly_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_state_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    // note: in the _STRIP topology modes, it's possible to break up lines and triangles, whatever this means..
    input_assembly_state_create_info.primitiveRestartEnable = VK_FALSE;

    // viewports and scissors
    // VkViewport viewport = { 0 };
    // viewport.x = 0.0f;
    // viewport.y = 0.0f;
    // viewport.width = (float) vk.sc.sc_extent.width;
    // viewport.height = (float) vk.sc.sc_extent.height;
    // viewport.minDepth = 0.0f;
    // viewport.maxDepth = 1.0f;
    
    // VkRect2D scissor = { 0 };
    // scissor.offset.x = 0;
    // scissor.offset.y = 0;
    // scissor.extent = vk.sc.sc_extent;
    VkPipelineViewportStateCreateInfo viewport_state_create_info = { 0 };
    viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;

    // note: could also use multiple, which requires a GPU feature (see logical device selection)
    // note: if this is set, its state will become immutable
    // viewport_state_create_info.pViewports = &viewport;
    viewport_state_create_info.scissorCount = 1;

    // note: if this is set, its state will become immutable
    // viewport_state_create_info.pScissors = &scissor;

    // rasterizer
    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = { 0 };
    rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;

    // note: if set, fragments that are beyond the near and far planes are clamped to them, as opposed to
    //       discarding them (whatever this means..), using this requires enabling a GPU feature
    rasterization_state_create_info.depthClampEnable = VK_FALSE;

    // note: if set, then geometry never passes through rasterization stage
    rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;

    // note: determines how fragments are generated for geometry, using any other than fill requires enabling
    //       a GPU feature
    rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;

    // note: describes the thickness of lines in terms of number of fragments, any line thicker than 1.0 requires
    //       enabling wideLines GPU feature
    rasterization_state_create_info.lineWidth = 1.0f;

    // note: determines the type of face culling
    rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;

    // note: specifies the vertex order for faces to be considered front-facing
    rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;

    // note: the rasterizer can alter the depth value by adding a constant value or biasing them based on fragment's slope
    //       this is sometimes used for shadow mapping
    rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    rasterization_state_create_info.depthBiasConstantFactor = 0.0f; // optional
    rasterization_state_create_info.depthBiasClamp = 0.0f; // optional
    rasterization_state_create_info.depthBiasSlopeFactor = 0.0f; // optional

    // multisampling
    // note: requires enabling a GPU feature
    VkPipelineMultisampleStateCreateInfo multisample_state_create_info = { 0 };
    multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_create_info.sampleShadingEnable = VK_FALSE;
    multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state_create_info.minSampleShading = 1.0f; // optional
    multisample_state_create_info.pSampleMask = 0; // optional
    multisample_state_create_info.alphaToCoverageEnable = VK_FALSE; // optional
    multisample_state_create_info.alphaToOneEnable = VK_FALSE; // optional

    // depth and stencil testing
    // VkPipelineDepthStencilStateCreateInfo depth_stencil_state_create_info = { 0 };

    // color blending
    // note: configuration per attached framebuffer
    /**
     * pseudo-code:
     *  if (blend_enabled) {
     *      final_color.rgb = (src_color_blend_factor * new_color.rgb) <color_blend_op> (dst_color_blend_factor * old_color.rgb)
     *      final_color.a   = (src_alpha_blend_factor * new_color.a) <alpha_blend_op> (dst_alpha_blend_factor * old_color.a)
     *  } else {
     *      final_color = new_color;
     *  }
     */
    VkPipelineColorBlendAttachmentState color_blend_attachment_state = { 0 };
    color_blend_attachment_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | 
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment_state.blendEnable = VK_FALSE;
    color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // optional
    color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // optional
    color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD; // optional
    color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // optional
    color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // optional
    color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD; // optional

    // note: global color blending settings
    // allows settings up blend constants for the above calculations
    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = { 0 };
    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_COPY; // optional
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment_state;
    color_blend_state_create_info.blendConstants[0] = 0.0f; // optional
    color_blend_state_create_info.blendConstants[1] = 0.0f; // optional
    color_blend_state_create_info.blendConstants[2] = 0.0f; // optional
    color_blend_state_create_info.blendConstants[3] = 0.0f; // optional

    // pipeline layout
    // note: create uniforms (global variables in shaders)
    VkPipelineLayoutCreateInfo layout_create_info = { 0 };
    layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.setLayoutCount = 0; // optional
    layout_create_info.pSetLayouts = 0; // optional
    layout_create_info.pushConstantRangeCount = 0; // optional
    layout_create_info.pPushConstantRanges = 0; // optional

    if (vkCreatePipelineLayout(vk.logical_device, &layout_create_info, 0, &vk.pipeline_layout) != VK_SUCCESS) {
        for (uint32_t shader_modules_index = 0; shader_modules_index < created_shader_modules; ++shader_modules_index) {
            shader_module_t* shader_module = &shader_modules[shader_modules_index];
            shader_module__destroy(shader_module);
        }
        return false;
    }

    VkGraphicsPipelineCreateInfo pipeline_create_info = { 0 };
    pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.stageCount = 2;
    pipeline_create_info.pStages = vk.shader_stage_creation_infos;
    pipeline_create_info.pVertexInputState = &vertex_input_state_create_info;
    pipeline_create_info.pInputAssemblyState = &input_assembly_state_create_info;
    pipeline_create_info.pViewportState = &viewport_state_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_state_create_info;
    pipeline_create_info.pMultisampleState = &multisample_state_create_info;
    pipeline_create_info.pDepthStencilState = 0; // optional
    pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    pipeline_create_info.pDynamicState = &dynamic_state_create_info;

    // note: describe fixed-function stage
    pipeline_create_info.layout = vk.pipeline_layout;

    pipeline_create_info.renderPass = vk.render_pass;

    // note: index of subpass where this gfx pipeline will be used
    pipeline_create_info.subpass = 0;

    // note: it is possible to use other render passes with this pipeline, but they have to be compatible with render_pass

    // note: vulkan allows to create pipelines by deriving from an existing one
    //       VK_PIPELINE_CREATE_DERIVATIVE_BIT must be specified in flags field in VkGraphicsPipelineCreateInfo 

    // note: reference an existing pipeline to derive from
    pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE; // optional

    // note: reference another pipeline that is about to be created
    pipeline_create_info.basePipelineIndex = -1; // optional

    if (vkCreateGraphicsPipelines(vk.logical_device, VK_NULL_HANDLE, 1, &pipeline_create_info, 0, &vk.gfx_pipeline) != VK_SUCCESS) {
        for (uint32_t shader_modules_index = 0; shader_modules_index < created_shader_modules; ++shader_modules_index) {
            shader_module_t* shader_module = &shader_modules[shader_modules_index];
            shader_module__destroy(shader_module);
        }

        return false;
    }

    for (uint32_t shader_modules_index = 0; shader_modules_index < created_shader_modules; ++shader_modules_index) {
        shader_module_t* shader_module = &shader_modules[shader_modules_index];
        shader_module__destroy(shader_module);
    }

    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "created vk graphics pipeline");

    return true;
}

static bool shader_code__create_from_file(shader_code_t* self, const char* shader_file_path) {
    FILE* fp = fopen(shader_file_path, "rb");
    if (!fp) {
        return false;
    }
    long fp_start = ftell(fp);
    fseek(fp, 0, SEEK_END);
    long fp_end = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    ASSERT(fp_end >= fp_start);
    self->code_size = fp_end - fp_start;
    if (self->code_size == 0) {
        fclose(fp);
        return false;
    }

    self->code = malloc(self->code_size);
    if (!self->code) {
        fclose(fp);
        return false;
    }

    const size_t bytes_read = fread(self->code, sizeof(char), self->code_size, fp);
    if (bytes_read != self->code_size) {
        fclose(fp);
        shader_code__destroy(self);
        return false;
    }

    fclose(fp);

    return true;
}

static void shader_code__destroy(shader_code_t* self) {
    if (self->code) {
        free(self->code);
    }
}

static bool vk__create_shader_module(VkShaderModule* self, shader_code_t* shader_code) {
    VkShaderModuleCreateInfo shader_module_create_info = { 0 };
    shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_module_create_info.codeSize = shader_code->code_size;
    ASSERT((size_t) shader_code->code % sizeof(uint32_t) == 0 && "make sure it's aligned when allocating");
    shader_module_create_info.pCode = (uint32_t*) shader_code->code;

    if (vkCreateShaderModule(vk.logical_device, &shader_module_create_info, 0, self) != VK_SUCCESS) {
        return false;
    }

    return true;
}

static bool shader_module__create_from_file(shader_module_t* self, const char* shader_file_path, VkDevice logical_device) {
    memset(self, 0, sizeof(*self));

    self->logical_device = logical_device;
    if (!shader_code__create_from_file(&self->shader_code, shader_file_path)) {
        return false;
    }

    if (!vk__create_shader_module(&self->vk_shader_module, &self->shader_code)) {
        shader_code__destroy(&self->shader_code);
        return false;
    }

    return true;
}

static void shader_module__destroy(shader_module_t* self) {
    vkDestroyShaderModule(self->logical_device, self->vk_shader_module, 0);
    shader_code__destroy(&self->shader_code);
}

static bool vk__create_framebuffers() {
    vk.framebuffers_size = vk.image_views_size;
    vk.framebuffers = malloc(vk.framebuffers_size * sizeof(*vk.framebuffers));
    uint32_t created_framebuffers = 0;
    for (uint32_t framebuffer_index = 0; framebuffer_index < vk.framebuffers_size; ++framebuffer_index, ++created_framebuffers) {
        VkImageView attachments[] = {
            vk.image_views[framebuffer_index]
        };

        VkFramebufferCreateInfo framebuffer_create_info = { 0 };
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.renderPass = vk.render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = attachments;
        framebuffer_create_info.width = vk.sc.extent.width;
        framebuffer_create_info.height = vk.sc.extent.height;
        framebuffer_create_info.layers = 1;

        if (vkCreateFramebuffer(vk.logical_device, &framebuffer_create_info, 0, &vk.framebuffers[framebuffer_index]) != VK_SUCCESS) {
            break ;
        }
    }

    if (created_framebuffers != vk.framebuffers_size) {
        for (uint32_t framebuffer_index = 0; framebuffer_index < created_framebuffers; ++framebuffer_index, ++created_framebuffers) {
            vkDestroyFramebuffer(vk.logical_device, vk.framebuffers[framebuffer_index], 0);
        }
        return false;
    }

    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "created vk framebuffers");

    return true;
}

static bool vk__create_command_pool() {
    queue_families_t queue_families = vk__find_queue_families(vk.physical_device);

    VkCommandPoolCreateInfo command_pool_create_info = { 0 };
    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;

    // note: Allow command buffers to be rerecorded individually, without this flag they all have to be reset together
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    command_pool_create_info.queueFamilyIndex = queue_families.graphics_family.index;

    if (vkCreateCommandPool(vk.logical_device, &command_pool_create_info, 0, &vk.command_pool) != VK_SUCCESS) {
        return false;
    }

    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "created vk command pool");

    return true;
}

static bool vk__create_command_buffer() {
    VkCommandBufferAllocateInfo command_buffer_allocate_info = { 0 };
    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = vk.command_pool;

    // note: primary can be submitted to a queue for execution, but cannot be called from other command buffers
    //       while secondary cannot be submitted directly, but can be called from other primary command buffers
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    command_buffer_allocate_info.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(vk.logical_device, &command_buffer_allocate_info, &vk.command_buffer) != VK_SUCCESS) {
        return false;
    }

    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "created vk command buffers");

    return true;
}

static bool cb__record(VkCommandBuffer self, uint32_t image_index) {
    VkCommandBufferBeginInfo cb_begin_info = { 0 };
    cb_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    /**
     * _ONE_TIME_SUBMIT_BIT: the cb will be rerecorded right after executing it once
     * _RENDER_PASS_CONTINUE_BIT: this is a secondary cb that will be entirey within a single render pass
     * _SIMULTANEOUS_USE_BIT: the cb can be resubmitted while is is also already pending execution
    */
    cb_begin_info.flags = 0; // optional

    // note: only relevant for secondary cbs, it specifies which state to inherit from the calling primary cbs
    cb_begin_info.pInheritanceInfo = 0; // optional

    if (vkBeginCommandBuffer(self, &cb_begin_info) != VK_SUCCESS) {
        return false;
    }

    // Start render pass
    VkRenderPassBeginInfo render_pass_begin_info = { 0 };
    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.renderPass = vk.render_pass;
    ASSERT(image_index < vk.framebuffers_size);
    render_pass_begin_info.framebuffer = vk.framebuffers[image_index];
    render_pass_begin_info.renderArea.offset.x = 0;
    render_pass_begin_info.renderArea.offset.y = 0;
    render_pass_begin_info.renderArea.extent = vk.sc.extent;

    // note: clear values to use from VK_ATTACHMENT_LOAD_OP_CLEAR
    VkClearValue clear_color = { 0 };
    clear_color.color.float32[0] = 1.0f;
    clear_color.color.float32[1] = 0.0f;
    clear_color.color.float32[2] = 0.0f;
    clear_color.color.float32[3] = 1.0f;

    render_pass_begin_info.clearValueCount = 1;
    render_pass_begin_info.pClearValues = &clear_color;

    /**
     * VK_SUBPASS_CONTENTS_INLINE controls how the drawing commands within the render pass will be provided
     *      _INLINE: the render pass cmds will be embedded in the primary cb itself and no secondary cb will be executed
     *      _SECONDARY_COMMAND_BUFFERS: the render pass cmds will be executed from secondary cbs
    */
    vkCmdBeginRenderPass(vk.command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

    /**
     * VK_PIPELINE_BIND_POINT_GRAPHICS: pipeline obj is gfx pipeline
     * VK_PIPELINE_BIND_POINT_COMPUTE: pipeline obj is compute pipeline
    */   
    vkCmdBindPipeline(vk.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk.gfx_pipeline);

    // viewports and scissors were specified as dynamic states, so we MUST set them in the cmd buffer before issueing the draw cmd
    VkViewport viewport = { 0 };
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) vk.sc.extent.width;
    viewport.height = (float) vk.sc.extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(vk.command_buffer, 0, 1, &viewport);
    
    VkRect2D scissor = { 0 };
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    scissor.extent = vk.sc.extent;
    vkCmdSetScissor(vk.command_buffer, 0, 1, &scissor);

    /**
     * vertexCount: number of vertices
     * instanceCount: number of instances
     * firstVertex: offset into the vertex buffer, defines the lowest value of gl_VertexIndex
     * firstInstance: offset for instance rendering, defines the lowest value of gl_InstanceIndex
    */
    vkCmdDraw(vk.command_buffer, 3, 1, 0, 0);

    vkCmdEndRenderPass(vk.command_buffer);

    if (vkEndCommandBuffer(vk.command_buffer) != VK_SUCCESS) {
        return false;
    }

    return true;
}

static bool vk__create_sync_objects() {
    VkSemaphoreCreateInfo semaphore_create_info = { 0 };
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fence_create_info = { 0 };
    fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    // note: so that first time waiting for the fence will not hang as it'll already be signalled
    fence_create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    if (vkCreateSemaphore(vk.logical_device, &semaphore_create_info, 0, &vk.semaphore_sc_image_available) != VK_SUCCESS) {
        return false;
    }

    if (vkCreateSemaphore(vk.logical_device, &semaphore_create_info, 0, &vk.semaphore_render_finished) != VK_SUCCESS) {
        return false;
    }

    if (vkCreateFence(vk.logical_device, &fence_create_info, 0, &vk.fence_present_finished) != VK_SUCCESS) {
        return false;
    }

    debug__write_and_flush(DEBUG_MODULE_VULKAN, DEBUG_INFO, "created vk sync objects");

    return true;
}
