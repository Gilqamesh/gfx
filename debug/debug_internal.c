typedef struct debug {
    str_builder_t str_builder;

    uint32_t number_of_lines;
    bool     error_level_availability[_DEBUG_MESSAGE_TYPE_SIZE];
    bool     error_module_availability[_DEBUG_MODULE_SIZE];
    FILE*    error_file;
} debug_t;

static debug_t debug;

static const char* debug_message_type__to_str(debug_message_type_t message_type);
static const char* debug_module__to_str(debug_module_t module);
static void debug__vwriteln(const char* format, va_list ap);
static void debug__flush_helper(FILE* fp, debug_module_t module, debug_message_type_t message_type);

static const char* debug_message_type__to_str(debug_message_type_t message_type) {
    switch (message_type) {
    case DEBUG_ERROR: return "error";
    case DEBUG_WARN:  return "warn";
    case DEBUG_INFO:  return "info";
    case DEBUG_NET:   return "net";
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
    case DEBUG_MODULE_GAME_CLIENT: return "game client";
    default: ASSERT(false);
    }

    return 0;
}

static void debug__vwriteln(const char* format, va_list ap) {
    const char* line_prefix = "  ";
    if (debug.number_of_lines == 1) {
        str_builder__fprepend(&debug.str_builder, "%s", line_prefix);
    }

    if (debug.number_of_lines > 0) {
        str_builder__fappend(&debug.str_builder, "%s", line_prefix);
    }

    str_builder__vfappend(&debug.str_builder, format, ap);
    str_builder__fappend(&debug.str_builder, "\n");

    ++debug.number_of_lines;
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

    fprintf(fp, "%s", str_builder__str(&debug.str_builder));
}
