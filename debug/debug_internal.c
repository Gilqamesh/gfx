typedef struct debug {
    char*    buffer_cur;
    char*    buffer_end;
    char*    buffer_start;

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
    case DEBUG_MODULE_APP:         return "app";
    case DEBUG_MODULE_GLFW:        return "glfw";
    case DEBUG_MODULE_GL:          return "gl";
    case DEBUG_MODULE_VULKAN:      return "vulkan";
    case DEBUG_MODULE_GAME:        return "game";
    case DEBUG_MODULE_GAME_SERVER: return "game server";
    default: ASSERT(false);
    }

    return 0;
}

static void debug__vwrite(const char* format, va_list ap) {
    const char* line_prefix = "  ";
    const uint32_t line_prefix_len = strlen(line_prefix);

    if (debug.number_of_lines == 1) {
        assert(debug.buffer_cur + line_prefix_len <= debug.buffer_end);
        // prepend first line
        memmove(debug.buffer_start + line_prefix_len, debug.buffer_start, debug.buffer_cur - debug.buffer_start);
        memcpy(debug.buffer_start, line_prefix, line_prefix_len);
        debug.buffer_cur += line_prefix_len;
    }

    if (debug.number_of_lines > 0) {
        // prepend current line
        int32_t bytes_written = snprintf(debug.buffer_cur, debug.buffer_end - debug.buffer_cur, "%s", line_prefix);
        debug.buffer_cur += bytes_written;
    }
    ++debug.number_of_lines;

    int32_t bytes_written = vsnprintf(debug.buffer_cur, debug.buffer_end - debug.buffer_cur, format, ap);
    ASSERT(bytes_written >= 0);
    debug.buffer_cur += bytes_written;

    if (debug.buffer_cur != debug.buffer_end) {
        *debug.buffer_cur = '\n';
        ++debug.buffer_cur;
    }
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

    fprintf(fp, "%s", debug.buffer_start);
}
