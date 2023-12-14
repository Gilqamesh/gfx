static const gl_buffer_t* default_frame_buffer = 0;

static uint32_t gl_buffer_type__to_gl(gl_buffer_type_t type);
static uint32_t gl_buffer_usage_type__to_gl(gl_buffer_usage_type_t usage_type, gl_bufer_freq_type_t freq_type);
static void APIENTRY gl__error_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param);
static const char* gl_error_message_source__to_type_str(GLenum source);
static const char* gl_error_message_source__to_str(GLenum source);
static const char* gl_error_message_type__to_type_str(GLenum type);
static const char* gl_error_message_type__to_str(GLenum type);
static const char* gl_error_message_severity__to_type_str(GLenum severity);
static const char* gl_error_message_severity__to_str(GLenum severity);
static GLenum gl_object_label__from_buffer_type(gl_buffer_type_t buffer_type);

static uint32_t gl_buffer_type__to_gl(gl_buffer_type_t type) {
    switch (type) {
    case GL_BUFFER_TYPE_VERTEX:             return GL_ARRAY_BUFFER;
    case GL_BUFFER_TYPE_INDEX:              return GL_ELEMENT_ARRAY_BUFFER;
    case GL_BUFFER_TYPE_TEXTURE:            return GL_TEXTURE_BUFFER;
    case GL_BUFFER_TYPE_TRANSFORM_FEEDBACK: return GL_TRANSFORM_FEEDBACK_BUFFER;
    case GL_BUFFER_TYPE_FRAMEBUFFER:        return GL_FRAMEBUFFER;
    default: ASSERT(false);
    }

    return 0;
}

static uint32_t gl_buffer_usage_type__to_gl(gl_buffer_usage_type_t usage_type, gl_bufer_freq_type_t freq_type) {
    switch (usage_type) {
    case GL_BUFFER_USAGE_TYPE_WRITE: {
        switch (freq_type) {
        case GL_BUFFER_FREQ_TYPE_STATIC:  return GL_STATIC_DRAW;
        case GL_BUFFER_FREQ_TYPE_DYNAMIC: return GL_DYNAMIC_DRAW;
        case GL_BUFFER_FREQ_TYPE_STREAM:  return GL_STREAM_DRAW;
        default: ASSERT(false);
        }
    } break ;
    case GL_BUFFER_USAGE_TYPE_READ: {
        switch (freq_type) {
        case GL_BUFFER_FREQ_TYPE_STATIC:  return GL_STATIC_READ;
        case GL_BUFFER_FREQ_TYPE_DYNAMIC: return GL_DYNAMIC_READ;
        case GL_BUFFER_FREQ_TYPE_STREAM:  return GL_STREAM_READ;
        default: ASSERT(false);
        }
    } break ;
    case GL_BUFFER_USAGE_TYPE_COPY: {
        switch (freq_type) {
        case GL_BUFFER_FREQ_TYPE_STATIC:  return GL_STATIC_COPY;
        case GL_BUFFER_FREQ_TYPE_DYNAMIC: return GL_DYNAMIC_COPY;
        case GL_BUFFER_FREQ_TYPE_STREAM:  return GL_STREAM_COPY;
        default: ASSERT(false);
        }
    } break ;
    default: ASSERT(false);
    }

    return 0;
}

static void APIENTRY gl__error_message_callback(
    GLenum source, GLenum type, GLuint id, GLenum severity,
    GLsizei length, const GLchar* message,
    const void* user_param
) {
    (void) length;
    (void) user_param;
    (void) id;

    const char* source_type_str          = gl_error_message_source__to_type_str(source);
    const uint32_t source_type_str_len   = strlen(source_type_str);
    const char* source_str               = gl_error_message_source__to_str(source);
    const uint32_t source_str_len        = strlen(source_str);
    const char* type_type_str            = gl_error_message_type__to_type_str(type);
    const uint32_t type_type_str_len     = strlen(type_type_str);
    const char* type_str                 = gl_error_message_type__to_str(type);
    const uint32_t type_str_len          = strlen(type_str);
    const char* severity_type_str        = gl_error_message_severity__to_type_str(severity);
    const uint32_t severity_type_str_len = strlen(severity_type_str);
    const char* severity_str             = gl_error_message_severity__to_str(severity);
    const uint32_t severity_str_len      = strlen(severity_str);
    const uint32_t max_type_len          = MAX(MAX(source_type_str_len, type_type_str_len), severity_type_str_len);
    const uint32_t max_str_len           = MAX(MAX(source_str_len, type_str_len), severity_str_len);

    debug__write("source:   %-*.*s %-*.*s", max_type_len, max_type_len, source_type_str,   max_str_len, max_str_len, source_str);
    debug__write("type:     %-*.*s %-*.*s", max_type_len, max_type_len, type_type_str,     max_str_len, max_str_len, type_str);
    debug__write("severity: %-*.*s %-*.*s", max_type_len, max_type_len, severity_type_str, max_str_len, max_str_len, severity_str);
    debug__write("message:  %s", message);
    debug__flush(DEBUG_MODULE_GL, DEBUG_ERROR);
}

static const char* gl_error_message_source__to_type_str(GLenum source) {
    switch (source) {
    case GL_DEBUG_SOURCE_API:             return "DEBUG_SOURCE_API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "DEBUG_SOURCE_WINDOW_SYSTEM";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "DEBUG_SOURCE_SHADER_COMPILER";
    case GL_DEBUG_SOURCE_THIRD_PARTY:     return "DEBUG_SOURCE_THIRD_PARTY";
    case GL_DEBUG_SOURCE_APPLICATION:     return "DEBUG_SOURCE_APPLICATION";
    case GL_DEBUG_SOURCE_OTHER:           return "DEBUG_SOURCE_OTHER";
    default: ASSERT(false);
    }

    return 0;
}

static const char* gl_error_message_source__to_str(GLenum source) {
    switch (source) {
    case GL_DEBUG_SOURCE_API:             return "Calls to the OpenGL API";
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "Calls to a window-system API";
    case GL_DEBUG_SOURCE_SHADER_COMPILER: return "A compiler for a shading language";
    case GL_DEBUG_SOURCE_THIRD_PARTY:     return "An application associated with OpenGL";
    case GL_DEBUG_SOURCE_APPLICATION:     return "Generated by the user of this application";
    case GL_DEBUG_SOURCE_OTHER:           return "Some source that isn't one of these";
    default: ASSERT(false);
    }

    return 0;
}

static const char* gl_error_message_type__to_type_str(GLenum type) {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:               return "DEBUG_TYPE_ERROR";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEBUG_TYPE_DEPRECATED_BEHAVIOR";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "DEBUG_TYPE_UNDEFINED_BEHAVIOR";
    case GL_DEBUG_TYPE_PORTABILITY:         return "DEBUG_TYPE_PORTABILITY";
    case GL_DEBUG_TYPE_PERFORMANCE:         return "DEBUG_TYPE_PERFORMANCE";
    case GL_DEBUG_TYPE_MARKER:              return "DEBUG_TYPE_MARKER";
    case GL_DEBUG_TYPE_PUSH_GROUP:          return "DEBUG_TYPE_PUSH_GROUP";
    case GL_DEBUG_TYPE_POP_GROUP:           return "DEBUG_TYPE_POP_GROUP";
    case GL_DEBUG_TYPE_OTHER:               return "DEBUG_TYPE_OTHER";
    default: ASSERT(false);
    }

    return 0;
}

static const char* gl_error_message_type__to_str(GLenum type) {
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:               return "An error, typically from the API";
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "Some behavior marked deprecated has been used";
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "Something has invoked undefined behavior";
    case GL_DEBUG_TYPE_PORTABILITY:         return "Some functionality the user relies upon is not portable";
    case GL_DEBUG_TYPE_PERFORMANCE:         return "Code has triggered possible performance issues";
    case GL_DEBUG_TYPE_MARKER:              return "Command stream annotation";
    case GL_DEBUG_TYPE_PUSH_GROUP:          return "Group pushing";
    case GL_DEBUG_TYPE_POP_GROUP:           return "Group popping";
    case GL_DEBUG_TYPE_OTHER:               return "Some type that isn't one of these";
    default: ASSERT(false);
    }

    return 0;
}

static const char* gl_error_message_severity__to_type_str(GLenum severity) {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:         return "DEBUG_SEVERITY_HIGH";
    case GL_DEBUG_SEVERITY_MEDIUM:       return "DEBUG_SEVERITY_MEDIUM";
    case GL_DEBUG_SEVERITY_LOW:          return "DEBUG_SEVERITY_LOW";
    case GL_DEBUG_SEVERITY_NOTIFICATION: return "DEBUG_SEVERITY_NOTIFICATION";
    default: ASSERT(false);
    }

    return 0;
}

static const char* gl_error_message_severity__to_str(GLenum severity) {
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:         return "All OpenGL Errors, shader compilation/linking errors, or highly-dangerous undefined behavior";
    case GL_DEBUG_SEVERITY_MEDIUM:       return "Major performance warnings, shader compilation/linking warnings, or the use of deprecated functionality";
    case GL_DEBUG_SEVERITY_LOW:          return "Redundant state change performance warning, or unimportant undefined behavior";
    case GL_DEBUG_SEVERITY_NOTIFICATION: return "Anything that isn't an error or performance issue.";
    default: ASSERT(false);
    }

    return 0;
}

static GLenum gl_object_label__from_buffer_type(gl_buffer_type_t buffer_type) {
    switch (buffer_type) {
    case GL_BUFFER_TYPE_VERTEX:             return GL_BUFFER;
    case GL_BUFFER_TYPE_INDEX:              return GL_BUFFER;
    case GL_BUFFER_TYPE_TEXTURE:            return GL_TEXTURE;
    case GL_BUFFER_TYPE_TRANSFORM_FEEDBACK: return GL_TRANSFORM_FEEDBACK;
    case GL_BUFFER_TYPE_FRAMEBUFFER:        return GL_FRAMEBUFFER;
    default: ASSERT(false);
    }

    return 0;
}
