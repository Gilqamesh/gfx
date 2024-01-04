#ifndef GFX_H
# define GFX_H

/**
 * Purpose of this module:
 *  - window handle to the operating system
 *  - window input
 *  - window manip
 *  - pixel manip
 *  - rendering manip regardless of the backend (vulkan, opengl, software rendering)
*/

# include <stdbool.h>
# include <stdint.h>

/********************************************************************************
 * Module API
 ********************************************************************************/

bool gfx__init();
void gfx__deinit();

// @brief polls all pending events and inputs, which in turn calls all callbacks associated with every window
void gfx__poll_events();

// @brief blocks thread and waits for at least one incoming event
void gfx__wait_events();

/********************************************************************************
 * Monitor API
 ********************************************************************************/

struct         monitor;
typedef struct monitor* monitor_t;

monitor_t* monitor__get_monitors(uint32_t* number_of_monitors);

void monitor__get_screen_size(monitor_t self, uint32_t* w, uint32_t* h);

// @brief get monitor's workable area that is not occupied by global menu/task bars
void monitor__get_work_area(monitor_t self, int32_t* x, int32_t* y, uint32_t* w, uint32_t* h);

/********************************************************************************
 * Window API
 ********************************************************************************/

/**
 *  _________________________________      
 * |         WINDOW FRAME            |    |
 * |  _____________________________  |    |
 * | |                             | |    |
 * | |          CONTENT            | |    |
 * | |                             | |    |   WINDOW AREA HEIGHT
 * | |           AREA              | |    |
 * | |                             | |    |
 * | |                             | |    |
 * | |_____________________________| |    |
 * |_________________________________|    |
 * 
 * ___________________________________
 *          WINDOW AREA WIDTH
*/

struct         window;
typedef struct window* window_t;

/**
 * @brief Creates a window object without a graphics context, and sets it as the current window in the thread
*/
window_t window__create(monitor_t monitor, const char* title, uint32_t width, uint32_t height);
void window__destroy(window_t self);

void window__set_default_button_actions(window_t self, bool value);

void window__set_monitor(window_t self, monitor_t monitor);

void window__set_title(window_t self, const char* title);
const char* window__get_title(window_t self);

void window__set_current(window_t self);
window_t window__get_current();

void window__set_icon(window_t self, uint8_t* pixels, uint32_t w, uint32_t h);

typedef enum window_display_state {
    WINDOW_DISPLAY_STATE_WINDOWED,
    WINDOW_DISPLAY_STATE_WINDOWED_MINIMIZED,
    WINDOW_DISPLAY_STATE_WINDOWED_MAXIMIZED,
    WINDOW_DISPLAY_STATE_FULLSCREEN,
    WINDOW_DISPLAY_STATE_MINIMIZED_FULLSCREEN
} window_display_state_t;

/**
 * @brief set position and dimensions of the content area for window's WINDOW_DISPLAY_STATE_WINDOWED state
 * @note does no state transitions
*/
void window__set_windowed_state_content_area(window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height);

/**
 * @brief get position and dimensions of the content area for window's WINDOW_DISPLAY_STATE_WINDOWED state
 * @note the window could be in a different display state
*/
void window__get_windowed_state_content_area(window_t self, int32_t* x, int32_t* y, uint32_t* width, uint32_t* height);

/**
 * @brief set position and dimensions of the window area for window's WINDOW_DISPLAY_STATE_WINDOWED state
 * @note does no state transitions
*/
void window__set_windowed_state_window_area(window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height);

/**
 * @brief get position and dimensions of the window area for window's WINDOW_DISPLAY_STATE_WINDOWED state
 * @note the window could be in a different display state
*/
void window__get_windowed_state_window_area(window_t self, int32_t* x, int32_t* y, uint32_t* width, uint32_t* height);

void window__set_display_state(window_t self, window_display_state_t state);
window_display_state_t window__get_display_state(window_t self);

void window__set_hidden(window_t self, bool value);
bool window__get_hidden(window_t self);

void window__set_focus(window_t self);
bool window__get_focus(window_t self);

void window__set_should_close(window_t self, bool value);
bool window__get_should_close(window_t self);

void window__request_attention(window_t self);

// @param opacity [0, 1], where 0 is fully transparent, and 1 is fully opaque
void window__set_window_opacity(window_t self, float opacity);
float window__get_window_opacity(window_t self);

// @brief set content area size limits
// @note supply 0 for any of the parameters if do not care
void window__set_size_limit(
    window_t self,
    uint32_t min_width, uint32_t min_height,
    uint32_t max_width, uint32_t max_height
);

// @note set either to 0 to disable enforcing aspect ratio
void window__set_aspect_ratio(window_t self, uint32_t num, uint32_t den);

/**
 * @returns current content of the system's clipboard if it is convertible to a UTF-8 encoded string
 * @note the lifetime of the string is valid until the next call to window__get_clipboard or window__set_clipboard
*/
const char* window__get_clipboard(window_t self);

/**
 * @brief sets system's clipboard as a UTF-8 encoded string
*/
void window__set_clipboard(window_t self, const char* str);

/********************************************************************************
 * Controller and Virtual button API
 ********************************************************************************/

/**
 * @brief Virtual button, each window has its own button state
 */
enum           button;
struct         controller;
typedef enum   button     button_t;
typedef struct controller controller_t;

enum button {
    BUTTON_0, BUTTON_1, BUTTON_2, BUTTON_3, BUTTON_4, BUTTON_5, BUTTON_6, BUTTON_7, BUTTON_8, BUTTON_9,
    BUTTON_A, BUTTON_B, BUTTON_C, BUTTON_D, BUTTON_E, BUTTON_F, BUTTON_G, BUTTON_H, BUTTON_I, BUTTON_J, BUTTON_K, BUTTON_L, BUTTON_M,
    BUTTON_N, BUTTON_O, BUTTON_P, BUTTON_Q, BUTTON_R, BUTTON_S, BUTTON_T, BUTTON_U, BUTTON_V, BUTTON_W, BUTTON_X, BUTTON_Y, BUTTON_Z,

    BUTTON_LEFT, BUTTON_UP, BUTTON_RIGHT, BUTTON_DOWN,
    BUTTON_CAPS_LOCK, BUTTON_SHIFT,
    BUTTON_SPACE, BUTTON_BACKSPACE,
    BUTTON_ENTER,

    BUTTON_FPS_LOCK_INC /* default: alt + + */,
    BUTTON_FPS_LOCK_DEC /* default: alt + - */,

    BUTTON_WINDOW_CLOSE /* default: esc, alt+f4 */,
    BUTTON_WINDOW_MINIMIZE /* default: alt + backspace */,
    BUTTON_WINDOW_MAXIMIZE /* default: alt + key-up    */,
    BUTTON_WINDOW_WINDOWED /* default: alt + key-down  */,
    BUTTON_WINDOW_FULL_SCREEN /* default: alt + enter  */,

    BUTTON_DEBUG_INFO_MESSAGE_TOGGLE /* default: alt + i */,

    BUTTON_GAMEPAD_A, BUTTON_GAMEPAD_B, BUTTON_GAMEPAD_X, BUTTON_GAMEPAD_Y,
    BUTTON_GAMEPAD_LEFT_BUMPER, BUTTON_GAMEPAD_RIGHT_BUMPER, BUTTON_GAMEPAD_BACK,
    BUTTON_GAMEPAD_START, BUTTON_GAMEPAD_GUIDE, BUTTON_GAMEPAD_LEFT_THUMB, BUTTON_GAMEPAD_RIGHT_THUMB,
    BUTTON_GAMEPAD_DPAD_UP, BUTTON_GAMEPAD_DPAD_RIGHT, BUTTON_GAMEPAD_DPAD_DOWN, BUTTON_GAMEPAD_DPAD_LEFT,

    BUTTON_GAMEPAD_AXIS_LEFT_X, BUTTON_GAMEPAD_AXIS_LEFT_Y, BUTTON_GAMEPAD_AXIS_RIGHT_X, BUTTON_GAMEPAD_AXIS_RIGHT_Y,
    BUTTON_GAMEPAD_AXIS_LEFT_TRIGGER, BUTTON_GAMEPAD_AXIS_RIGHT_TRIGGER,

    BUTTON_CURSOR_LEFT,
    BUTTON_CURSOR_RIGHT,
    BUTTON_CURSOR_MIDDLE,

    BUTTON_GET_CLIPBOARD /* default: ctrl + v */,
    BUTTON_SET_CLIPBOARD /* default: ctrl + c */,

    _BUTTON_SIZE
};

controller_t* window__get_controller(window_t self);

// @brief current state of button
bool controller__button_is_down(controller_t* self, button_t button);
uint32_t controller__button_n_of_repeats(controller_t* self, button_t button);

// @brief number of press/release transitions
uint32_t controller__button_n_of_transitions(controller_t* self, button_t button);

void controller__get_cursor_pos(controller_t* self, double* x, double* y);

void controller__button_register_action(controller_t* self, button_t button, void* user_pointer, void (*action_on_button_down)(void*));

/********************************************************************************
 * Cursor API
 ********************************************************************************/

struct         cursor;
enum           cursor_state;
typedef struct cursor*      cursor_t;
typedef enum   cursor_state cursor_state_t;

enum cursor_state {
    // Hides and locks cursor to the window
    // todo: something about 'raw mouse motion', to take over the system's
    CURSOR_DISABLED,

    // Normal system cursor behavior
    CURSOR_ENABLED,

    // Hides the cursor but does not lock it to the window
    CURSOR_HIDDEN,

    _CURSOR_STATE_SIZE
};

void window__set_cursor_state(window_t self, cursor_state_t state);
cursor_state_t window__get_cursor_state(window_t self);

cursor_t cursor__create(uint8_t* pixels, uint32_t w, uint32_t h);
void cursor__destroy(cursor_t cursor);

// @brief replaces the current cursor for the window
void window__set_cursor(window_t self, cursor_t cursor);
void window__reset_cursor(window_t self);

bool window__cursor_is_in_content_area(window_t self);

/********************************************************************************
 * Render API
 ********************************************************************************/

/**
 * @brief Defines the affine transformation from normalized device coordinates to window coordinates
*/
void window__set_viewport(window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height);

/**
 * @brief Defines the scissor box in window coordinates
*/
void window__set_scissor(window_t self, int32_t x, int32_t y, uint32_t width, uint32_t height);

/**
 * @brief Blocking call that awaits the currently processed framebuffer to be drawn
*/
void window__swap_buffers(window_t self);

#endif // GFX_H
