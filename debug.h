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

typedef enum debug_message_type {
    DEBUG_ERROR,
    DEBUG_WARN,
    DEBUG_INFO,

    _DEBUG_MESSAGE_TYPE_SIZE
} debug_message_type_t;
const char* debug_message_type__to_str(debug_message_type_t message_type);

typedef enum debug_module {
    DEBUG_MODULE_APP,
    DEBUG_MODULE_GLFW,

    _DEBUG_MODULE_SIZE
} debug_module_t;
const char* debug_module__to_str(debug_module_t module);

void debug__write(const char* format, ...);
void debug__write_and_flush(debug_module_t module, debug_message_type_t message_type, const char* format, ...);
void debug__flush(debug_module_t module, debug_message_type_t message_type);

void debug__set_message_type_availability(debug_message_type_t message_type, bool value);
bool debug__get_message_type_availability(debug_message_type_t message_type);

#endif // DEBUG_H
