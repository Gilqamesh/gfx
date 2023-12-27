#ifndef DEBUG_H
# define DEBUG_H

# include <stdbool.h>
# include <stdarg.h>
# include <assert.h>
# include <stdint.h>

// todo: turn some of the message types that are resource-intensitve into compile-time api

# define ASSERT(expr) do { \
    if (!(expr)) { \
        assert(false); \
    } \
} while (false)

bool debug__init_module();
void debug__deinit_module();

enum         debug_message_type;
enum         debug_module;
typedef enum debug_message_type debug_message_type_t;
typedef enum debug_module       debug_module_t;

enum debug_message_type {
    DEBUG_ERROR,
    DEBUG_WARN,
    DEBUG_INFO,
    DEBUG_NET,

    _DEBUG_MESSAGE_TYPE_SIZE
};

enum debug_module {
    DEBUG_MODULE_APP,
    DEBUG_MODULE_GLFW,
    DEBUG_MODULE_GL,
    DEBUG_MODULE_VULKAN,
    DEBUG_MODULE_GAME,
    DEBUG_MODULE_GAME_SERVER,
    DEBUG_MODULE_GAME_CLIENT,

    _DEBUG_MODULE_SIZE
};

void debug__write_raw(const char* format, ...);
void debug__writeln(const char* format, ...);
void debug__write_and_flush(debug_module_t module, debug_message_type_t message_type, const char* format, ...);
void debug__flush(debug_module_t module, debug_message_type_t message_type);
void debug__clear();

void debug__set_message_type_availability(debug_message_type_t message_type, bool value);
bool debug__get_message_type_availability(debug_message_type_t message_type);

void debug__set_message_module_availability(debug_module_t module, bool value);
bool debug__get_message_module_availability(debug_module_t module);

#endif // DEBUG_H
