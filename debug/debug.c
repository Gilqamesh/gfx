#include "debug.h"

#include "str_builder.h"
#include "helper_macros.h"

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "debug_internal.c"

bool debug__init_module() {
    memset(&debug, 0, sizeof(debug));

    str_builder__create(&debug.str_builder);

    for (uint32_t module_index = 0; module_index < ARRAY_SIZE(debug.modules); ++module_index) {
        module_t* module = &debug.modules[module_index];
        module->available = true;
        for (uint32_t level_available_index = 0; level_available_index < ARRAY_SIZE(module->level_available); ++level_available_index) {
            module->level_available[level_available_index] = true;
        }
    }

    debug.error_file = fopen("debug/debug.txt", "w");
    if (!debug.error_file) {
        return false;
    }

    debug.write_mutex = mutex__create();
    if (!debug.write_mutex) {
        return false;
    }

    return true;
}

void debug__deinit_module() {
    if (debug.error_file) {
        fclose(debug.error_file);
        debug.error_file = 0;
    }

    if (debug.write_mutex) {
        mutex__destroy(debug.write_mutex);
    }

    str_builder__destroy(&debug.str_builder);
}

void debug__lock() {
    mutex__lock(debug.write_mutex);
}

void debug__unlock() {
    mutex__unlock(debug.write_mutex);
}

void debug__write_raw(const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    str_builder__vfappend(&debug.str_builder, format, ap);

    va_end(ap);
}

void debug__writeln(const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    debug__vwriteln(format, ap);

    va_end(ap);
}

void debug__write_and_flush(debug_module_t module, debug_message_type_t message_type, const char* format, ...) {
    debug__lock();

    if (
        !debug__get_message_module_availability(module) ||
        !debug__get_message_type_availability(module, message_type)
    ) {
        debug__clear();
        debug__unlock();
        return ;
    }

    va_list ap;
    va_start(ap, format);

    debug__vwriteln(format, ap);

    va_end(ap);

    debug__flush(module, message_type);

    debug__unlock();
}

void debug__flush(debug_module_t module, debug_message_type_t message_type) {
    if (
        !debug__get_message_module_availability(module) ||
        !debug__get_message_type_availability(module, message_type)
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

void debug__set_message_type_availability(debug_module_t module, debug_message_type_t message_type, bool value) {
    ASSERT(module <= _DEBUG_MODULE_SIZE);
    ASSERT(message_type < _DEBUG_MESSAGE_TYPE_SIZE);
    if (module == _DEBUG_MODULE_SIZE) {
        for (uint32_t module_index = 0; module_index < ARRAY_SIZE(debug.modules); ++module_index) {
            debug.modules[module_index].level_available[message_type] = value;
        }
    } else {
        debug.modules[module].level_available[message_type] = value;
    }
}

bool debug__get_message_type_availability(debug_module_t module, debug_message_type_t message_type) {
    ASSERT(module < _DEBUG_MODULE_SIZE);
    ASSERT(message_type < _DEBUG_MESSAGE_TYPE_SIZE);
    return debug.modules[module].level_available[message_type];
}

void debug__set_message_module_availability(debug_module_t module, bool value) {
    ASSERT(module < _DEBUG_MODULE_SIZE);
    debug.modules[module].available = value;
}

bool debug__get_message_module_availability(debug_module_t module) {
    ASSERT(module < _DEBUG_MODULE_SIZE);
    return debug.modules[module].available;
}
