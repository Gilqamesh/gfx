typedef struct debug_buffer {
    buffer_t    buffer;
} debug_buffer_t;

typedef struct debug {
    buffer_t buffer;
    uint32_t number_of_lines;
    bool     error_level_availability[_DEBUG_MESSAGE_TYPE_SIZE];
    bool     error_module_availability[_DEBUG_MODULE_SIZE];
    FILE*    error_file;
} debug_t;

static debug_t debug;

static const char* debug_message_type__to_str(debug_message_type_t message_type);
static const char* debug_module__to_str(debug_module_t module);
static void debug__vwrite(const char* format, va_list ap);
static void debug__flush_helper(FILE* fp, debug_module_t module, debug_message_type_t message_type);

static const char* debug_message_type__to_str(debug_message_type_t message_type) {
    switch (message_type) {
    case DEBUG_ERROR: return "error";
    case DEBUG_WARN:  return "warn";
    case DEBUG_INFO:  return "info";
    default: ASSERT(false);
    }

    return 0;
}

static const char* debug_module__to_str(debug_module_t module) {
    switch (module) {
    case DEBUG_MODULE_APP:  return "app";
    case DEBUG_MODULE_GLFW: return "glfw";
    case DEBUG_MODULE_GL:   return "gl";
    default: ASSERT(false);
    }

    return 0;
}

static void debug__vwrite(const char* format, va_list ap) {
    const char* line_prefix = "  ";
    const uint32_t line_prefix_len = strlen(line_prefix);

    if (debug.number_of_lines == 1) {
        assert(debug.buffer.cur + line_prefix_len <= debug.buffer.end);
        // prepend first line
        memmove(debug.buffer.start + line_prefix_len, debug.buffer.start, debug.buffer.cur - debug.buffer.start);
        memcpy(debug.buffer.start, line_prefix, line_prefix_len);
        debug.buffer.cur += line_prefix_len;
    }

    if (debug.number_of_lines > 0) {
        // prepend current line
        buffer__write(&debug.buffer, "%s", line_prefix);
    }
    ++debug.number_of_lines;

    buffer__vwrite(&debug.buffer, format, ap);
    buffer__write(&debug.buffer, "\n");
}

static void debug__flush_helper(FILE* fp, debug_module_t module, debug_message_type_t message_type) {
    time_t cur_time = time(NULL);
    struct tm* cur_localtime = localtime(&cur_time);
    fprintf(
        fp,
        "[%02d:%02d:%02d] [%s] - %s: ",
        cur_localtime->tm_hour, cur_localtime->tm_min, cur_localtime->tm_sec, debug_module__to_str(module), debug_message_type__to_str(message_type)
    );
    if (debug.number_of_lines > 1) {
        fprintf(fp, "\n");
    }

    fprintf(fp, "%s", debug.buffer.start);
}
