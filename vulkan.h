#ifndef VULKAN_H
# define VULKAN_H

/**
 * Steps to draw a triangle:
 * 1. Select physical device (create VkInstance)
 * 2. Create logical device (VkDevice)
 * 3. Window surface (GLFW, VkSurfaceKHR) and Swapchain (collection of render targets, VkSwapchainKHR)
 * 4. Image views (VkImageView) and Framebuffer (VkFramebuffer, reference to an image view that are to be used for color, depth, and stencil)
 * 5. Render passes
 * 6. Graphics pipeline (VkPipeline, VkShaderModule)
 * 7. Command pools (VkCommandPool), and Command buffers (VkCommandBuffer)
 * 8. Main loop
 *  - wait for the previous frame to finish
 *  - acquire image from swapchain (vkAcquireNextImageKHR)
 *  - record a command buffer which draws the scene onto that image
 *  - submit the recorded command buffer
 *  - present te swap chain image
*/

/********************************************************************************
 * Module API
 ********************************************************************************/

# include <stdint.h>
# include <stdbool.h>

# include "glfw.h"

bool vk__init(window_t window);
void vk__deinit();

void vk__render();

// todo: add glsl -> spir-v compiler

#endif // VULKAN_H
