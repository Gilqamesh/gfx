struct         g_modelformat_header;
struct         g_modelformat_chunk_header;
struct         g_modelformat_chunk_position_header;
struct         g_modelformat_chunk_normal_header;
struct         g_modelformat_chunk_texture_2d_header;
struct         g_modelformat_chunk_index_header;
typedef struct g_modelformat_header                  g_modelformat_header_t;
typedef struct g_modelformat_chunk_header            g_modelformat_chunk_header_t;
typedef struct g_modelformat_chunk_position_header   g_modelformat_chunk_position_header_t;
typedef struct g_modelformat_chunk_normal_header     g_modelformat_chunk_normal_header_t;
typedef struct g_modelformat_chunk_texture_2d_header g_modelformat_chunk_texture_2d_header_t;
typedef struct g_modelformat_chunk_material_header   g_modelformat_chunk_material_header_t;
typedef struct g_modelformat_chunk_index_header      g_modelformat_chunk_index_header_t;

struct g_modelformat_header {
union {
    uint32_t magic;
    char     magic_name[4];
};
uint32_t n_of_chunks;
uint32_t header_size;
};

struct g_modelformat_chunk_header {
uint32_t type;
uint32_t size_including_header;
};

struct g_modelformat_chunk_position_header {
g_modelformat_chunk_header_t chunk_header;
uint32_t n_of_vertices;
// header is followed by 'n_of_vertices' of { r32 x, r32 y, r32 z }
};

struct g_modelformat_chunk_normal_header {
g_modelformat_chunk_header_t chunk_header;
uint32_t n_of_normals;
// header is followed by 'n_of_normals' of { r32 x, r32 y, r32 z }, the resulting vector is normalized
};

struct g_modelformat_chunk_texture_2d_header {
g_modelformat_chunk_header_t chunk_header;
uint32_t n_of_textures;
//! TODO: figure out this association
uint32_t texture_file_index;
// header is followed by 'n_of_textures' of { r32 u, r32 v }, the r32 values are clamped to the range of [0, 1]
};

struct g_modelformat_chunk_material_header {
g_modelformat_chunk_header_t chunk_header;
uint32_t n_of_materials;
};

struct g_modelformat_chunk_index_header {
g_modelformat_chunk_header_t chunk_header;
uint32_t n_of_indices;
// header is followed by 'n_of_indices' of u32 indices
};

// static void gl_buffer__bind(gl_buffer_t* self);
static void gl_buffer__unbind(gl_buffer_t* self);
static uint32_t gl_buffer_type__to_gl(gl_buffer_type_t type);
static uint32_t gl_buffer_access_type__to_gl(gl_buffer_access_type_t access_type);
static void APIENTRY gl__error_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* user_param);
static const char* gl_error_message_source__to_type_str(GLenum source);
static const char* gl_error_message_source__to_str(GLenum source);
static const char* gl_error_message_type__to_type_str(GLenum type);
static const char* gl_error_message_type__to_str(GLenum type);
static const char* gl_error_message_severity__to_type_str(GLenum severity);
static const char* gl_error_message_severity__to_str(GLenum severity);
static uint32_t shader_type__to_gl(shader_type_t shader_type);
static uint32_t shader_type__to_bit(shader_type_t shader_type);
static uint32_t shader_type__to_gl_subroutine(shader_type_t shader_type);
static const char* shader_type__to_str(shader_type_t shader_type);
static void geometry_object__bind(geometry_object_t* self);
static void geometry_object__unbind(geometry_object_t* self);
static uint32_t gl_type__to_gl(gl_type_t type);
static uint32_t gl_channel_count__to_gl(gl_channel_count_t channel_count);
static uint32_t gl_channel_count__to_size(gl_channel_count_t channel_count);
static uint32_t gl_type_and_channel__to_internal_format(gl_type_t type, gl_channel_count_t channel_count);
static GLenum primitive_type__to_gl(primitive_type_t type);
static bool geometry_object__load_from_g_modelformat(geometry_object_t* self, char* buffer, size_t buffer_size);
// static void shader_program__bind(shader_program_t* self);
static uint32_t texture_type__to_gl(texture_type_t type);
static uint32_t texture_wrap_type__to_gl(wrap_type_t type);
static uint32_t texture_filter_stretch_type__to_gl(filter_stretch_type_t stretch_type);
static uint32_t texture_filter_sample_type__to_gl(filter_sample_type_t sample_type);

// static void gl_buffer__bind(gl_buffer_t* self) {
//     if (self->is_bound) {
//         return ;
//     }

//     self->is_bound = true;
//     if (self->target == GL_FRAMEBUFFER) {
//         glBindFramebuffer(GL_FRAMEBUFFER, self->id);
//     } else {
//         glBindBuffer(self->target, self->id);
//     }
// }

static void gl_buffer__unbind(gl_buffer_t* self) {
    if (!self->is_bound) {
        return ;
    }

    self->is_bound = false;
    if (self->target == GL_FRAMEBUFFER) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        glBindBuffer(self->target, 0);
    }
}

static uint32_t gl_buffer_type__to_gl(gl_buffer_type_t type) {
    switch (type) {
    case GL_BUFFER_TYPE_VERTEX:             return GL_ARRAY_BUFFER;
    case GL_BUFFER_TYPE_INDEX:              return GL_ELEMENT_ARRAY_BUFFER;
    case GL_BUFFER_TYPE_TEXTURE:            return GL_TEXTURE_BUFFER;
    case GL_BUFFER_TYPE_UNIFORM:            return GL_UNIFORM_BUFFER;
    case GL_BUFFER_TYPE_TRANSFORM_FEEDBACK: return GL_TRANSFORM_FEEDBACK_BUFFER;
    case GL_BUFFER_TYPE_FRAMEBUFFER:        return GL_FRAMEBUFFER;
    default: ASSERT(false);
    }

    return 0;
}

static uint32_t gl_buffer_access_type__to_gl(gl_buffer_access_type_t access_type) {
    switch (access_type) {
    case GL_BUFFER_ACCESS_TYPE_READ:       return GL_MAP_READ_BIT;
    case GL_BUFFER_ACCESS_TYPE_WRITE:      return GL_MAP_WRITE_BIT;
    case GL_BUFFER_ACCESS_TYPE_WRITE_READ: return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
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

    debug__lock();

    debug__writeln("source:   %-*.*s %-*.*s", max_type_len, max_type_len, source_type_str,   max_str_len, max_str_len, source_str);
    debug__writeln("type:     %-*.*s %-*.*s", max_type_len, max_type_len, type_type_str,     max_str_len, max_str_len, type_str);
    debug__writeln("severity: %-*.*s %-*.*s", max_type_len, max_type_len, severity_type_str, max_str_len, max_str_len, severity_str);
    debug__writeln("message:  %s", message);
    debug__flush(DEBUG_MODULE_GL, DEBUG_ERROR);

    debug__unlock();
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

static uint32_t shader_type__to_gl(shader_type_t shader_type) {
    switch (shader_type) {
    case SHADER_TYPE_VERTEX:       return GL_VERTEX_SHADER;
    case SHADER_TYPE_FRAGMENT:     return GL_FRAGMENT_SHADER;
    case SHADER_TYPE_TESS_CONTROL: return GL_TESS_CONTROL_SHADER;
    case SHADER_TYPE_TESS_EVAL:    return GL_TESS_EVALUATION_SHADER;
    case SHADER_TYPE_GEOMETRY:     return GL_GEOMETRY_SHADER;
    case SHADER_TYPE_COMPUTE:      return GL_COMPUTE_SHADER;
    default: ASSERT(false);
    }

    return 0;
}

static uint32_t shader_type__to_bit(shader_type_t shader_type) {
    switch (shader_type) {
    case SHADER_TYPE_VERTEX:       return GL_VERTEX_SHADER_BIT;
    case SHADER_TYPE_FRAGMENT:     return GL_FRAGMENT_SHADER_BIT;
    case SHADER_TYPE_TESS_CONTROL: return GL_TESS_CONTROL_SHADER_BIT;
    case SHADER_TYPE_TESS_EVAL:    return GL_TESS_EVALUATION_SHADER_BIT;
    case SHADER_TYPE_GEOMETRY:     return GL_GEOMETRY_SHADER_BIT;
    case SHADER_TYPE_COMPUTE:      return GL_COMPUTE_SHADER_BIT;
    default: ASSERT(false);
    }

    return 0;
}

static uint32_t shader_type__to_gl_subroutine(shader_type_t shader_type) {
    switch (shader_type) {
    case SHADER_TYPE_VERTEX:       return GL_VERTEX_SUBROUTINE;
    case SHADER_TYPE_FRAGMENT:     return GL_FRAGMENT_SUBROUTINE;
    case SHADER_TYPE_TESS_CONTROL: return GL_TESS_CONTROL_SUBROUTINE;
    case SHADER_TYPE_TESS_EVAL:    return GL_TESS_EVALUATION_SUBROUTINE;
    case SHADER_TYPE_GEOMETRY:     return GL_GEOMETRY_SUBROUTINE;
    case SHADER_TYPE_COMPUTE:      return GL_COMPUTE_SUBROUTINE;
    default: ASSERT(false);
    }

    return 0;
}

static const char* shader_type__to_str(shader_type_t shader_type) {
    switch (shader_type) {
    case SHADER_TYPE_VERTEX:       return "VERTEX";
    case SHADER_TYPE_FRAGMENT:     return "FRAGMENT";
    case SHADER_TYPE_TESS_CONTROL: return "TESS_CONTROL";
    case SHADER_TYPE_TESS_EVAL:    return "TESS_EVAL";
    case SHADER_TYPE_GEOMETRY:     return "GEOMETRY";
    case SHADER_TYPE_COMPUTE:      return "COMPUTE";
    default: ASSERT(false);
    }

    return 0;
}

static void geometry_object__bind(geometry_object_t* self) {
    glBindVertexArray(self->id);
}

static void geometry_object__unbind(geometry_object_t* self) {
    (void) self;

    glBindVertexArray(0);
}

static uint32_t gl_type__to_gl(gl_type_t type) {
    switch (type) {
    case GL_TYPE_R32:  return GL_FLOAT;
    case GL_TYPE_S8:   return GL_BYTE;
    case GL_TYPE_S16:  return GL_SHORT;
    case GL_TYPE_S32:  return GL_INT;
    case GL_TYPE_U8:   return GL_UNSIGNED_BYTE;
    case GL_TYPE_U16:  return GL_UNSIGNED_SHORT;
    case GL_TYPE_U32:  return GL_UNSIGNED_INT;
    default: ASSERT(false);
    }

    return 0;
}

static uint32_t gl_channel_count__to_gl(gl_channel_count_t channel_count) {
    switch (channel_count) {
    case GL_CHANNEL_COUNT_1: return GL_RED;
    case GL_CHANNEL_COUNT_2: return GL_RG;
    case GL_CHANNEL_COUNT_3: return GL_RGB;
    case GL_CHANNEL_COUNT_4: return GL_RGBA;
    default: ASSERT(false);
    }

    return 0;
}

static uint32_t gl_channel_count__to_size(gl_channel_count_t channel_count) {
    switch (channel_count) {
    case GL_CHANNEL_COUNT_1: return 1;
    case GL_CHANNEL_COUNT_2: return 2;
    case GL_CHANNEL_COUNT_3: return 3;
    case GL_CHANNEL_COUNT_4: return 4;
    default: ASSERT(false);
    }

    return 0;
}

static uint32_t gl_type_and_channel__to_internal_format(gl_type_t type, gl_channel_count_t channel_count) {
    /**
     * @todo None of these are normalized, there are normalized versions like GL_R8 instead of GL_R8UI in case its necessary in the future
    */
    switch (channel_count) {
    case GL_CHANNEL_COUNT_1: {
        switch (type) {
        case GL_TYPE_U8:    return GL_R8UI;
        case GL_TYPE_U16:   return GL_R16UI;
        case GL_TYPE_U32:   return GL_R32UI;
        case GL_TYPE_S8:    return GL_R8I;
        case GL_TYPE_S16:   return GL_R16I;
        case GL_TYPE_S32:   return GL_R32I;
        case GL_TYPE_R32:   return GL_R32F;
        default: ASSERT(false);
        }
    } break ;
    case GL_CHANNEL_COUNT_2: {
        switch (type) {
        case GL_TYPE_U8:    return GL_RG8UI;
        case GL_TYPE_U16:   return GL_RG16UI;
        case GL_TYPE_U32:   return GL_RG32UI;
        case GL_TYPE_S8:    return GL_RG8I;
        case GL_TYPE_S16:   return GL_RG16I;
        case GL_TYPE_S32:   return GL_RG32I;
        case GL_TYPE_R32:   return GL_RG32F;
        default: ASSERT(false);
        }
    } break ;
    case GL_CHANNEL_COUNT_3: {
        switch (type) {
        case GL_TYPE_U8:    return GL_RGB8UI;
        case GL_TYPE_U16:   return GL_RGB16UI;
        case GL_TYPE_U32:   return GL_RGB32UI;
        case GL_TYPE_S8:    return GL_RGB8I;
        case GL_TYPE_S16:   return GL_RGB16I;
        case GL_TYPE_S32:   return GL_RGB32I;
        case GL_TYPE_R32:   return GL_RGB32F;
        default: ASSERT(false);
        }
    } break ;
    case GL_CHANNEL_COUNT_4: {
        switch (type) {
        case GL_TYPE_U8:    return GL_RGBA8UI;
        case GL_TYPE_U16:   return GL_RGBA16UI;
        case GL_TYPE_U32:   return GL_RGBA32UI;
        case GL_TYPE_S8:    return GL_RGBA8I;
        case GL_TYPE_S16:   return GL_RGBA16I;
        case GL_TYPE_S32:   return GL_RGBA32I;
        case GL_TYPE_R32:   return GL_RGBA32F;
        default: ASSERT(false);
        }
    } break ;
    default: ASSERT(false);
    }

    return 0;
}

static GLenum primitive_type__to_gl(primitive_type_t type) {
    switch (type) {
    case PRIMITIVE_TYPE_POINT:          return GL_POINTS;
    case PRIMITIVE_TYPE_LINE:           return GL_LINES;
    case PRIMITIVE_TYPE_LINE_STRIP:     return GL_LINE_STRIP;
    case PRIMITIVE_TYPE_LINE_LOOP:      return GL_LINE_LOOP;
    case PRIMITIVE_TYPE_TRIANGLE:       return GL_TRIANGLES;
    case PRIMITIVE_TYPE_TRIANGLE_STRIP: return GL_TRIANGLE_STRIP;
    case PRIMITIVE_TYPE_TRIANGLE_FAN:   return GL_TRIANGLE_FAN;
    case PRIMITIVE_TYPE_PATCHES:        return GL_PATCHES;
    default: ASSERT(false);
    }

    return 0;
}

static bool geometry_object__load_from_g_modelformat(geometry_object_t* self, char* buffer, size_t buffer_size) {
    (void) self;
    (void) buffer;
    (void) buffer_size;
    
    return true;
}

static uint32_t texture_type__to_gl(texture_type_t type) {
    switch (type) {
    case TEXTURE_TYPE_1D:             return GL_TEXTURE_1D;
    case TEXTURE_TYPE_1D_ARRAY:       return GL_TEXTURE_1D_ARRAY;
    case TEXTURE_TYPE_2D:             return GL_TEXTURE_2D;
    case TEXTURE_TYPE_2D_ARRAY:       return GL_TEXTURE_2D_ARRAY;
    case TEXTURE_TYPE_3D:             return GL_TEXTURE_3D;
    case TEXTURE_TYPE_CUBE_MAP:       return GL_TEXTURE_CUBE_MAP;
    case TEXTURE_TYPE_CUBE_MAP_ARRAY: return GL_TEXTURE_CUBE_MAP_ARRAY;
    default: ASSERT(false);
    }

    return 0;
}

static uint32_t texture_wrap_type__to_gl(wrap_type_t type) {
    switch (type) {
    case WRAP_TYPE_REPEAT:          return GL_REPEAT;
    case WRAP_TYPE_MIRRORED_REPEAT: return GL_MIRRORED_REPEAT;
    case WRAP_TYPE_CLAMP_TO_EDGE:   return GL_CLAMP_TO_EDGE;
    case WRAP_TYPE_CLAMP_TO_BORDER: return GL_CLAMP_TO_BORDER;
    default: ASSERT(false);
    }

    return 0;
}

static uint32_t texture_filter_stretch_type__to_gl(filter_stretch_type_t stretch_type) {
    switch (stretch_type) {
    case FILTER_STRETCH_TYPE_MINIFICATION:  return GL_TEXTURE_MIN_FILTER;
    case FILTER_STRETCH_TYPE_MAGNIFICATION: return GL_TEXTURE_MAG_FILTER;
    default: ASSERT(false);
    }

    return 0;
}

static uint32_t texture_filter_sample_type__to_gl(filter_sample_type_t sample_type) {
    // note: choose nearest mipmap instead of lerping between two
    // GL_NEAREST_MIPMAP_NEAREST;
    // GL_NEAREST_MIPMAP_LINEAR;

    switch (sample_type) {
    case FILTER_SAMPLE_TYPE_NEAREST: return GL_LINEAR_MIPMAP_NEAREST;
    case FILTER_SAMPLE_TYPE_LINEAR:  return GL_LINEAR_MIPMAP_LINEAR;
    default: ASSERT(false);
    }

    return 0;
}
