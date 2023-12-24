struct         controller;
struct         gfx;
typedef struct controller controller_t;
typedef struct gfx        gfx_t;

struct controller {
    const char*    name;
    button_state_t buttons[_BUTTON_SIZE];
    bool           is_connected;
    bool           received_button_input;
    float          axes[ARRAY_SIZE(((GLFWgamepadstate*)(0))->axes)];
};

struct gfx {
    /**
     * @brief global controllers that affect all windows, such as gamepads
    */
    controller_t controller[16];
    
    window_t* windows;
    uint32_t  windows_top;
    uint32_t  windows_size;

    monitor_t* monitors;
    uint32_t   monitors_top;
    uint32_t   monitors_size;

    window_t current_window;
};

struct monitor {
    GLFWmonitor* glfw_monitor;
};

struct window {
    GLFWwindow*    glfw_window;
    const char*    title;
    const char*    clipboard;

    window_display_state_t requested_display_state;
    bool                   transitioning;

    /**
     * @note to transition back to WINDOW_DISPLAY_STATE_WINDOWED
     */
    int32_t  content_area_x;
    int32_t  content_area_y;
    uint32_t content_area_w;
    uint32_t content_area_h;

    // todo: I think this can be removed as it's unnecessary state as it can be queried, so check where it's necessary
    monitor_t monitor;

    controller_t controller;
};

struct cursor {
    GLFWcursor* glfw_cursor;
};

static gfx_t gfx;

static void gfx__pre_poll_events();
static void gfx__post_poll_events();
static void gfx__post_poll_event_poll_controllers();
static void gfx__post_poll_event_handle_window_display_state_transitions();
static void gfx__error_callback(int code, const char* description);
static void gfx__monitor_callback(GLFWmonitor* monitor, int event);
static void gfx__controller_callback(int jid, int event);

static void window__display_state_transition(window_t self);
static void window__should_close_callback(GLFWwindow* glfw_window);
static void window__pos_changed_callback(GLFWwindow* glfw_window, int x, int y);
static void window__size_changed_callback(GLFWwindow* glfw_window, int width, int height);
static window_t window__from_glfw_window(GLFWwindow* glfw_window);
static void window__framebuffer_resize_callback(GLFWwindow* glfw_window, int width, int height);
static void window__content_scale_callback(GLFWwindow* glfw_window, float xscale, float yscale);
static void window__minimized_callback(GLFWwindow* glfw_window, int minimized);
static void window__maximized_callback(GLFWwindow* glfw_window, int maximized);
static void window__focus_callback(GLFWwindow* glfw_window, int focused);
static void window__key_callback(GLFWwindow* glfw_window, int key, int platform_specific_scancode, int action, int mods);
static void window__utf32_callback(GLFWwindow* window, uint32_t utf32);
static void window__add_default_button_actions(window_t self);
static void window__button_default_action_window_close(void* user_pointer);
static void window__button_default_action_window_minimize(void* user_pointer);
static void window__button_default_action_window_maximize(void* user_pointer);
static void window__button_default_action_window_windowed(void* user_pointer);
static void window__button_default_action_window_full_screen(void* user_pointer);
static void window__button_default_action_debug_info_message_toggle(void* user_pointer);
static void window__button_default_action_get_clipboard(void* user_pointer);
static void window__button_default_action_set_clipboard(void* user_pointer);
static void window__cursor_pos_callback(GLFWwindow* glfw_window, double x, double y);
static void window__button_process_input(window_t self, button_t button, bool is_pressed);
static void window__cursor_enter_callback(GLFWwindow* glfw_window, int entered);
static void window__cursor_button_callback(GLFWwindow* glfw_window, int button, int action, int mods);
static void window__cursor_scroll_callback(GLFWwindow* glfw_window, double xoffset, double yoffset);
static void window__drop_callback(GLFWwindow* glfw_window, int paths_size, const char** paths);
static void window__set_fullscreen(window_t self, bool value);
static bool window__get_fullscreen(window_t self);
static void window__set_minimized(window_t self, bool value);
static bool window__get_minimized(window_t self);
static void window__set_maximized(window_t self, bool value);
static bool window__get_maximized(window_t self);
static void window__restore_content_area(window_t self);

static void controller__button_process_input(controller_t* self, button_t button, bool is_pressed);
static void controller__button_process_axes(controller_t* self, button_t button, uint32_t axes_index, float value);
static void controller__clear(controller_t* self);
static void controller__set_connected(controller_t* self, bool value);
static bool controller__get_connected(controller_t* self);

static const char* button__to_str(button_t button);

static void gfx__pre_poll_events() {
    for (uint32_t controller_index = 0; controller_index < ARRAY_SIZE(gfx.controller); ++controller_index) {
        if (gfx.controller[controller_index].is_connected) {
            controller__clear(&gfx.controller[controller_index]);
        }
    }
    for (uint32_t window_index = 0; window_index < gfx.windows_top; ++window_index) {
        window_t window = gfx.windows[window_index];
        controller__clear(&window->controller);

        if (window__get_display_state(window) == WINDOW_DISPLAY_STATE_WINDOWED) {
            glfwGetWindowPos(window->glfw_window, &window->content_area_x, &window->content_area_y);
            glfwGetWindowSize(window->glfw_window, (int32_t*) &window->content_area_w, (int32_t*) &window->content_area_h);
        }
    }
}

static void gfx__post_poll_events() {
    gfx__post_poll_event_poll_controllers();
    gfx__post_poll_event_handle_window_display_state_transitions();
}

static void gfx__post_poll_event_poll_controllers() {
    static button_t glfw_gamepad_button_to_button[ARRAY_SIZE(((GLFWgamepadstate*)(0))->buttons)] = {
        [GLFW_GAMEPAD_BUTTON_A]             = BUTTON_GAMEPAD_A,
        [GLFW_GAMEPAD_BUTTON_B]             = BUTTON_GAMEPAD_B,
        [GLFW_GAMEPAD_BUTTON_X]             = BUTTON_GAMEPAD_X,
        [GLFW_GAMEPAD_BUTTON_Y]             = BUTTON_GAMEPAD_Y,
        [GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]   = BUTTON_GAMEPAD_LEFT_BUMPER,
        [GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER]  = BUTTON_GAMEPAD_RIGHT_BUMPER,
        [GLFW_GAMEPAD_BUTTON_BACK]          = BUTTON_GAMEPAD_BACK,
        [GLFW_GAMEPAD_BUTTON_START]         = BUTTON_GAMEPAD_START,
        [GLFW_GAMEPAD_BUTTON_GUIDE]         = BUTTON_GAMEPAD_GUIDE,
        [GLFW_GAMEPAD_BUTTON_LEFT_THUMB]    = BUTTON_GAMEPAD_LEFT_THUMB,
        [GLFW_GAMEPAD_BUTTON_RIGHT_THUMB]   = BUTTON_GAMEPAD_RIGHT_THUMB,
        [GLFW_GAMEPAD_BUTTON_DPAD_UP]       = BUTTON_GAMEPAD_DPAD_UP,
        [GLFW_GAMEPAD_BUTTON_DPAD_RIGHT]    = BUTTON_GAMEPAD_DPAD_RIGHT,
        [GLFW_GAMEPAD_BUTTON_DPAD_DOWN]     = BUTTON_GAMEPAD_DPAD_DOWN,
        [GLFW_GAMEPAD_BUTTON_DPAD_LEFT]     = BUTTON_GAMEPAD_DPAD_LEFT
    };
    static button_t glfw_gamepad_axis_to_button[ARRAY_SIZE(((GLFWgamepadstate*)(0))->axes)] = {
        [GLFW_GAMEPAD_AXIS_LEFT_X] = BUTTON_GAMEPAD_AXIS_LEFT_X,
        [GLFW_GAMEPAD_AXIS_LEFT_Y] = BUTTON_GAMEPAD_AXIS_LEFT_Y,
        [GLFW_GAMEPAD_AXIS_RIGHT_X] = BUTTON_GAMEPAD_AXIS_RIGHT_X,
        [GLFW_GAMEPAD_AXIS_RIGHT_Y] = BUTTON_GAMEPAD_AXIS_RIGHT_Y,
        [GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] = BUTTON_GAMEPAD_AXIS_LEFT_TRIGGER,
        [GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] = BUTTON_GAMEPAD_AXIS_RIGHT_TRIGGER
    };
    for (uint32_t controller_index = 0; controller_index < ARRAY_SIZE(gfx.controller); ++controller_index) {
        controller_t* controller = &gfx.controller[controller_index];
        if (controller->is_connected) {
            GLFWgamepadstate state;
            glfwGetGamepadState(controller_index, &state);
            for (uint32_t button_index = 0; button_index < ARRAY_SIZE(state.buttons); ++button_index) {
                button_t button = glfw_gamepad_button_to_button[button_index];
                controller__button_process_input(controller, button, state.buttons[button_index] == GLFW_PRESS);
            }
            for (uint32_t axis_index = 0; axis_index < ARRAY_SIZE(state.axes); ++axis_index) {
                button_t button = glfw_gamepad_axis_to_button[axis_index];
                controller__button_process_axes(controller, button, axis_index, state.axes[axis_index]);
            }
        }
    }
}

static void gfx__post_poll_event_handle_window_display_state_transitions() {
    for (uint32_t window_index = 0; window_index < gfx.windows_top; ++window_index) {
        window_t window = gfx.windows[window_index];
        window__display_state_transition(window);
    }
}

static void window__display_state_transition(window_t self) {
    const window_display_state_t current_state  = window__get_display_state(self);
    const window_display_state_t new_state      = self->requested_display_state;

    const bool was_hidden = window__get_hidden(self);
    if (!was_hidden) {
        window__set_hidden(self, true);
    }

    if (current_state == new_state) {
        return ;
    }

    /*
    states:
        windowed   normal
        windowed   minimized
        windowed   maximized
        fullscreen normal
        fullscreen minimized

    state transitions:                                  Apply
        windowed normal      -> windowed minimized      minimize(true)
        windowed normal      -> windowed maximized      maximize(true)
        windowed normal      -> fullscreen normal       fullscreen(true)
        windowed normal      -> fullscreen minimized    fullscreen(true) -> minimize(true)

        windowed minimized   -> windowed normal         minimize(false) -> restore content area
        windowed minimized   -> windowed maximized      minimize(false) -> maximize(true)
        windowed minimized   -> fullscreen normal       minimize(false) -> fullscreen(true)
        windowed minimized   -> fullscreen minimized    minimize(false) -> fullscreen(true) -> minimize(true)

        windowed maximized   -> windowed normal         maximize(false) -> restore content area
        windowed maximized   -> windowed minimized      maximize(false) -> minimize(true)
        windowed maximized   -> fullscreen normal       maximize(false) -> fullscreen(true)
        windowed maximized   -> fullscreen minimized    maximize(false) -> fullscreen(true) -> minimize(true)

        fullscreen normal    -> windowed normal         fullscreen(false) -> maximize(true) -> schedule windowed normal (windowed maximized -> windowed normal) 
        fullscreen normal    -> windowed minimized      fullscreen(false) -> minimize(true)
        fullscreen normal    -> windowed maximized      fullscreen(false) -> maximize(true)
        fullscreen normal    -> fullscreen minimized    minimize(true)

        fullscreen minimized -> windowed normal         minimize(false) -> fullscreen(false) -> restore content area
        fullscreen minimized -> windowed minimized      minimize(false) -> fullscreen(false) -> minimize(true)
        fullscreen minimized -> windowed maximized      minimize(false) -> fullscreen(false) -> maximize(true)
        fullscreen minimized -> fullscreen normal       minimize(false)
    */

    switch (current_state) {
    case WINDOW_DISPLAY_STATE_WINDOWED: {
        switch (new_state) {
        case WINDOW_DISPLAY_STATE_WINDOWED: {
            ASSERT(false);
        } break ;
        case WINDOW_DISPLAY_STATE_WINDOWED_MINIMIZED: {
            window__set_minimized(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_WINDOWED_MAXIMIZED: {
            window__set_maximized(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_FULLSCREEN: {
            window__set_fullscreen(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_MINIMIZED_FULLSCREEN: {
            window__set_fullscreen(self, true);
            window__set_minimized(self, true);
        } break ;
        default: ASSERT(false);
        }
    } break ;
    case WINDOW_DISPLAY_STATE_WINDOWED_MINIMIZED: {
        window__set_minimized(self, false);

        switch (new_state) {
        case WINDOW_DISPLAY_STATE_WINDOWED: {
            window__restore_content_area(self);
        } break ;
        case WINDOW_DISPLAY_STATE_WINDOWED_MINIMIZED: {
            ASSERT(false);
        } break ;
        case WINDOW_DISPLAY_STATE_WINDOWED_MAXIMIZED: {
            window__set_maximized(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_FULLSCREEN: {
            window__set_fullscreen(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_MINIMIZED_FULLSCREEN: {
            window__set_fullscreen(self, true);
            window__set_minimized(self, true);
        } break ;
        default: ASSERT(false);
        }
    } break ;
    case WINDOW_DISPLAY_STATE_WINDOWED_MAXIMIZED: {
        window__set_maximized(self, false);

        switch (new_state) {
        case WINDOW_DISPLAY_STATE_WINDOWED: {
            window__restore_content_area(self);
        } break ;
        case WINDOW_DISPLAY_STATE_WINDOWED_MINIMIZED: {
            window__set_minimized(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_WINDOWED_MAXIMIZED: {
            ASSERT(false);
        } break ;
        case WINDOW_DISPLAY_STATE_FULLSCREEN: {
            window__set_fullscreen(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_MINIMIZED_FULLSCREEN: {
            window__set_fullscreen(self, true);
            window__set_minimized(self, true);
        } break ;
        default: ASSERT(false);
        }
    } break ;
    case WINDOW_DISPLAY_STATE_FULLSCREEN: {
        switch (new_state) {
        case WINDOW_DISPLAY_STATE_WINDOWED: {
            window__set_fullscreen(self, false);
            window__set_maximized(self, true);
            self->requested_display_state = WINDOW_DISPLAY_STATE_WINDOWED;
        } break ;
        case WINDOW_DISPLAY_STATE_WINDOWED_MINIMIZED: {
            window__set_fullscreen(self, false);
            window__set_minimized(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_WINDOWED_MAXIMIZED: {
            window__set_fullscreen(self, false);
            window__set_maximized(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_FULLSCREEN: {
            ASSERT(false);
        } break ;
        case WINDOW_DISPLAY_STATE_MINIMIZED_FULLSCREEN: {
            window__set_minimized(self, true);
        } break ;
        default: ASSERT(false);
        }
    } break ;
    case WINDOW_DISPLAY_STATE_MINIMIZED_FULLSCREEN: {
        window__set_minimized(self, false);
        switch (new_state) {
        case WINDOW_DISPLAY_STATE_WINDOWED: {
            window__set_fullscreen(self, false);
            window__restore_content_area(self);
        } break ;
        case WINDOW_DISPLAY_STATE_WINDOWED_MINIMIZED: {
            window__set_fullscreen(self, false);
            window__set_minimized(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_WINDOWED_MAXIMIZED: {
            window__set_fullscreen(self, false);
            window__set_maximized(self, true);
        } break ;
        case WINDOW_DISPLAY_STATE_FULLSCREEN: {
        } break ;
        case WINDOW_DISPLAY_STATE_MINIMIZED_FULLSCREEN: {
            ASSERT(false);
        } break ;
        default: ASSERT(false);
        }
    } break ;
    default: ASSERT(false);
    }

    if (!was_hidden) {
        window__set_hidden(self, false);
    }
}

static void gfx__error_callback(int code, const char* description) {
    debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_ERROR, "code: [%d], description: [%s]", code, description);
}

static void gfx__monitor_callback(GLFWmonitor* glfw_monitor, int event) {
    if (event == GLFW_CONNECTED) {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "monitor %s has connected", glfwGetMonitorName(glfw_monitor));
        if (gfx.monitors_top == gfx.monitors_size) {
            uint32_t monitors_prev_size = gfx.monitors_size;
            if (gfx.monitors_size == 0) {
                gfx.monitors_size = 4;
                gfx.monitors = malloc(gfx.monitors_size * sizeof(*gfx.monitors));
            } else {
                gfx.monitors_size <<= 1;
                gfx.monitors = realloc(gfx.monitors, gfx.monitors_size * sizeof(*gfx.monitors));
            }
            for (uint32_t monitor_index = monitors_prev_size; monitor_index < gfx.monitors_size; ++monitor_index) {
                gfx.monitors[monitor_index] = malloc(sizeof(*gfx.monitors[monitor_index]));
            }
        }
        ASSERT(gfx.monitors_top < gfx.monitors_size);
        monitor_t monitor = gfx.monitors[gfx.monitors_top++];
        memset(monitor, 0, sizeof(*monitor));
        monitor->glfw_monitor = glfw_monitor;
    } else if (GLFW_DISCONNECTED) {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "monitor %s has disconnected", glfwGetMonitorName(glfw_monitor));

        bool found_monitor = false;
        for (uint32_t monitor_index = 0; monitor_index < gfx.monitors_top; ++monitor_index) {
            if (gfx.monitors[monitor_index]->glfw_monitor == glfw_monitor) {
                found_monitor = true;
                while (monitor_index < gfx.monitors_top - 1) {
                    gfx.monitors[monitor_index] = gfx.monitors[monitor_index + 1];
                    ++monitor_index;
                }
                break ;
            }
        }

        ASSERT(found_monitor);

        --gfx.monitors_top;

        // todo: realloc gfx.monitors to save space?
    }
}

static void gfx__controller_callback(int jid, int event) {
    ASSERT(jid >= 0 && (uint32_t) jid < ARRAY_SIZE(gfx.controller));
    controller_t* controller = &gfx.controller[jid];
    const bool exists_gamepad_mapping = glfwJoystickIsGamepad(jid);

    if (event == GLFW_CONNECTED && exists_gamepad_mapping) {
        controller->name = glfwGetJoystickName(jid);
        controller__set_connected(controller, true);
    } else if (event == GLFW_DISCONNECTED) {
        controller__set_connected(controller, false);
    } else {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_WARN, "joystick event? %d\n", event);
    }
}

static void window__should_close_callback(GLFWwindow* glfw_window) {
    window_t window = window__from_glfw_window(glfw_window);
    window__set_should_close(window, true);
    debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: closed by user", window->title);
}

static void window__pos_changed_callback(GLFWwindow* glfw_window, int x, int y) {
    window_t window = window__from_glfw_window(glfw_window);

    debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: content area's position changed to: %d %d", window->title, x, y);
}

static void window__size_changed_callback(GLFWwindow* glfw_window, int width, int height) {
    window_t window = window__from_glfw_window(glfw_window);

    debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: content area's size changed to: %u %u", window->title, width, height);
}

static window_t window__from_glfw_window(GLFWwindow* glfw_window) {
    window_t window = (window_t) glfwGetWindowUserPointer(glfw_window);
    ASSERT(window);

    return window;
}

static void window__framebuffer_resize_callback(GLFWwindow* glfw_window, int width, int height) {
    window_t window = window__from_glfw_window(glfw_window);

    debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: framebuffer dimensions changed to: %dpx %dpx", window->title, width, height);
    // note: width and height could be greater or smaller than the (content area converted to pixels)
    //  for example they could be smaller to display other elements outside of the opengl viewport
    window__set_viewport(window, 0, 0, width, height);
}

static void window__content_scale_callback(GLFWwindow* glfw_window, float xscale, float yscale) {
    window_t window = window__from_glfw_window(glfw_window);

    debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: content scale: %f %f", window->title, xscale, yscale);
}

static void window__minimized_callback(GLFWwindow* glfw_window, int minimized) {
    window_t window = window__from_glfw_window(glfw_window);

    if (minimized) {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: is minimized", window->title);
    } else {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: is restored", window->title);
    }
}

static void window__maximized_callback(GLFWwindow* glfw_window, int maximized) {
    window_t window = window__from_glfw_window(glfw_window);

    if (maximized) {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: is maximized", window->title);
    } else {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: is restored", window->title);
    }
}

static void window__focus_callback(GLFWwindow* glfw_window, int focused) {
    window_t window = window__from_glfw_window(glfw_window);
    
    if (focused) {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: has gained input focus", window->title);
    } else {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: has lost input focus", window->title);
    }
}

static void window__key_callback(GLFWwindow* glfw_window, int key, int platform_specific_scancode, int action, int mods) {
    (void) platform_specific_scancode;

    window_t window = window__from_glfw_window(glfw_window);
    button_t button = _BUTTON_SIZE;

    const bool is_pressed           = action == GLFW_PRESS || action == GLFW_REPEAT;
    const bool is_shift_down        = mods & GLFW_MOD_SHIFT;
    const bool is_ctrl_down         = mods & GLFW_MOD_CONTROL;
    const bool is_alt_down          = mods & GLFW_MOD_ALT;
    const bool is_super_down        = mods & GLFW_MOD_SUPER;
    const bool is_caps_lock_enabled = mods & GLFW_MOD_CAPS_LOCK;
    const bool is_num_lock_enabled  = mods & GLFW_MOD_NUM_LOCK;
    (void) is_shift_down;
    (void) is_ctrl_down;
    (void) is_super_down;
    (void) is_caps_lock_enabled;
    (void) is_num_lock_enabled;

    switch (key) {
    case GLFW_KEY_0: button = BUTTON_0; break ; case GLFW_KEY_1: button = BUTTON_1; break ; case GLFW_KEY_2: button = BUTTON_2; break ; case GLFW_KEY_3: button = BUTTON_3; break ; case GLFW_KEY_4: button = BUTTON_4; break ; case GLFW_KEY_5: button = BUTTON_5; break ; case GLFW_KEY_6: button = BUTTON_6; break ; case GLFW_KEY_7: button = BUTTON_7; break ; case GLFW_KEY_8: button = BUTTON_8; break ; case GLFW_KEY_9: button = BUTTON_9; break ;
    case GLFW_KEY_A: button = BUTTON_A; break ; case GLFW_KEY_B: button = BUTTON_B; break ;
    case GLFW_KEY_C: {
        if (is_ctrl_down) {
            button = BUTTON_SET_CLIPBOARD;
        } else {
            button = BUTTON_C;
        }
    } break ;
    case GLFW_KEY_D: button = BUTTON_D; break ; case GLFW_KEY_E: button = BUTTON_E; break ; case GLFW_KEY_F: button = BUTTON_F; break ; case GLFW_KEY_G: button = BUTTON_G; break ; case GLFW_KEY_H: button = BUTTON_H; break ; case GLFW_KEY_I: button = BUTTON_I; break ; case GLFW_KEY_J: button = BUTTON_J; break ; case GLFW_KEY_K: button = BUTTON_K; break ; case GLFW_KEY_L: button = BUTTON_L; break ; case GLFW_KEY_M: button = BUTTON_M; break ;
    case GLFW_KEY_N: button = BUTTON_N; break ; case GLFW_KEY_O: button = BUTTON_O; break ; case GLFW_KEY_P: button = BUTTON_P; break ; case GLFW_KEY_Q: button = BUTTON_Q; break ; case GLFW_KEY_R: button = BUTTON_R; break ; case GLFW_KEY_S: button = BUTTON_S; break ; case GLFW_KEY_T: button = BUTTON_T; break ; case GLFW_KEY_U: button = BUTTON_U; break ;
    case GLFW_KEY_V: {
        if (is_ctrl_down) {
            button = BUTTON_GET_CLIPBOARD;
        } else {
            button = BUTTON_V;
        }
    } break ;
    case GLFW_KEY_W: button = BUTTON_W; break ; case GLFW_KEY_X: button = BUTTON_X; break ; case GLFW_KEY_Y: button = BUTTON_Y; break ; case GLFW_KEY_Z: button = BUTTON_Z; break ;
    case GLFW_KEY_LEFT: button = BUTTON_LEFT; break ;
    case GLFW_KEY_UP: {
        if (is_alt_down) {
            button = BUTTON_WINDOW_MAXIMIZE;
        } else {
            button = BUTTON_UP;
        }
    } break ;
    case GLFW_KEY_RIGHT: button = BUTTON_RIGHT; break ;
    case GLFW_KEY_DOWN: {
        if (is_alt_down) {
            button = BUTTON_WINDOW_WINDOWED;
        } else {
            button = BUTTON_DOWN;
        }
    } break ;
    case GLFW_KEY_CAPS_LOCK: button = BUTTON_CAPS_LOCK; break ; case GLFW_KEY_LEFT_SHIFT: button = BUTTON_SHIFT; break ; case GLFW_KEY_RIGHT_SHIFT: button = BUTTON_SHIFT; break ;
    case GLFW_KEY_SPACE: button = BUTTON_SPACE; break ;
    case GLFW_KEY_BACKSPACE: {
        if (is_alt_down) {
            button = BUTTON_WINDOW_MINIMIZE;
        } else {
            button = BUTTON_BACKSPACE;
        }
    } break ;
    case GLFW_KEY_ESCAPE: {
        button = BUTTON_WINDOW_CLOSE;
    } break ;
    case GLFW_KEY_F4: {
        if (is_alt_down) {
            button = BUTTON_WINDOW_CLOSE;
        }
    } break ;
    case GLFW_KEY_ENTER: {
        if (is_alt_down) {
            button = BUTTON_WINDOW_FULL_SCREEN;
        } else {
            button = BUTTON_ENTER;
        }
    } break ;
    case GLFW_KEY_EQUAL: {
        if (is_alt_down) {
            button = BUTTON_FPS_LOCK_INC;
        }
    } break ;
    case GLFW_KEY_MINUS: {
        if (is_alt_down) {
            button = BUTTON_FPS_LOCK_DEC;
        }
    } break ;
    }

    if (button == BUTTON_I && is_alt_down) {
        button = BUTTON_DEBUG_INFO_MESSAGE_TOGGLE;
    }

    if (button == _BUTTON_SIZE) {
        return ;
    }

    window__button_process_input(window, button, is_pressed);
}

static void window__utf32_callback(GLFWwindow* window, uint32_t utf32) {
    (void) window;
    (void) utf32;
}

static void window__add_default_button_actions(window_t self) {
    window__button_register_action(self, BUTTON_WINDOW_CLOSE, (void*) self, &window__button_default_action_window_close);
    window__button_register_action(self, BUTTON_WINDOW_MINIMIZE, (void*) self, &window__button_default_action_window_minimize);
    window__button_register_action(self, BUTTON_WINDOW_MAXIMIZE, (void*) self, &window__button_default_action_window_maximize);
    window__button_register_action(self, BUTTON_WINDOW_WINDOWED, (void*) self, &window__button_default_action_window_windowed);
    window__button_register_action(self, BUTTON_WINDOW_FULL_SCREEN, (void*) self, &window__button_default_action_window_full_screen);
    window__button_register_action(self, BUTTON_DEBUG_INFO_MESSAGE_TOGGLE, (void*) self, &window__button_default_action_debug_info_message_toggle);
    window__button_register_action(self, BUTTON_GET_CLIPBOARD, (void*) self, &window__button_default_action_get_clipboard);
    window__button_register_action(self, BUTTON_SET_CLIPBOARD, (void*) self, &window__button_default_action_set_clipboard);
}

static void window__button_default_action_window_close(void* user_pointer) {
    window_t window = (window_t) user_pointer;

    window__set_should_close(window, true);
}

static void window__button_default_action_window_minimize(void* user_pointer) {
    window_t window = (window_t) user_pointer;

    if (window__get_fullscreen(window)) {
        window__set_display_state(window, WINDOW_DISPLAY_STATE_MINIMIZED_FULLSCREEN);
    } else {
        window__set_display_state(window, WINDOW_DISPLAY_STATE_WINDOWED_MINIMIZED);
    }
}

static void window__button_default_action_window_maximize(void* user_pointer) {
    window_t window = (window_t) user_pointer;

    window__set_display_state(window, WINDOW_DISPLAY_STATE_WINDOWED_MAXIMIZED);
}

static void window__button_default_action_window_windowed(void* user_pointer) {
    window_t window = (window_t) user_pointer;

    window__set_display_state(window, WINDOW_DISPLAY_STATE_WINDOWED);
}

static void window__button_default_action_window_full_screen(void* user_pointer) {
    window_t window = (window_t) user_pointer;

    window__set_display_state(window, WINDOW_DISPLAY_STATE_FULLSCREEN);
}

static void window__button_default_action_debug_info_message_toggle(void* user_pointer) {
    window_t window = (window_t) user_pointer;
    (void) window;

    debug__set_message_type_availability(DEBUG_INFO, !debug__get_message_type_availability(DEBUG_INFO));
}

static void window__button_default_action_get_clipboard(void* user_pointer) {
    window_t window = (window_t) user_pointer;

    window->clipboard = window__get_clipboard(window);

    debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "paste from clipboard: %s", window->clipboard);
}

static void window__button_default_action_set_clipboard(void* user_pointer) {
    window_t window = (window_t) user_pointer;

    window->clipboard = "whaaat";
    window__set_clipboard(window, window->clipboard);

    debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "copied to clipboard: %s", window->clipboard);
}

static void window__cursor_pos_callback(GLFWwindow* glfw_window, double x, double y) {
    window_t window = window__from_glfw_window(glfw_window);
    (void) window;
    (void) x;
    (void) y;
}

static void window__button_process_input(window_t self, button_t button, bool is_pressed) {
    debug__write("window %s: processing button", self->title);
    controller__button_process_input(&self->controller, button, is_pressed);

    button_state_t* button_state = &self->controller.buttons[button];
    if (is_pressed && button_state->action_on_button_down) {
        button_state->action_on_button_down(button_state->user_pointer);
    }
}

static void window__cursor_enter_callback(GLFWwindow* glfw_window, int entered) {
    window_t window = window__from_glfw_window(glfw_window);

    if (entered) {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: cursor has entered the content area", window->title);
    } else {
        debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "window %s: cursor has left the content area", window->title);
    }
}

static void window__cursor_button_callback(GLFWwindow* glfw_window, int key, int action, int mods) {
    window_t window = window__from_glfw_window(glfw_window);

    const bool is_pressed           = action == GLFW_PRESS;
    const bool is_shift_down        = mods & GLFW_MOD_SHIFT;
    const bool is_ctrl_down         = mods & GLFW_MOD_CONTROL;
    const bool is_alt_down          = mods & GLFW_MOD_ALT;
    const bool is_super_down        = mods & GLFW_MOD_SUPER;
    const bool is_caps_lock_enabled = mods & GLFW_MOD_CAPS_LOCK;
    const bool is_num_lock_enabled  = mods & GLFW_MOD_NUM_LOCK;
    (void) is_shift_down;
    (void) is_ctrl_down;
    (void) is_alt_down;
    (void) is_super_down;
    (void) is_caps_lock_enabled;
    (void) is_num_lock_enabled;

    button_t button = _BUTTON_SIZE;

    switch (key) {
    case GLFW_MOUSE_BUTTON_LEFT: button = BUTTON_CURSOR_LEFT; break ;
    case GLFW_MOUSE_BUTTON_RIGHT: button = BUTTON_CURSOR_RIGHT; break ;
    case GLFW_MOUSE_BUTTON_MIDDLE: button = BUTTON_CURSOR_MIDDLE; break ;
    }

    if (button == _BUTTON_SIZE) {
        return ;
    }

    window__button_process_input(window, button, is_pressed);
}

static void window__cursor_scroll_callback(GLFWwindow* glfw_window, double xoffset, double yoffset) {
    (void) glfw_window;

    debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "cursor scroll: %.3lf %.3lf", xoffset, yoffset);
}

static void window__drop_callback(GLFWwindow* glfw_window, int paths_size, const char** paths) {
    (void) glfw_window;

    debug__write("files dropped:");
    for (uint32_t path_index = 0; path_index < (uint32_t) paths_size; ++path_index) {
        debug__write("  %s", paths[path_index]);
    }
    debug__flush(DEBUG_MODULE_GLFW, DEBUG_INFO);
}

static void controller__button_process_input(controller_t* self, button_t button, bool is_pressed) {
    button_state_t* button_state = &self->buttons[button];

    const bool was_down = button_state->ended_down;
    button_state->ended_down = is_pressed;
    if (
        (is_pressed && !was_down) ||
        (!is_pressed && was_down)
    ) {
        self->received_button_input = true;
        ++button_state->n_of_transitions;
        debug__write(
            "button transition [%s]: %s -> %s",
            button__to_str(button),
            was_down ? "pressed" : "released",
            is_pressed ? "pressed" : "released"
        );
    }
    if (is_pressed) {
        ++button_state->n_of_repeats;
        debug__write("button repeats %u", button_state->n_of_repeats);
    }
    debug__flush(DEBUG_MODULE_GLFW, DEBUG_INFO);
}

static void controller__button_process_axes(controller_t* self, button_t button, uint32_t axes_index, float value) {
    button_state_t* button_state = &self->buttons[button];

    ASSERT(axes_index < ARRAY_SIZE(self->axes));
    if (self->axes[axes_index] != value) {
        self->received_button_input = true;
        ++button_state->n_of_transitions;
        debug__write_and_flush(
            DEBUG_MODULE_GLFW, DEBUG_INFO,
            "axes transition [%s]: %f -> %f",
            button__to_str(button),
            self->axes[axes_index],
            value
        );
        self->axes[axes_index] = value;
    }
}

static void controller__clear(controller_t* self) {
    for (uint32_t button_index = 0; button_index < ARRAY_SIZE(self->buttons); ++button_index) {
        self->buttons[button_index].n_of_repeats     = 0;
        self->buttons[button_index].n_of_transitions = 0;
    }
    self->received_button_input = false;
}

static void controller__set_connected(controller_t* self, bool value) {
    if (value) {
        if (!controller__get_connected(self)) {
            debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "controller [%s] is connected", self->name);
            memset(self->buttons, 0, sizeof(self->buttons));
            self->received_button_input = false;
        }
    } else {
        if (controller__get_connected(self)) {
            debug__write_and_flush(DEBUG_MODULE_GLFW, DEBUG_INFO, "controller [%s] is disconnected", self->name);
        }
    }
    self->is_connected = value;
}

static bool controller__get_connected(controller_t* self) {
    return self->is_connected;
}

static void window__set_fullscreen(window_t self, bool value)  {
    if (value) {
        const GLFWvidmode* mode = glfwGetVideoMode(self->monitor->glfw_monitor);
        glfwSetWindowMonitor(
            self->glfw_window,
            self->monitor->glfw_monitor,
            0, 0,
            mode->width, mode->height,
            mode->refreshRate
        );
    } else {
        glfwSetWindowMonitor(
            self->glfw_window,
            NULL,
            self->content_area_x, self->content_area_y,
            self->content_area_w, self->content_area_h,
            GLFW_DONT_CARE
        );
    }
}

static bool window__get_fullscreen(window_t self) {
    return glfwGetWindowMonitor(self->glfw_window) != 0;
}

static void window__set_minimized(window_t self, bool value) {
    if (value) {
        glfwIconifyWindow(self->glfw_window);
    } else {
        glfwRestoreWindow(self->glfw_window);
    }
}

static bool window__get_minimized(window_t self) {
    return glfwGetWindowAttrib(self->glfw_window, GLFW_ICONIFIED);
}

static void window__set_maximized(window_t self, bool value) {
    if (value) {
        glfwMaximizeWindow(self->glfw_window);
    } else {
        glfwRestoreWindow(self->glfw_window);
    }
}

static bool window__get_maximized(window_t self) {
    return glfwGetWindowAttrib(self->glfw_window, GLFW_MAXIMIZED);
}

static void window__restore_content_area(window_t self) {
    glfwSetWindowPos(self->glfw_window, self->content_area_x, self->content_area_y);
    glfwSetWindowSize(self->glfw_window, (int32_t) self->content_area_w, (int32_t) self->content_area_h);
}

static const char* button__to_str(button_t button) {
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
