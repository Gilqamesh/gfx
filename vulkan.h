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
 *  - acquire image from swapchain (vkAcquireNextImageKHR)
 *  - select appropriate command buffer for that image, and execute it (vkQueueSubmit)
 *  - return the image to the swapchain for presentation the the screen (vkQueuePresentKHR)
*/

/********************************************************************************
 * Module API
 ********************************************************************************/

# include <stdint.h>
# include <stdbool.h>

bool vk__init();
void vk__deinit();

#endif // VULKAN_H
