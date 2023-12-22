#include "vulkan.h"

#include "debug.h"
#include "helper_macros.h"

#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "vulkan_internal.c"

bool vk__init(window_t window) {
    memset(&vk, 0, sizeof(vk));

    vk.window = window;

    if (!vk__init_create_instance()) {
        return false;
    }

    if (!vk__init_setup_debug_messenger()) {
        return false;
    }

    if (!vk__create_surface()) {
        return false;
    }

    if (!vk__pick_physical_device()) {
        return false;
    }

    if (!vk__create_logical_device()) {
        return false;
    }

    if (!vk__create_swapchain()) {
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
