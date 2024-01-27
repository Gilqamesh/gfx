#include "glad/include/glad/glad.h"

#include "file.h"

#include <string.h>

#include "gl_impl.c"

bool gl__init_context() {
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_ERROR, "gladLoadGLLoader failed to load opengl function pointers");
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    // note: may want this
    // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_CULL_FACE);
    glDebugMessageCallback(&gl__error_message_callback, 0);
    // note: to set point size programmatically
    glEnable(GL_PROGRAM_POINT_SIZE);

    // note: to filter out certain messages
    // glDebugMessageControl();

    int32_t major_version = 0;
    int32_t minor_version = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_INFO, "current context opengl version: %d.%d", major_version, minor_version);

    return true;
}

uint32_t gl__number_of_extensions() {
    GLint value;
    glGetIntegerv(GL_NUM_EXTENSIONS, &value);
    return (uint32_t) value;
}

const char* gl__get_extension_str(uint32_t index) {
    ASSERT(index < gl__number_of_extensions());
    return (const char*) glGetStringi(GL_EXTENSIONS, index);
}

void gl_buffer__create(
    gl_buffer_t* self,
    const void* opt_data, uint32_t size,
    gl_buffer_type_t buffer_type, gl_buffer_access_type_t access_type
) {
    memset(self, 0, sizeof(*self));

    self->target = gl_buffer_type__to_gl(buffer_type);
    self->access = gl_buffer_access_type__to_gl(access_type);
    self->size   = size;

    if (buffer_type == GL_BUFFER_TYPE_FRAMEBUFFER) {
        glCreateFramebuffers(1, &self->id);
    } else {
        glCreateBuffers(1, &self->id);
        glNamedBufferStorage(
            self->id, size, opt_data,
            self->access | (self->target & GL_MAP_WRITE_BIT ? GL_DYNAMIC_STORAGE_BIT : 0)
        );
    }

    gl_buffer__unbind(self);
}

void gl_buffer__destroy(gl_buffer_t* self) {
    if (self->target == GL_FRAMEBUFFER) {
        glDeleteFramebuffers(1, &self->id);
    } else {
        glDeleteBuffers(1, &self->id);
    }
}

uint32_t gl_buffer__size(gl_buffer_t* self) {
    return self->size;
}

void gl_buffer__write(gl_buffer_t* self, const void* data, uint32_t size, uint32_t offset) {
    ASSERT(offset + size <= self->size);
    glNamedBufferSubData(self->id, offset, size, data);
}

void gl_buffer__copy(
    gl_buffer_t* dst, uint32_t dst_offset,
    gl_buffer_t* src, uint32_t src_offset,
    uint32_t size
) {
    ASSERT(src->target != GL_FRAMEBUFFER && dst->target != GL_FRAMEBUFFER);
    ASSERT(dst_offset + size <= dst->size);
    ASSERT(src_offset + size <= src->size);
    glCopyNamedBufferSubData(src->id, dst->id, src_offset, dst_offset, size);
}

void gl_buffer__clear(
    gl_buffer_t* self,
    const void* data, gl_channel_count_t data_channel_count, gl_type_t data_type, bool is_normalized,
    uint32_t size_to_clear, uint32_t offset_to_clear_from
) {
    glClearNamedBufferSubData(self->id, gl_type_and_channel__to_internal_format(data_type, data_channel_count, is_normalized), offset_to_clear_from, size_to_clear, gl_channel_count__to_gl(data_channel_count), gl_type__to_gl(data_type), data);
}

void* gl_buffer__map(gl_buffer_t* self, uint32_t size, uint32_t offset) {
    ASSERT(offset + size <= self->size);
    return glMapNamedBufferRange(self->id, offset, size, self->access);
}

void gl_buffer__unmap(gl_buffer_t* self) {
    glUnmapNamedBuffer(self->id);
}

void gl_buffer__bind(gl_buffer_t* self, uint32_t binding_point, uint32_t offset, uint32_t size) {
    // todo(david): limit check for binding point
    // glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS);
    // glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS);
    // glGetIntegerv(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS);
    // todo:  why is there no max for transform feedback
    // glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_BUFFER_BINDINGS);

    ASSERT(offset + size <= self->size);
    glBindBufferRange(self->target, binding_point, self->id, offset, size);
}

void gl_buffer__set_attached_buffer(gl_buffer_t* self, attached_buffer_t* attached_buffer) {
    (void) self;
    (void) attached_buffer;

    // Todo: implement, source: https://www.khronos.org/opengl/wiki/Framebuffer_Object

    /**
     * Todo: to create the attachments
     * void glFramebufferTexture1D(GLenum target​, GLenum attachment​, GLenum textarget​, GLuint texture​, GLint level​);
     * void glFramebufferTexture2D(GLenum target​, GLenum attachment​, GLenum textarget​, GLuint texture​, GLint level​);
     * void glFramebufferTextureLayer(GLenum target​, GLenum attachment​, GLuint texture​, GLint level​, GLint layer​);
    */

    /**
     * Todo: attach
     * void glFramebufferRenderbuffer(GLenum target​, GLenum attachment​, GLenum renderbuffertarget​, GLuint renderbuffer​);
    */

    /**
     * Todo: check for framebuffer completeness
     * GLenum glCheckFramebufferStatus(GLenum target​);
    */
}

attached_buffer_t* gl_buffer__get_attached_buffer(gl_buffer_t* self, attached_buffer_type_t* attached_buffer_type) {
    (void) self;
    (void) attached_buffer_type;
    
    return 0;
}

void attached_buffer_color__cleariv(attached_buffer_color_t* self, int32_t r, int32_t g, int32_t b, int32_t a) {
    (void) self;

    const GLint value[] = { r, g, b, a };
    glClearBufferiv(GL_COLOR, 0, value);
    // glClearBufferiv(GL_COLOR, GL_DRAW_BUFFER0, value);
}

void attached_buffer_color__clearuv(attached_buffer_color_t* self, uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
    (void) self;

    const GLuint value[] = { r, g, b, a };
    glClearBufferuiv(GL_COLOR, 0, value);
    // glClearBufferuiv(GL_COLOR, GL_DRAW_BUFFER0, value);
}

void attached_buffer_color__clearfv(attached_buffer_color_t* self, float r, float g, float b, float a) {
    (void) self;

    const GLfloat value[] = { r, g, b, a };
    glClearBufferfv(GL_COLOR, 0, value);
    // glClearBufferfv(GL_COLOR, GL_DRAW_BUFFER0, value);
}

void attached_buffer_stencil__cleariv(attached_buffer_stencil_t* self, int32_t r, int32_t g, int32_t b, int32_t a) {
    (void) self;

    const GLint value[] = { r, g, b, a };
    glClearBufferiv(GL_STENCIL, 0, value);
}

void attached_buffer_depth__clearfv(attached_buffer_depth_t* self, float r, float g, float b, float a) {
    (void) self;

    const GLfloat value[] = { r, g, b, a };
    glClearBufferfv(GL_DEPTH, 0, value);
}

void attached_buffer_depth_stencil__clearfi(attached_buffer_depth_stencil_t* self, float depth, int32_t stencil) {
    (void) self;

    glClearBufferfi(GL_DEPTH_STENCIL, 0, depth, stencil);
}

bool shader_object__create(shader_object_t* self, shader_type_t type, const char* source) {
    const uint32_t shader_type = shader_type__to_gl(type);
    self->id = glCreateShader(shader_type);
    self->type = shader_type__to_bit(type);
    if (self->id == 0) {
        return false;
    }

    glShaderSource(self->id, 1, &source, NULL);
    glCompileShader(self->id);

    GLint compilation_result = 0;
    glGetShaderiv(self->id, GL_COMPILE_STATUS, &compilation_result);
    if (compilation_result == GL_FALSE) {
        GLint log_size = 0;
        glGetShaderiv(self->id, GL_INFO_LOG_LENGTH, &log_size);
        char error_message[256] = { 0 };
        log_size = MIN(log_size, ARRAY_SIZE(error_message));
        glGetShaderInfoLog(self->id, log_size, &log_size, error_message);
        debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_ERROR, "%s shader compilation error: %s", shader_type__to_str(type), error_message);

        shader_object__destroy(self);
        return false;
    }

    return true;
}

void shader_object__destroy(shader_object_t* self) {
    glDeleteShader(self->id);
}

bool shader_program__create(shader_program_t* self) {
    memset(self, 0, sizeof(*self));

    self->id = glCreateProgram();
    if (self->id == 0) {
        return false;
    }

    return true;
}

bool shader_program__create_from_shader_program_binary(shader_program_t* self, shader_program_binary_t* shader_program_binary) {
    // assert(false && "does not work currently, todo: compare binaries saved vs retrieved");

    if (!shader_program__create(self)) {
        return false;
    }

    glProgramBinary(self->id, shader_program_binary->format, shader_program_binary->binary, shader_program_binary->binary_size);

    GLint link_result = 0;
    glGetProgramiv(self->id, GL_LINK_STATUS, &link_result);
    if (link_result == GL_FALSE) {
        GLint log_size = 0;
        glGetProgramiv(self->id, GL_INFO_LOG_LENGTH, &log_size);
        char error_message[256] = { 0 };
        log_size = MIN(log_size, ARRAY_SIZE(error_message));
        glGetProgramInfoLog(self->id, log_size, &log_size, error_message);
        debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_ERROR, "shader link error: %s", error_message);

        shader_program__destroy(self);
        return false;
    }

    return true;
}

void shader_program__destroy(shader_program_t* self) {
    glDeleteProgram(self->id);
}

void shader_program__attach(shader_program_t* self, shader_object_t* shader_object) {
    self->type |= shader_object->type;
    glAttachShader(self->id, shader_object->id);
}

void shader_program__detach(shader_program_t* self, shader_object_t* shader_object) {
    glDetachShader(self->id, shader_object->id);
}

bool shader_program__link(shader_program_t* self) {
    // note: enable binary retrieval
    glProgramParameteri(self->id, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);

    // note: enable separable compilation
    glProgramParameteri(self->id, GL_PROGRAM_SEPARABLE, GL_TRUE);

    glLinkProgram(self->id);

    GLint link_result = 0;
    glGetProgramiv(self->id, GL_LINK_STATUS, &link_result);
    if (link_result == GL_FALSE) {
        GLint log_size = 0;
        glGetProgramiv(self->id, GL_INFO_LOG_LENGTH, &log_size);
        char error_message[256] = { 0 };
        log_size = MIN(log_size, ARRAY_SIZE(error_message));
        glGetProgramInfoLog(self->id, log_size, &log_size, error_message);
        debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_ERROR, "shader link error: %s", error_message);

        shader_program__destroy(self);
        return false;
    }

    return true;
}

void shader_program__set_predraw_callback(shader_program_t* self, shader_program_predraw_callback_t predraw_callback) {
    self->predraw_callback = predraw_callback;
}

bool shader_program__get_uniform_location(shader_program_t* self, const char* name, uint32_t* location) {
    GLint _location = glGetUniformLocation(self->id, name);
    if (_location == -1) {
        debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_WARN, "could not retrieve uniform location for symbol: '%s'", name);
        return false;
    }

    *location = (uint32_t) _location;

    return true;
}

bool shader_program__get_uniform_block_location(shader_program_t* self, const char* name, uint32_t* location) {
    GLuint _location = glGetUniformBlockIndex(self->id, name);
    if (_location == GL_INVALID_INDEX) {
        debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_WARN, "could not retrieve uniform block location for symbol: '%s'", name);
        return false;
    }

    *location = (uint32_t) _location;

    return true;
}

void shader_program__set_uniform_i(shader_program_t* self, uint32_t location, int32_t value) {
    (void) self;
    glUniform1i(location, value);
}

void shader_program__set_uniform_iv(shader_program_t* self, uint32_t location, const int32_t* values, uint32_t size) {
    (void) self;
    glUniform1iv(location, size, values);
}

void shader_program__set_uniform_f(shader_program_t* self, uint32_t location, float value) {
    (void) self;
    glUniform1f(location, value);
}

void shader_program__set_uniform_fv(shader_program_t* self, uint32_t location, const float* values, uint32_t size) {
    (void) self;
    glUniform1fv(location, size, values);
}

void shader_program__set_uniform_mat(shader_program_t* self, uint32_t location, const float values[16]) {
    (void) self;
    glUniformMatrix4fv(location, 16, GL_FALSE, values);
}

void shader_program__set_uniform_block_binding(shader_program_t* self, uint32_t location, uint32_t binding) {
    GLint max_bindings = 0;
    glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_bindings);
    ASSERT(binding < (uint32_t) max_bindings);
    glUniformBlockBinding(self->id, location, binding);
}

void shader_program__get_uniform_block_member_locations(
    shader_program_t* self,
    uint32_t n_of_uniforms, const char** names, uint32_t* out_locations
) {
    glGetUniformIndices(self->id, n_of_uniforms, names, out_locations);
}

void shader_program__get_uniform_block_info(
    shader_program_t* self,
    uniform_block_info_t info,
    uint32_t n_of_uniforms, const uint32_t* in_locations, uint32_t* out_infos
) {
    glGetActiveUniformsiv(self->id, n_of_uniforms, in_locations, uniform_block_info__to_gl(info), (GLint*) out_infos);
}

uint32_t shader_program__get_uniform_subroutine(shader_program_t* self, shader_type_t type, const char* name) {
    return glGetProgramResourceIndex(self->id, shader_type__to_gl_subroutine(type), name);
}

void shader_program__set_uniform_subroutine(shader_program_t* self, shader_type_t type, uint32_t location) {
    (void) self;
    glUniformSubroutinesuiv(shader_type__to_gl(type), 1, &location);
}

bool shader_program_pipeline__create(shader_program_pipeline_t* self) {
    memset(self, 0, sizeof(*self));

    glGenProgramPipelines(1, &self->id);
    if (self->id == 0) {
        return false;
    }

    return true;
}

void shader_program_pipeline__destroy(shader_program_pipeline_t* self) {
    glDeleteProgramPipelines(1, &self->id);
}

void shader_program_pipeline__set(shader_program_pipeline_t* self, shader_program_t* shader_program) {
    glUseProgramStages(self->id, shader_program->type, shader_program->id);
    ASSERT(self->programs_top < ARRAY_SIZE(self->programs));
    self->programs[self->programs_top++] = shader_program;
}

void shader_program_pipeline__bind(shader_program_pipeline_t* self) {
    glBindProgramPipeline(self->id);
}

bool shader_program_binary__create(shader_program_binary_t* self, shader_program_t* linked_shader_program) {
    glGetProgramiv(linked_shader_program->id, GL_PROGRAM_BINARY_LENGTH, (GLint*) &self->binary_size);
    self->binary = malloc(self->binary_size);
    if (!self->binary) {
        return false;
    }
    glGetProgramBinary(linked_shader_program->id, self->binary_size, 0, &self->format, self->binary);

    return true;
}

void shader_program_binary__destroy(shader_program_binary_t* self) {
    free(self->binary);
}

vertex_stream_specification_t vertex_stream_specification(
    primitive_type_t primitive_type,
    uint32_t number_of_vertices,
    uint32_t starting_vertex,
    uint32_t number_of_instances,
    uint32_t starting_instace
) {
    return (vertex_stream_specification_t) {
        .primitive_type = primitive_type,
        .number_of_vertices = number_of_vertices,
        .starting_vertex = starting_vertex,
        .number_of_instances = number_of_instances,
        .starting_instace = starting_instace
    };
}

bool texture__create(
    texture_t* self,
    texture_type_t texture_type, gl_type_t format_type, gl_channel_count_t format_channel_count, bool is_normalized, uint32_t mipmap_levels,
    const void* data, uint32_t width, uint32_t height, uint32_t depth
) {
    memset(self, 0, sizeof(*self));

    self->dimensions = texture_type__to_dimension_size(texture_type);
    self->gl_channel_count = gl_channel_count__to_gl(format_channel_count);
    self->gl_type = gl_type__to_gl(format_type);
    self->gl_internal_format = gl_type_and_channel__to_internal_format(format_type, format_channel_count, is_normalized);

    glCreateTextures(texture_type__to_gl(texture_type), 1, &self->id);
    if (self->id == 0) {
        return false;
    }
    if (texture_type == TEXTURE_TYPE_BUFFER) {
        return true;
    }

    if (mipmap_levels == 0) {
        mipmap_levels = 1;
    }

    glTextureParameteri(self->id, GL_TEXTURE_BASE_LEVEL, 0);
    glTextureParameteri(self->id, GL_TEXTURE_MAX_LEVEL, mipmap_levels - 1);

    switch (self->dimensions) {
    case 1: {
        glTextureStorage1D(
            self->id,
            mipmap_levels,
            self->gl_internal_format,
            width
        );
    } break ;
    case 2: {
        glTextureStorage2D(
            self->id,
            mipmap_levels,
            self->gl_internal_format,
            width,
            height
        );
    } break ;
    case 3: {
        glTextureStorage3D(
            self->id,
            mipmap_levels,
            self->gl_internal_format,
            width,
            height,
            depth
        );
    } break ;
    default: ASSERT(false);
    }

    if (data) {
        texture__write(self, 0, 0, 0, data, width, height, depth);
    }

    if (mipmap_levels > 1) {
        glGenerateTextureMipmap(self->id);
    }

    return true;
}

void texture__destroy(texture_t* self) {
    glDeleteTextures(1, &self->id);
}

void texture__write(
    texture_t* self,
    uint32_t x_offset, uint32_t y_offset, uint32_t z_offset,
    const void* data, uint32_t width, uint32_t height, uint32_t depth
) {
    switch (self->dimensions) {
    case 1: {
        glTextureSubImage1D(
            self->id,
            0,
            x_offset,
            width,
            self->gl_channel_count,
            self->gl_type,
            data
        );
    } break ;
    case 2: {
        glTextureSubImage2D(
            self->id,
            0,
            x_offset,
            y_offset,
            width,
            height,
            self->gl_channel_count,
            self->gl_type,
            data
        );
    } break ;
    case 3: {
        glTextureSubImage3D(
            self->id,
            0,
            x_offset,
            y_offset,
            z_offset,
            width,
            height,
            depth,
            self->gl_channel_count,
            self->gl_type,
            data
        );
    } break ;
    default: ASSERT(false);
    }
}

void texture__attach_buffer(texture_t* self, gl_buffer_t* buffer, uint32_t buffer_offset, uint32_t buffer_size) {
    ASSERT(buffer_offset + buffer_size <= buffer->size);
    // note: offset must be aligned
    // GLint alignment = 0;
    // glGetIntegerv(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, &alignment);
    GLint max_size = 0;
    glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE, &max_size);
    ASSERT(buffer_size < (uint32_t) max_size);
    glTextureBufferRange(self->id, self->gl_internal_format, buffer->id, buffer_offset, buffer_size);
}

bool texture_sampler__create(texture_sampler_t* self) {
    glCreateSamplers(1, &self->id);
    if (self->id == 0) {
        return false;
    }

    texture_sampler__set_filtering(self, FILTER_STRETCH_TYPE_MINIFICATION, FILTER_SAMPLE_TYPE_LINEAR_MIPMAP_LINEAR);
    texture_sampler__set_filtering(self, FILTER_STRETCH_TYPE_MAGNIFICATION, FILTER_SAMPLE_TYPE_LINEAR);
    texture_sampler__set_wrapping(
        self,
        WRAP_DIRECTION_WIDTH | WRAP_DIRECTION_HEIGHT | WRAP_DIRECTION_DEPTH,
        WRAP_TYPE_REPEAT
    );

    return true;
}

void texture_sampler__destroy(texture_sampler_t* self) {
    glDeleteSamplers(1, &self->id);
}

void texture_sampler__set_filtering(texture_sampler_t* self, filter_stretch_type_t stretch_type, filter_sample_type_t sample_type) {
    glSamplerParameteri(self->id, texture_filter_stretch_type__to_gl(stretch_type), texture_filter_sample_type__to_gl(sample_type));
}

void texture_sampler__set_wrapping(texture_sampler_t* self, uint32_t bitwise_wrap_direction, wrap_type_t wrap_type) {
    const uint32_t gl_wrap_type = texture_wrap_type__to_gl(wrap_type);
    if (bitwise_wrap_direction & WRAP_DIRECTION_WIDTH) {
        glSamplerParameteri(self->id, GL_TEXTURE_WRAP_S, gl_wrap_type);
    }
    if (bitwise_wrap_direction & WRAP_DIRECTION_HEIGHT) {
        glSamplerParameteri(self->id, GL_TEXTURE_WRAP_T, gl_wrap_type);
    }
    if (bitwise_wrap_direction & WRAP_DIRECTION_DEPTH) {
        glSamplerParameteri(self->id, GL_TEXTURE_WRAP_R, gl_wrap_type);
    }
}

void texture_sampler__set_wrapping_border_color(texture_sampler_t* self, float red, float green, float blue, float alpha) {
    const GLfloat v[4] = { red, green, blue, alpha };
    glSamplerParameterfv(self->id, GL_TEXTURE_BORDER_COLOR, v);
}

void texture__bind(texture_t* self, texture_sampler_t* opt_sampler, uint32_t texture_unit) {
    // todo: cache this for the module
    GLint max_texture_units = 0;
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_texture_units);
    ASSERT(texture_unit < (uint32_t) max_texture_units);

    glBindTextureUnit(texture_unit, self->id);
    if (opt_sampler) {
        glBindSampler(texture_unit, opt_sampler->id);
    }
}

void geometry_object__create(geometry_object_t* self) {
    memset(self, 0, sizeof(*self));

    glCreateVertexArrays(1, &self->id);
}

bool geometry_object__create_from_file(geometry_object_t* self, const char* file_path) {
    // parse extension and decide to use custom format or not
    char* dot_pos = strrchr(file_path, '.');
    if (!dot_pos) {
        return false;
    }
    if (strcmp(dot_pos, ".g_modelformat")) {
        return false;
    }

    size_t file_size = 0;
    if (!file__size(file_path, &file_size)) {
        return false;
    }

    file_t file;
    if (!file__open(&file, file_path, FILE_ACCESS_MODE_READ, FILE_CREATION_MODE_OPEN)) {
        return false;
    }

    char* buffer = malloc(file_size);
    size_t bytes_read = 0;
    if (!file__read(&file, buffer, file_size, &bytes_read)) {
        file__close(&file);
        free(buffer);
        return false;
    }

    geometry_object__create(self);
    bool result = geometry_object__load_from_g_modelformat(self, buffer, file_size);

    file__close(&file);
    free(buffer);

    return result;
}

void geometry_object__destroy(geometry_object_t* self) {
    glDeleteVertexArrays(1, &self->id);
}

void geometry_object__define_vertex_attribute_format(geometry_object_t* self, uint32_t attribute_index, gl_type_t components_type, gl_channel_count_t number_of_components, bool normalize, uint32_t instance_divisor) {
    glVertexArrayAttribFormat(
        self->id,
        attribute_index,
        gl_channel_count__to_size(number_of_components),
        gl_type__to_gl(components_type),
        normalize,
        0
    );
    glVertexArrayBindingDivisor(self->id, attribute_index, instance_divisor);
}

void geometry_object__enable_vertex_attribute_format(geometry_object_t* self, uint32_t attribute_index, bool enable) {
    GLint result = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &result);
    ASSERT(attribute_index < (uint32_t) result);

    if (enable) {
        glEnableVertexArrayAttrib(self->id, attribute_index);
    } else {
        glDisableVertexArrayAttrib(self->id, attribute_index);
    }
}

void geometry_object__set_vertex_buffer_for_binding(geometry_object_t* self, gl_buffer_t* vertex_buffer, uint32_t vertex_binding_index, uint32_t offset_to_first_vertex, uint32_t stride) {
    GLint result = 0;
    // todo: move to global module variable
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_BINDINGS, &result);
    ASSERT(vertex_binding_index < (uint32_t) result);
    glVertexArrayVertexBuffer(self->id, vertex_binding_index, vertex_buffer->id, offset_to_first_vertex, stride);
}

void geometry_object__associate_binding(geometry_object_t* self, uint32_t attribute_index, uint32_t vertex_binding_index) {
    glVertexArrayAttribBinding(self->id, attribute_index, vertex_binding_index);
}

bool geometry_object__attach_index_buffer(geometry_object_t* self, gl_buffer_t* index_buffer) {
    ASSERT(index_buffer->target == GL_ELEMENT_ARRAY_BUFFER);
    self->has_index_buffer = true;

    glVertexArrayElementBuffer(self->id, index_buffer->id);

    return true;
}

void geometry_object__detach_index_buffer(geometry_object_t* self) {
    self->has_index_buffer = false;

    glVertexArrayElementBuffer(self->id, 0);
}

void geometry_object__draw(
    geometry_object_t* self,
    shader_program_pipeline_t* shader,
    vertex_stream_specification_t vertex_stream_specification,
    void* shader_callback_data
) {
    geometry_object__bind(self);
    shader_program_pipeline__bind(shader);

    for (uint32_t program_index = 0; program_index < shader->programs_top; ++program_index) {
        shader_program_t* shader_program = shader->programs[program_index];
        glActiveShaderProgram(shader->id, shader_program->id);
        if (shader_program->predraw_callback) {
            shader_program->predraw_callback(shader_program, shader_callback_data);
        }
    }

    if (self->has_index_buffer) {
        glDrawElementsInstancedBaseVertexBaseInstance(
            primitive_type__to_gl(vertex_stream_specification.primitive_type),
            vertex_stream_specification.number_of_vertices,
            GL_UNSIGNED_INT,
            0,
            vertex_stream_specification.number_of_instances,
            vertex_stream_specification.starting_vertex,
            vertex_stream_specification.starting_instace
        );
    } else {
        glDrawArraysInstancedBaseInstance(
            primitive_type__to_gl(vertex_stream_specification.primitive_type),
            vertex_stream_specification.starting_vertex,
            vertex_stream_specification.number_of_vertices,
            vertex_stream_specification.number_of_instances,
            vertex_stream_specification.starting_instace
        );
    }

    geometry_object__unbind(self);
}

void gl__viewport(int32_t bottom_left_x, int32_t bottom_left_y, uint32_t width_px, uint32_t height_px) {
    glViewport(bottom_left_x, bottom_left_y, width_px, height_px);
}

void gl__scissor(int32_t bottom_left_x, int32_t bottom_left_y, uint32_t width_px, uint32_t height_px) {
    glScissor(bottom_left_x, bottom_left_y, width_px, height_px);
}

void  gl__set_point_size(float px) {
    glPointSize(px);
}

float gl__get_point_size() {
    GLfloat size;
    glGetFloatv(GL_POINT_SIZE, &size);
    return size;
}

void gl__get_point_size_range(float* min, float* max) {
    GLfloat range[2];
    glGetFloatv(GL_POINT_SIZE_RANGE, range);
    *min = range[0];
    *max = range[1];
}

float gl__get_point_size_min() {
    GLfloat range[2];
    glGetFloatv(GL_POINT_SIZE_RANGE, range);
    return range[0];
}

float gl__get_point_size_max() {
    GLfloat range[2];
    glGetFloatv(GL_POINT_SIZE_RANGE, range);
    return range[1];
}

void gl__set_polygon_mode(polygon_rasterization_mode_t mode) {
    GLenum gl_mode = 0;

    switch (mode) {
    case POLYGON_RASTERIZATION_MODE_POINT: gl_mode = GL_POINT; break ;
    case POLYGON_RASTERIZATION_MODE_LINE:  gl_mode = GL_LINE;  break ;
    case POLYGON_RASTERIZATION_MODE_FILL:  gl_mode = GL_FILL;  break ;
    default: ASSERT(false);
    }
    
    glPolygonMode(GL_FRONT_AND_BACK, gl_mode);
}

polygon_rasterization_mode_t gl__get_polygon_mode() {
    GLint gl_mode;
    glGetIntegerv(GL_POLYGON_MODE, &gl_mode);

    switch (gl_mode) {
    case GL_POINT: return POLYGON_RASTERIZATION_MODE_POINT;
    case GL_LINE:  return POLYGON_RASTERIZATION_MODE_LINE;
    case GL_FILL:  return POLYGON_RASTERIZATION_MODE_FILL;
    default: ASSERT(false);
    }

    return 0;
}

void gl__set_cull_mode(cull_mode_t mode) {
    switch (mode) {
    case CULL_MODE_DISABLED: {
        glDisable(GL_CULL_FACE);
    } break ;
    case CULL_MODE_FRONT: {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
    } break ;
    case CULL_MODE_BACK: {
        glCullFace(GL_BACK);
    } break ;
    case CULL_MODE_FRONT_BACK: {
        glCullFace(GL_FRONT_AND_BACK);
    } break ;
    default: ASSERT(false);
    }
}

cull_mode_t gl__get_cull_mode() {
    if (!glIsEnabled(GL_CULL_FACE)) {
        return CULL_MODE_DISABLED;
    }

    GLint gl_mode;
    glGetIntegerv(GL_CULL_FACE_MODE, &gl_mode);

    switch (gl_mode) {
    case GL_FRONT:          return CULL_MODE_FRONT;
    case GL_BACK:           return CULL_MODE_BACK;
    case GL_FRONT_AND_BACK: return CULL_MODE_FRONT_BACK;
    default: ASSERT(false);
    }

    return 0;
}
