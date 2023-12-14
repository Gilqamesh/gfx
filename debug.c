#include "debug.h"

#include "buffer.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

#include "debug_internal.c"

bool debug__init_module() {
    memset(&debug, 0, sizeof(debug));

    for (uint32_t error_level_availability_index = 0; error_level_availability_index < _DEBUG_MESSAGE_TYPE_SIZE; ++error_level_availability_index) {
        debug.error_level_availability[error_level_availability_index] = true;
    }

    for (uint32_t error_module_availability_index = 0; error_module_availability_index < _DEBUG_MODULE_SIZE; ++error_module_availability_index) {
        debug.error_module_availability[error_module_availability_index] = true;
    }

    debug.error_file = fopen("debug.txt", "w");
    if (!debug.error_file) {
        return false;
    }

    buffer__create(&debug.buffer, 4096);

    return true;
}

void debug__deinit_module() {
    if (debug.error_file) {
        fclose(debug.error_file);
        debug.error_file = 0;
    }
    buffer__destroy(&debug.buffer);
}

void debug__write(const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    debug__vwrite(format, ap);

    va_end(ap);
}

void debug__write_and_flush(debug_module_t module, debug_message_type_t message_type, const char* format, ...) {
    if (!debug__get_message_type_availability(message_type)) {
        debug__clear();
        return ;
    }

    va_list ap;
    va_start(ap, format);

    debug__vwrite(format, ap);

    va_end(ap);

    debug__flush(module, message_type);
}

void debug__flush(debug_module_t module, debug_message_type_t message_type) {
    if (
        !debug__get_message_module_availability(module) ||
        !debug__get_message_type_availability(message_type)
    ) {
        debug__clear();
        return ;
    }

    debug__flush_helper(debug.error_file, module, message_type);
    if (debug.error_file != stderr) {
        debug__flush_helper(stderr, module, message_type);
    }
    
    debug__clear();
}

void debug__clear() {
    buffer__clear(&debug.buffer);
    debug.number_of_lines = 0;
}

void debug__set_message_type_availability(debug_message_type_t message_type, bool value) {
    ASSERT(message_type < _DEBUG_MESSAGE_TYPE_SIZE);
    debug.error_level_availability[message_type] = value;
}

bool debug__get_message_type_availability(debug_message_type_t message_type) {
    ASSERT(message_type < _DEBUG_MESSAGE_TYPE_SIZE);
    return debug.error_level_availability[message_type];
}

void debug__set_message_module_availability(debug_module_t module, bool value) {
    ASSERT(module < _DEBUG_MODULE_SIZE);
    debug.error_module_availability[module] = value;
}

bool debug__get_message_module_availability(debug_module_t module) {
    ASSERT(module < _DEBUG_MODULE_SIZE);
    return debug.error_module_availability[module];
}
