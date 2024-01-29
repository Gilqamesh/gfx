#ifndef DEBUG_H
# define DEBUG_H

# include <stdbool.h>
# include <stdarg.h>
# include <assert.h>
# include <stdint.h>

# include "thread.h"
# include "helper_macros.h"

// todo: turn some of the message types that are resource-intensitve into compile-time api

# define ASSERT(expr) do { \
    if (!(expr)) { \
        assert(false); \
    } \
} while (false)

PUBLIC_API bool debug__init_module();
PUBLIC_API void debug__deinit_module();

typedef enum debug_message_type {
    DEBUG_ERROR,
    DEBUG_WARN,
    DEBUG_INFO,
    DEBUG_NET,

    _DEBUG_MESSAGE_TYPE_SIZE
} debug_message_type_t;

typedef enum debug_module {
    DEBUG_MODULE_APP,
    DEBUG_MODULE_GFX,
    DEBUG_MODULE_GL,
    DEBUG_MODULE_VULKAN,
    DEBUG_MODULE_GAME,
    DEBUG_MODULE_GAME_SERVER,
    DEBUG_MODULE_GAME_CLIENT,

    _DEBUG_MODULE_SIZE
} debug_module_t;

/**
 * @brief Call before non-atomic operations
*/
PUBLIC_API void debug__lock();
PUBLIC_API void debug__unlock();

//! @note Non-atomic
PUBLIC_API void debug__write_raw(const char* format, ...);
//! @note Non-atomic
PUBLIC_API void debug__writeln(const char* format, ...);
//! @note Atomic
PUBLIC_API void debug__write_and_flush(debug_module_t module, debug_message_type_t message_type, const char* format, ...);
//! @note Non-atomic
PUBLIC_API void debug__flush(debug_module_t module, debug_message_type_t message_type);

//! @note Atomic
//! @param module to disable for all modules, set this to _DEBUG_MODULE_SIZE
PUBLIC_API void debug__set_message_type_availability(debug_module_t module, debug_message_type_t message_type, bool value);
//! @note Atomic
PUBLIC_API bool debug__get_message_type_availability(debug_module_t module, debug_message_type_t message_type);

//! @note Atomic
PUBLIC_API void debug__set_message_module_availability(debug_module_t module, bool value);
//! @note Atomic
PUBLIC_API bool debug__get_message_module_availability(debug_module_t module);

#endif // DEBUG_H
