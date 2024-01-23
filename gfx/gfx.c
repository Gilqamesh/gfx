#include "gfx.h"

#include "debug.h"
#include "helper_macros.h"

# include <stdlib.h>
# include <string.h>
# include <stdio.h>

#if defined(OPENGL)
# define GLFW_INCLUDE_NONE
# include <GLFW/glfw3.h>
# include "gl/gl.h"
# include "gl/gl.c"
#elif defined(VULKAN)
# define GLFW_INCLUDE_VULKAN
# include <GLFW/glfw3.h>
# include <vulkan/vulkan.h>
# include "vulkan/vulkan_impl.c"
#else
# error "undefined backend, must either be OPENGL or VULKAN"
#endif

#include "gfx_impl.c"

bool gfx__init() {
#if defined(OPENGL)
#elif defined(VULKAN)
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif

    memset(&gfx, 0, sizeof(gfx));

    glfwSetErrorCallback(&gfx__error_callback);

    debug__lock();

    debug__writeln("version string: %s", glfwGetVersionString());
    debug__writeln("compiled version: %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);

    int major = 0;
    int minor = 0;
    int rev   = 0;
    glfwGetVersion(&major, &minor, &rev);
    debug__writeln("runtime version: %d.%d.%d", major, minor, rev);

    // glfwInitHint();
    if (glfwInit() == GLFW_FALSE) {
        debug__writeln("glfwInit == GLFW_FALSE");
        debug__flush(DEBUG_MODULE_GLFW, DEBUG_ERROR);
        
        debug__unlock();
        return false;
    }

    glfwSetMonitorCallback(&gfx__monitor_callback);
    glfwSetJoystickCallback(&gfx__controller_callback);

    uint32_t number_of_monitors;
    GLFWmonitor** glfw_monitors = glfwGetMonitors((int32_t*) &number_of_monitors);
    gfx.monitors_size = number_of_monitors;
    gfx.monitors_top = number_of_monitors;
    gfx.monitors = malloc(gfx.monitors_size * sizeof(*gfx.monitors));
    debug__writeln("number of connected monitors: %u", number_of_monitors);
    // todo: turn this into column-based format
    for (uint32_t monitor_index = 0; monitor_index < number_of_monitors; ++monitor_index) {
        gfx.monitors[monitor_index] = calloc(1, sizeof(*gfx.monitors[monitor_index]));
        monitor_t monitor = gfx.monitors[monitor_index];
        monitor->glfw_monitor = glfw_monitors[monitor_index];
        int32_t width_mm;
        int32_t height_mm;
        glfwGetMonitorPhysicalSize(monitor->glfw_monitor, &width_mm, &height_mm);
        debug__writeln("  %u: %s - physical size: %ux%u [mm]", monitor_index, glfwGetMonitorName(monitor->glfw_monitor), width_mm, height_mm);
    }

    debug__flush(DEBUG_MODULE_GLFW, DEBUG_INFO);

    debug__unlock();

    for (uint32_t controller_index = 0; controller_index < ARRAY_SIZE(gfx.controller); ++controller_index) {
        if (glfwJoystickIsGamepad(controller_index)) {
            controller_t* controller = &gfx.controller[controller_index];
            controller->name = glfwGetJoystickName(controller_index);
            controller__set_connected(controller, true);
        }
    }

    return true;
}

void gfx__deinit() {
#if defined(OPENGL)
#elif defined(VULKAN)
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
    glfwTerminate();
}

void gfx__poll_events() {
    gfx__pre_poll_events();
    glfwPollEvents();
    gfx__post_poll_events();
}

void gfx__wait_events() {
    gfx__pre_poll_events();
    glfwWaitEvents();
    gfx__post_poll_events();
}

monitor_t* monitor__get_monitors(uint32_t* number_of_monitors) {
    *number_of_monitors = gfx.monitors_top;
    return gfx.monitors;
}

void monitor__get_screen_size(monitor_t self, uint32_t* w, uint32_t* h) {
    const GLFWvidmode* mode = glfwGetVideoMode(self->glfw_monitor);
    *w = mode->width;
    *h = mode->height;
}

void monitor__get_work_area(monitor_t self, int32_t* x, int32_t* y, uint32_t* w, uint32_t* h) {
    glfwGetMonitorWorkarea(self->glfw_monitor, x, y, (int32_t*) w, (int32_t*) h);
}

window_t window__create(monitor_t monitor, const char* title, uint32_t width, uint32_t height) {
    if (gfx.windows_top == gfx.windows_size) {
        uint32_t windows_prev_size = gfx.windows_size;
        if (gfx.windows_size == 0) {
            gfx.windows_size = 4;
            gfx.windows = malloc(gfx.windows_size * sizeof(*gfx.windows));
        } else {
            gfx.windows_size <<= 1;
            gfx.windows = realloc(gfx.windows, gfx.windows_size * sizeof(*gfx.windows));
        }
        for (uint32_t window_index = windows_prev_size; window_index < gfx.windows_size; ++window_index) {
            gfx.windows[window_index] = malloc(sizeof(*gfx.windows[window_index]));
        }
    }
    window_t result = gfx.windows[gfx.windows_top++];
    memset(result, 0, sizeof(*result));

    result->title                   = title;
    result->monitor                 = monitor;
    result->controller.name         = title;
    result->requested_display_state = WINDOW_DISPLAY_STATE_WINDOWED;
    controller__set_connected(&result->controller, true);

    // for faster window creation and application switching, choose the closest video mode available (relative to monitor)
    const GLFWvidmode* mode = glfwGetVideoMode(monitor->glfw_monitor);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

#if defined(OPENGL)
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#elif defined(VULKAN)
    // The window surface cannot be shared with another API
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif

    result->glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!result->glfw_window) {
        window__destroy(result);
        return 0;
    }
    
# if defined(OPENGL)
    window__set_current(result);
    if (!gl__init_context()) {
        // todo: first initialize stuff and only then fill in the result struct
        window__destroy(result);
        return 0;
    }
#elif defined(VULKAN)
    if (!vk__init(result->glfw_window)) {
        window__destroy(result);
        return 0;
    }
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif

    glfwSwapInterval(1);

    // glfwSetJoystickUserPointer();

    {
        window__add_default_button_actions(result);

        glfwSetWindowUserPointer(result->glfw_window, result);
        glfwSetWindowPosCallback(result->glfw_window, &window__pos_changed_callback);
        glfwSetWindowSizeCallback(result->glfw_window, &window__size_changed_callback);
        glfwSetWindowCloseCallback(result->glfw_window, &window__should_close_callback);
        glfwSetFramebufferSizeCallback(result->glfw_window, &window__framebuffer_resize_callback);
        glfwSetWindowContentScaleCallback(result->glfw_window, &window__content_scale_callback);
        glfwSetWindowIconifyCallback(result->glfw_window, &window__minimized_callback);
        glfwSetWindowMaximizeCallback(result->glfw_window, &window__maximized_callback);
        glfwSetWindowFocusCallback(result->glfw_window, &window__focus_callback);
        glfwSetKeyCallback(result->glfw_window, &window__key_callback);
        glfwSetCharCallback(result->glfw_window, &window__utf32_callback);
        glfwSetCursorPosCallback(result->glfw_window, &window__cursor_pos_callback);
        glfwSetCursorEnterCallback(result->glfw_window, &window__cursor_enter_callback);
        glfwSetMouseButtonCallback(result->glfw_window, &window__cursor_button_callback);
        glfwSetScrollCallback(result->glfw_window, &window__cursor_scroll_callback);
        glfwSetDropCallback(result->glfw_window, &window__drop_callback);

        glfwGetWindowPos(result->glfw_window, &result->content_area_x, &result->content_area_y);
        glfwGetWindowSize(result->glfw_window, (int32_t*) &result->content_area_w, (int32_t*) &result->content_area_h);
    }

    window__set_hidden(result, false);

    return result;
}

void window__destroy(window_t self) {
#if defined(VULKAN)
    vk__deinit();
#endif

    glfwDestroyWindow(self->glfw_window);

    bool found_window = false;
    for (uint32_t window_index = 0; window_index < gfx.windows_top; ++window_index) {
        if (gfx.windows[window_index] == self) {
            found_window = true;
            while (window_index < gfx.windows_top - 1) {
                gfx.windows[window_index] = gfx.windows[window_index + 1];
                ++window_index;
            }
            break ;
        }
    }

    ASSERT(found_window);

    --gfx.windows_top;

    // todo: realloc gfx.windows to save space?
}

void window__set_default_button_actions(window_t self, bool value) {
    if (value) {
        window__add_default_button_actions(self);
    }
    ASSERT(false && "todo: implement");
}

void window__set_monitor(window_t self, monitor_t monitor) {
    self->monitor = monitor;
}

void window__set_title(window_t self, const char* title) {
    self->title = title;
    glfwSetWindowTitle(self->glfw_window, self->title);
}

const char* window__get_title(window_t self) {
    return self->title;
}

void window__set_current(window_t self) {
    glfwMakeContextCurrent(self->glfw_window);
}

window_t window__get_current() {
    GLFWwindow* glfw_window = glfwGetCurrentContext();
    return window__from_glfw_window(glfw_window);
}

void window__set_icon(window_t self, uint8_t* pixels, uint32_t w, uint32_t h) {
    GLFWimage image = {
        .pixels = pixels,
        .width = w,
        .height = h
    };
    glfwSetWindowIcon(self->glfw_window, 1, &image);
}

void window__set_windowed_state_content_area(window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height) {
    if (window__get_display_state(self) == WINDOW_DISPLAY_STATE_WINDOWED) {
        const bool was_hidden = window__get_hidden(self);
        if (!was_hidden) {
            window__set_hidden(self, true);
        }

        glfwSetWindowPos(self->glfw_window, x, y);
        glfwSetWindowSize(self->glfw_window, (int32_t) width, (int32_t) height);
        glfwGetWindowPos(self->glfw_window, &self->content_area_x, &self->content_area_y);
        glfwGetWindowSize(self->glfw_window, (int32_t*) &self->content_area_w, (int32_t*) &self->content_area_h);

        if (!was_hidden) {
            window__set_hidden(self, false);
        }
    } else {
        self->content_area_x = x;
        self->content_area_y = y;
        self->content_area_w = width;
        self->content_area_h = height;
    }
}

void window__get_windowed_state_content_area(window_t self, int32_t* x, int32_t* y, uint32_t* width, uint32_t* height) {
    *x      = self->content_area_x;
    *y      = self->content_area_y;
    *width  = self->content_area_w;
    *height = self->content_area_h;
}

void window__set_windowed_state_window_area(window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height) {
    int32_t left, right, top, bottom;
    glfwGetWindowFrameSize(self->glfw_window, &left, &top, &right, &bottom);
    x      -= left;
    y      -= top;
    width  += left + right;
    height += top + bottom;
    window__set_windowed_state_content_area(self, x, y, width, height);
}

void window__get_windowed_state_window_area(window_t self, int32_t* x, int32_t* y, uint32_t* width, uint32_t* height) {
    int32_t left, right, top, bottom;
    glfwGetWindowFrameSize(self->glfw_window, &left, &top, &right, &bottom);
    window__get_windowed_state_content_area(self, x, y, width, height);
    *x      -= left;
    *y      -= top;
    *width  += left + right;
    *height += top + bottom;
}

void window__set_display_state(window_t self, window_display_state_t state) {
    // note: limit the state change by delegating it and doing it once during event polling in order to avoid race conditions due to asynchronous event handling on some platforms (X11 for example)
    self->requested_display_state = state;
}

window_display_state_t window__get_display_state(window_t self) {
    if (window__get_fullscreen(self)) {
        if (window__get_minimized(self)) {
            return WINDOW_DISPLAY_STATE_MINIMIZED_FULLSCREEN;
        } else {
            return WINDOW_DISPLAY_STATE_FULLSCREEN;
        }
    } else {
        if (window__get_minimized(self)) {
            return WINDOW_DISPLAY_STATE_WINDOWED_MINIMIZED;
        } else if (window__get_maximized(self)) {
            return WINDOW_DISPLAY_STATE_WINDOWED_MAXIMIZED;
        } else {
            return WINDOW_DISPLAY_STATE_WINDOWED;
        }
    }
}

void window__set_hidden(window_t self, bool value) {
    if (value) {
        glfwHideWindow(self->glfw_window);
    } else {
        glfwShowWindow(self->glfw_window);
    }
}

bool window__get_hidden(window_t self) {
    return glfwGetWindowAttrib(self->glfw_window, GLFW_VISIBLE);
}

void window__set_focus(window_t self) {
    glfwFocusWindow(self->glfw_window);
}

bool window__get_focus(window_t self) {
    return glfwGetWindowAttrib(self->glfw_window, GLFW_FOCUSED);
}

void window__set_should_close(window_t self, bool value) {
    glfwSetWindowShouldClose(self->glfw_window, value);
}

bool window__get_should_close(window_t self) {
    return glfwWindowShouldClose(self->glfw_window);
}

void window__request_attention(window_t self) {
    glfwRequestWindowAttention(self->glfw_window);
}

void window__set_window_opacity(window_t self, float opacity) {
    ASSERT(opacity >= 0.0f && opacity <= 1.0f);

    glfwSetWindowOpacity(self->glfw_window, opacity);
}

float window__get_window_opacity(window_t self) {
    return glfwGetWindowOpacity(self->glfw_window);
}

void window__set_size_limit(
    window_t self,
    uint32_t min_width, uint32_t min_height,
    uint32_t max_width, uint32_t max_height
) {
    glfwSetWindowSizeLimits(
        self->glfw_window,
        min_width == 0 ? GLFW_DONT_CARE : (int32_t) min_width, min_height == 0 ? GLFW_DONT_CARE : (int32_t) min_height,
        max_width == 0 ? GLFW_DONT_CARE : (int32_t) max_width, max_height == 0 ? GLFW_DONT_CARE : (int32_t) max_height
    );
}

void window__set_aspect_ratio(window_t self, uint32_t num, uint32_t den) {
    if (num == 0 || den == 0) {
        num = GLFW_DONT_CARE;
        den = GLFW_DONT_CARE;
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window aspect disabled");
    } else {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window aspect ratio changed to: %u %u", num, den);
    }
    glfwSetWindowAspectRatio(self->glfw_window, num, den);
}

const char* window__get_clipboard(window_t self) {
    return glfwGetClipboardString(self->glfw_window);
}

void window__set_clipboard(window_t self, const char* str) {
    glfwSetClipboardString(self->glfw_window, str);
}

controller_t* window__get_controller(window_t self) {
    return &self->controller;
}

bool controller__button_is_down(controller_t* self, button_t button) {
    ASSERT(button < _BUTTON_SIZE);
    return self->buttons[button].ended_down;
}

uint32_t controller__button_n_of_repeats(controller_t* self, button_t button) {
    ASSERT(button < _BUTTON_SIZE);
    return self->buttons[button].n_of_repeats;
}

uint32_t controller__button_n_of_transitions(controller_t* self, button_t button) {
    ASSERT(button < _BUTTON_SIZE);
    return self->buttons[button].n_of_transitions;
}

void controller__get_cursor_pos(controller_t* self, double* x, double* y) {
    *x = self->cursor_x;
    *y = self->cursor_y;
}

void controller__button_register_action(controller_t* self, button_t button, void* user_pointer, void (*action_on_button_down)(void*)) {
    self->buttons[button].action_on_button_down = action_on_button_down;
    self->buttons[button].user_pointer          = user_pointer;
}

void window__set_cursor_state(window_t self, cursor_state_t state) {
    int glfw_cursor_state;

    switch (state) {
    case CURSOR_DISABLED: {
        glfw_cursor_state = GLFW_CURSOR_DISABLED;
    } break ;
    case CURSOR_ENABLED: {
        glfw_cursor_state = GLFW_CURSOR_NORMAL;
    } break ;
    case CURSOR_HIDDEN: {
        glfw_cursor_state = GLFW_CURSOR_HIDDEN;
    } break ;
    default: ASSERT(false);
    }

    glfwSetInputMode(self->glfw_window, GLFW_CURSOR, glfw_cursor_state);
}

cursor_state_t window__get_cursor_state(window_t self) {
    int glfw_cursor_state = glfwGetInputMode(self->glfw_window, GLFW_CURSOR);

    switch (glfw_cursor_state) {
    case GLFW_CURSOR_DISABLED: return CURSOR_DISABLED;
    case GLFW_CURSOR_NORMAL: return CURSOR_ENABLED;
    case GLFW_CURSOR_HIDDEN: return CURSOR_HIDDEN;
    default: ASSERT(false && "not implemented cursor mode");
    }

    return 0;
}

cursor_t cursor__create(uint8_t* pixels, uint32_t w, uint32_t h) {
    GLFWimage cursor_image = {
        .pixels = pixels,
        .width  = w,
        .height = h
    };

    GLFWcursor* glfw_cursor = glfwCreateCursor(&cursor_image, 0, 0);
    if (!glfw_cursor) {
        return 0;
    }

    cursor_t result = calloc(1, sizeof(*result));

    result->glfw_cursor = glfw_cursor;

    return result;
}

void cursor__destroy(cursor_t cursor) {
    glfwDestroyCursor(cursor->glfw_cursor);

    free(cursor);
}

void window__set_cursor(window_t self, cursor_t cursor) {
    glfwSetCursor(self->glfw_window, cursor->glfw_cursor);
}

void window__reset_cursor(window_t self) {
    glfwSetCursor(self->glfw_window, NULL);
}

bool window__cursor_is_in_content_area(window_t self) {
    return glfwGetWindowAttrib(self->glfw_window, GLFW_HOVERED);
}

void window__set_viewport(window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height) {
    window__set_current(self);

#if defined(OPENGL)
    gl__viewport(x, y, width, height);
#elif defined(VULKAN)
    (void) x;
    (void) y;
    (void) width;
    (void) height;
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
}

void window__set_scissor(window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height) {
    window__set_current(self);

#if defined(OPENGL)
    gl__scissor(x, y, width, height);
#elif defined(VULKAN)
    (void) x;
    (void) y;
    (void) width;
    (void) height;
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
}

void window__swap_buffers(window_t self) {
    window__set_current(self);

#if defined(OPENGL)
    glfwSwapBuffers(self->glfw_window);
#elif defined(VULKAN)
    vk__render();
#else
    #error "undefined backend, must either be OPENGL or VULKAN"
#endif
}
