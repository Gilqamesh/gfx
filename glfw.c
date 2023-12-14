#include "glfw.h"

#include "debug.h"
#include "helper_macros.h"
#include "gl.h"

#include <stdlib.h>
#include <string.h>

#include "glfw_internal.c"

bool glfw__init() {
    memset(&glfw, 0, sizeof(glfw));

    glfwSetErrorCallback(&glfw__error_callback);

    debug__write("version string: %s", glfwGetVersionString());
    debug__write("compiled version: %d.%d.%d", GLFW_VERSION_MAJOR, GLFW_VERSION_MINOR, GLFW_VERSION_REVISION);

    int major = 0;
    int minor = 0;
    int rev   = 0;
    glfwGetVersion(&major, &minor, &rev);
    debug__write("runtime version: %d.%d.%d", major, minor, rev);

    // glfwInitHint();
    if (glfwInit() == GLFW_FALSE) {
        debug__write("glfwInit == GLFW_FALSE");
        debug__flush(DEBUG_MODULE_GLFW, DEBUG_ERROR);
        return false;
    }

    glfwSetMonitorCallback(&glfw__monitor_callback);
    glfwSetJoystickCallback(&glfw__controller_callback);

    uint32_t number_of_monitors;
    monitor_t* monitors = monitor__get_monitors(&number_of_monitors);
    debug__write("number of connected monitors: %u", number_of_monitors);
    // todo: turn this into column-based format
    for (uint32_t monitor_index = 0; monitor_index < number_of_monitors; ++monitor_index) {
        monitor_t monitor = monitors[monitor_index];
        int32_t width_mm;
        int32_t height_mm;
        glfwGetMonitorPhysicalSize(monitor, &width_mm, &height_mm);
        debug__write("  %u: %s - physical size: %ux%u [mm]", monitor_index, glfwGetMonitorName(monitor), width_mm, height_mm);
    }

    debug__flush(DEBUG_MODULE_GLFW, DEBUG_INFO);

    for (uint32_t controller_index = 0; controller_index < ARRAY_SIZE(glfw.controller); ++controller_index) {
        if (glfwJoystickIsGamepad(controller_index)) {
            controller_t* controller = &glfw.controller[controller_index];
            controller->name = glfwGetJoystickName(controller_index);
            controller__set_connected(controller, true);
        }
    }

    return true;
}

void glfw__deinit() {
    glfwTerminate();
}

void glfw__poll_events() {
    glfw__pre_poll_events();
    glfwPollEvents();
    glfw__post_poll_events();
}

void glfw__wait_events() {
    glfw__pre_poll_events();
    glfwWaitEvents();
    glfw__post_poll_events();
}

double glfw__get_time_s() {
    return glfwGetTime();
}

const char* window__get_clipboard(window_t self) {
    return glfwGetClipboardString(self->glfw_window);
}

void window__set_clipboard(window_t self, const char* str) {
    glfwSetClipboardString(self->glfw_window, str);
}

monitor_t* monitor__get_monitors(uint32_t* number_of_monitors) {
    return (monitor_t*) glfwGetMonitors((int32_t*) number_of_monitors);
}

void monitor__get_screen_size(monitor_t self, uint32_t* w, uint32_t* h) {
    const GLFWvidmode* mode = glfwGetVideoMode(self);
    *w = mode->width;
    *h = mode->height;
}

void monitor__get_work_area(monitor_t self, int32_t* x, int32_t* y, uint32_t* w, uint32_t* h) {
    glfwGetMonitorWorkarea(self, x, y, (int32_t*) w, (int32_t*) h);
}

const char* button__to_str(button_t button) {
    switch (button) {
    case BUTTON_0: return "0"; case BUTTON_1: return "1"; case BUTTON_2: return "2"; case BUTTON_3: return "3"; case BUTTON_4: return "4"; case BUTTON_5: return "5"; case BUTTON_6: return "6"; case BUTTON_7: return "7"; case BUTTON_8: return "8"; case BUTTON_9: return "9";
    case BUTTON_A: return "A"; case BUTTON_B: return "B";
    case BUTTON_C: return "C"; case BUTTON_D: return "D"; case BUTTON_E: return "E"; case BUTTON_F: return "F"; case BUTTON_G: return "G"; case BUTTON_H: return "H"; case BUTTON_I: return "I"; case BUTTON_J: return "J"; case BUTTON_K: return "K"; case BUTTON_L: return "L"; case BUTTON_M: return "M";
    case BUTTON_N: return "N"; case BUTTON_O: return "O"; case BUTTON_P: return "P"; case BUTTON_Q: return "Q"; case BUTTON_R: return "R"; case BUTTON_S: return "S"; case BUTTON_T: return "T"; case BUTTON_U: return "U"; case BUTTON_V: return "V"; case BUTTON_W: return "W"; case BUTTON_X: return "X"; case BUTTON_Y: return "Y"; case BUTTON_Z: return "Z";

    case BUTTON_LEFT: return "LEFT"; case BUTTON_UP: return "UP"; case BUTTON_RIGHT: return "RIGHT"; case BUTTON_DOWN: return "DOWN";
    case BUTTON_CAPS_LOCK: return "CAPS_LOCK"; case BUTTON_SHIFT: return "SHIFT";
    case BUTTON_SPACE: return "SPACE"; case BUTTON_BACKSPACE: return "BACK_SPACE";
    case BUTTON_ENTER: return "ENTER";

    case BUTTON_FPS_LOCK_INC: return "FPS_LOCK_INC";
    case BUTTON_FPS_LOCK_DEC: return "FPS_LOCK_DEC";

    case BUTTON_WINDOW_CLOSE: return "WINDOW_CLOSE";
    case BUTTON_WINDOW_MINIMIZE: return "WINDOW_MINIMIZE";
    case BUTTON_WINDOW_MAXIMIZE: return "WINDOW_MAXIMIZE";
    case BUTTON_WINDOW_WINDOWED: return "WINDOW_WINDOWED";
    case BUTTON_WINDOW_FULL_SCREEN: return "WINDOW_FULL_SCREEN";

    case BUTTON_DEBUG_INFO_MESSAGE_TOGGLE: return "DEBUG_INFO_MESSAGE_TOGGLE";

    case BUTTON_GAMEPAD_A: return "GAMEPAD_A"; case BUTTON_GAMEPAD_B: return "GAMEPAD_B"; case BUTTON_GAMEPAD_X: return "GAMEPAD_X"; case BUTTON_GAMEPAD_Y: return "GAMEPAD_Y";
    case BUTTON_GAMEPAD_LEFT_BUMPER: return "GAMEPAD_LEFT_BUMPER"; case BUTTON_GAMEPAD_RIGHT_BUMPER: return "GAMEPAD_RIGHT_BUMPER"; case BUTTON_GAMEPAD_BACK: return "GAMEPAD_BACK";
    case BUTTON_GAMEPAD_START: return "GAMEPAD_START"; case BUTTON_GAMEPAD_GUIDE: return "GAMEPAD_GUIDE"; case BUTTON_GAMEPAD_LEFT_THUMB: return "GAMEPAD_LEFT_THUMB"; case BUTTON_GAMEPAD_RIGHT_THUMB: return "GAMEPAD_RIGHT_THUMB";
    case BUTTON_GAMEPAD_DPAD_UP: return "GAMEPAD_DPAD_UP"; case BUTTON_GAMEPAD_DPAD_RIGHT: return "GAMEPAD_DPAD_RIGHT"; case BUTTON_GAMEPAD_DPAD_DOWN: return "GAMEPAD_DPAD_DOWN"; case BUTTON_GAMEPAD_DPAD_LEFT: return "GAMEPAD_DPAD_LEFT";
    
    case BUTTON_GAMEPAD_AXIS_LEFT_X: return "GAMEPAD_AXIS_LEFT_X"; case BUTTON_GAMEPAD_AXIS_LEFT_Y: return "GAMEPAD_AXIS_LEFT_Y"; case BUTTON_GAMEPAD_AXIS_RIGHT_X: return "GAMEPAD_AXIS_RIGHT_X"; case BUTTON_GAMEPAD_AXIS_RIGHT_Y: return "GAMEPAD_AXIS_RIGHT_Y";
    case BUTTON_GAMEPAD_AXIS_LEFT_TRIGGER: return "GAMEPAD_AXIS_LEFT_TRIGGER"; case BUTTON_GAMEPAD_AXIS_RIGHT_TRIGGER: return "GAMEPAD_AXIS_RIGHT_TRIGGER";

    case BUTTON_CURSOR_LEFT: return "CURSOR_LEFT";
    case BUTTON_CURSOR_RIGHT: return "CURSOR_RIGHT";
    case BUTTON_CURSOR_MIDDLE: return "CURSOR_MIDDLE";

    case BUTTON_GET_CLIPBOARD: return "GET_CLIPBOARD";
    case BUTTON_SET_CLIPBOARD: return "SET_CLIPBOARD";

    default: ASSERT(false);
    }

    return 0;
}

window_t window__create(monitor_t monitor, const char* title, uint32_t width, uint32_t height) {
    if (glfw.windows_top == glfw.windows_size) {
        uint32_t windows_prev_size = glfw.windows_size;
        if (glfw.windows_size == 0) {
            glfw.windows_size = 4;
            glfw.windows = malloc(glfw.windows_size * sizeof(*glfw.windows));
        } else {
            glfw.windows_size <<= 1;
            glfw.windows = realloc(glfw.windows, glfw.windows_size * sizeof(*glfw.windows));
        }
        for (uint32_t window_index = windows_prev_size; window_index < glfw.windows_size; ++window_index) {
            glfw.windows[window_index] = malloc(sizeof(*glfw.windows[window_index]));
        }
    }
    assert(glfw.windows_top < glfw.windows_size);
    window_t result = glfw.windows[glfw.windows_top++];
    memset(result, 0, sizeof(*result));

    result->title = title;
    result->monitor = monitor;
    result->controller.name = title;
    controller__set_connected(&result->controller, true);

    // for faster window creation and application switching, choose the closest video mode available (relative to monitor)
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    result->glfw_window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!result->glfw_window) {
        window__destroy(result);
        return 0;
    }

    window__set_current_window(result);
    if (!gl__init((GLADloadproc) glfwGetProcAddress)) {
        return 0;
    }

    // glfwSetJoystickUserPointer();

    window__set_hidden(result, true);
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

        window__get_content_area_size(result, &result->content_area_w, &result->content_area_h);
        window__get_content_area_pos(result, &result->content_area_x, &result->content_area_y);
    }
    window__set_hidden(result, false);

    return result;
}

void window__destroy(window_t self) {
    glfwDestroyWindow(self->glfw_window);
    memset(self, 0, sizeof(*self));

    bool found_window = false;
    for (uint32_t window_index = 0; window_index < glfw.windows_top; ++window_index) {
        if (glfw.windows[window_index] == self) {
            found_window = true;
            while (window_index < glfw.windows_top - 1) {
                glfw.windows[window_index] = glfw.windows[window_index + 1];
                ++window_index;
            }
            break ;
        }
    }

    ASSERT(found_window);

    --glfw.windows_top;

    // todo: realloc glfw.windows_size to save space?
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

void window__set_current_window(window_t self) {
    glfwMakeContextCurrent(self->glfw_window);
    glfwSwapInterval(1);
}

window_t window__get_current_window() {
    GLFWwindow* glfw_window = glfwGetCurrentContext();
    return window__from_glfw_window(glfw_window);
}

void window__swap_buffers(window_t self) {
    glfwSwapBuffers(self->glfw_window);
}

void window__set_icon(window_t self, uint8_t* pixels, uint32_t w, uint32_t h) {
    GLFWimage image = {
        .pixels = pixels,
        .width = w,
        .height = h
    };
    glfwSetWindowIcon(self->glfw_window, 1, &image);
}

void window__set_content_area_size(window_t self, uint32_t width, uint32_t height) {
    glfwSetWindowSize(self->glfw_window, width, height);
}

void window__get_content_area_size(window_t self, uint32_t* width, uint32_t* height) {
    glfwGetWindowSize(self->glfw_window, (int32_t*) width, (int32_t*) height);
}

void window__get_window_size(window_t self, uint32_t* width, uint32_t* height) {
    window__get_content_area_size(self, width, height);
    int32_t left, top, right, bottom;
    window__get_window_frame_size(self, &left, &top, &right, &bottom);
    *width += left + right;
    *height += top + bottom;
}

void window__get_window_frame_size(window_t self, int32_t* left, int32_t* top, int32_t* right, int32_t* bottom) {
    glfwGetWindowFrameSize(self->glfw_window, left, top, right, bottom);
}

void window__set_content_area_pos(window_t self, int32_t x, int32_t y) {
    glfwSetWindowPos(self->glfw_window, x, y);
}

void window__get_content_area_pos(window_t self, int32_t* x, int32_t* y) {
    glfwGetWindowPos(self->glfw_window, x, y);
}

void window__set_window_pos(window_t self, int32_t x, int32_t y) {
    int32_t left, top, right, bottom;
    window__get_window_frame_size(self, &left, &top, &right, &bottom);
    window__set_content_area_pos(self, x + left, y + top);
}

void window__get_window_pos(window_t self, int32_t* x, int32_t* y) {
    int32_t left, top, right, bottom;
    window__get_window_frame_size(self, &left, &top, &right, &bottom);
    window__get_content_area_pos(self, x, y);
    *x -= left;
    *y -= top;
}

void window__set_state(window_t self, window_state_t state) {
    const bool was_hidden = window__get_hidden(self);
    if (!was_hidden) {
        window__set_hidden(self, true);
    }

    switch (state) {
    case WINDOW_STATE_MINIMIZED: {
        glfwIconifyWindow(self->glfw_window);
    } break ;
    case WINDOW_STATE_MAXIMIZED: {
        glfwMaximizeWindow(self->glfw_window);
    } break ;
    case WINDOW_STATE_WINDOWED: {
        window_state_t current_state = window__get_state(self);
        if (
            current_state == WINDOW_STATE_MINIMIZED ||
            current_state == WINDOW_STATE_MAXIMIZED
        ) {
            glfwRestoreWindow(self->glfw_window);
        }

        glfwSetWindowMonitor(
            self->glfw_window,
            NULL,
            self->content_area_x, self->content_area_y,
            self->content_area_w, self->content_area_h,
            GLFW_DONT_CARE
        );
    } break ;
    case WINDOW_STATE_FULL_SCREEN: {
        window_state_t current_state = window__get_state(self);
        if (
            current_state == WINDOW_STATE_MINIMIZED ||
            current_state == WINDOW_STATE_MAXIMIZED
        ) {
            glfwRestoreWindow(self->glfw_window);
        }

        const GLFWvidmode* mode = glfwGetVideoMode(self->monitor);
        glfwSetWindowMonitor(
            self->glfw_window,
            self->monitor,
            0, 0,
            mode->width, mode->height,
            mode->refreshRate
        );
    } break ;
    default: ASSERT(false);
    }

    if (!was_hidden) {
        window__set_hidden(self, false);
    }
}

window_state_t window__get_state(window_t self) {
    if (glfwGetWindowAttrib(self->glfw_window, GLFW_ICONIFIED)) {
        return WINDOW_STATE_MINIMIZED;
    }
    if (glfwGetWindowAttrib(self->glfw_window, GLFW_MAXIMIZED)) {
        return WINDOW_STATE_MAXIMIZED;
    }
    if (glfwGetWindowMonitor(self->glfw_window)) {
        return WINDOW_STATE_FULL_SCREEN;
    }

    return WINDOW_STATE_WINDOWED;

    // what about hidden state?
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

bool window__button_is_down(window_t self, button_t button) {
    ASSERT(button < _BUTTON_SIZE);
    return self->controller.buttons[button].ended_down;
}

uint32_t window__button_n_of_repeats(window_t self, button_t button) {
    ASSERT(button < _BUTTON_SIZE);
    return self->controller.buttons[button].n_of_repeats;
}

uint32_t window__button_n_of_transitions(window_t self, button_t button) {
    ASSERT(button < _BUTTON_SIZE);
    return self->controller.buttons[button].n_of_transitions;
}

void window__button_register_action(window_t self, button_t button, void* user_pointer, void (*action_on_button_down)(void*)) {
    self->controller.buttons[button].action_on_button_down = action_on_button_down;
    self->controller.buttons[button].user_pointer          = user_pointer;
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

void window__set_cursor_pos(window_t self, double x, double y) {
    (void) self;
    (void) x;
    (void) y;
}

void window__get_cursor_pos(window_t self, double* x, double* y) {
    glfwGetCursorPos(self->glfw_window, x, y);
}

cursor_t cursor__create(uint8_t* pixels, uint32_t w, uint32_t h) {
    GLFWimage cursor_image = {
        .pixels = pixels,
        .width  = w,
        .height = h
    };

    return glfwCreateCursor(&cursor_image, 0, 0);
}

void cursor__destroy(cursor_t cursor) {
    glfwDestroyCursor(cursor);
}

void window__set_cursor(window_t self, cursor_t cursor) {
    glfwSetCursor(self->glfw_window, cursor);
}

void window__reset_cursor(window_t self) {
    glfwSetCursor(self->glfw_window, NULL);
}

bool window__cursor_is_in_content_area(window_t self) {
    return glfwGetWindowAttrib(self->glfw_window, GLFW_HOVERED);
}
