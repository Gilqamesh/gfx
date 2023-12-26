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

    const uint32_t str_builder_memory_size = KILOBYTES(4);
    void* str_builder_memory = malloc(str_builder_memory_size);
    str_builder__create(&debug.str_builder, str_builder_memory, str_builder_memory_size);

    for (uint32_t error_level_availability_index = 0; error_level_availability_index < _DEBUG_MESSAGE_TYPE_SIZE; ++error_level_availability_index) {
        debug.error_level_availability[error_level_availability_index] = true;
    }

    for (uint32_t error_module_availability_index = 0; error_module_availability_index < _DEBUG_MODULE_SIZE; ++error_module_availability_index) {
        debug.error_module_availability[error_module_availability_index] = true;
    }

    debug.error_file = fopen("debug/debug.txt", "w");
    if (!debug.error_file) {
        return false;
    }

    return true;
}

void debug__deinit_module() {
    if (debug.error_file) {
        fclose(debug.error_file);
        debug.error_file = 0;
    }

    if (debug.str_builder.start) {
        free(debug.str_builder.start);
    }
}

void debug__write_packet_raw(packet_t* packet) {
    debug__write_raw("%-10u%-10u", packet->sequence_id, packet->ack);
    for (int32_t bit_index = (sizeof(packet->ack_bitfield) << 3) - 1; bit_index >= 0; --bit_index) {
        uint32_t bit = (packet->ack_bitfield & (1 << bit_index)) >> bit_index;
        debug__write_raw("%c", bit ? '1' : '0');
        if (bit_index > 0 && bit_index % 8 == 0) {
            debug__write_raw(" ");
        }
    }
    debug__write_raw("\n");
}

void debug__write_raw(const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    str_builder__vappend(&debug.str_builder, format, ap);

    va_end(ap);
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
    str_builder__clear(&debug.str_builder);
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
