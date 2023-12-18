#include "gl.h"

#include "debug.h"
#include "helper_macros.h"

#include <glad/glad.h>
#include <string.h>

#include "gl_internal.c"

bool gl__init(void* (*symbol_loader)(const char*)) {
    if (!gladLoadGLLoader((GLADloadproc) symbol_loader)) {
        debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_ERROR, "gladLoadGLLoader failed to load opengl function pointers");
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    // note: may want this
    // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glEnable(GL_CULL_FACE);
    glDebugMessageCallback(&gl__error_message_callback, 0);

    // note: to filter out certain messages
    // glDebugMessageControl();

    int32_t major_version = 0;
    int32_t minor_version = 0;
    glGetIntegerv(GL_MAJOR_VERSION, &major_version);
    glGetIntegerv(GL_MINOR_VERSION, &minor_version);
    debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_INFO, "current context opengl version: %d.%d", major_version, minor_version);

    return true;
}

void gl__deinit() {
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

const gl_buffer_t* gl_buffer__default_framebuffer() {
    return default_frame_buffer;
}

void gl_buffer__create(
    gl_buffer_t* self, const char* debug_name,
    const void* data, uint32_t size,
    gl_buffer_type_t buffer_type, gl_buffer_access_type_t access_type
) {
    memset(self, 0, sizeof(*self));

    ASSERT(data);

    self->target = gl_buffer_type__to_gl(buffer_type);
    self->access = gl_buffer_access_type__to_gl(access_type);
    self->size   = size;

    if (self->target == GL_FRAMEBUFFER) {
        glCreateFramebuffers(1, &self->id);
    } else {
        glCreateBuffers(1, &self->id);
        glNamedBufferStorage(
            self->id, size, data,
            self->access | (self->target & GL_MAP_WRITE_BIT ? GL_DYNAMIC_STORAGE_BIT : 0)
        );
    }

    glObjectLabel(gl_object_label__from_buffer_type(buffer_type), self->id, -1, debug_name);
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
    const void* data, gl_channel_count_t data_channel_count, gl_type_t data_type,
    uint32_t size_to_clear, uint32_t offset_to_clear_from
) {
    glClearNamedBufferSubData(self->id, gl_type_and_channel__to_internal_format(data_type, data_channel_count), offset_to_clear_from, size_to_clear, gl_channel_count__to_gl(data_channel_count), gl_type__to_gl(data_type), data);
}

void* gl_buffer__map(gl_buffer_t* self, uint32_t size, uint32_t offset) {
    ASSERT(offset + size <= self->size);
    return glMapNamedBufferRange(self->id, offset, size, self->access);
}

void gl_buffer__unmap(gl_buffer_t* self) {
    glUnmapNamedBuffer(self->id);
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
    self->id = glCreateProgram();
    if (self->id == 0) {
        return false;
    }

    return true;
}

void shader_program__destroy(shader_program_t* self) {
    glDeleteProgram(self->id);
}

void shader_program__attach(shader_program_t* self, shader_object_t* shader_object) {
    glAttachShader(self->id, shader_object->id);
}

void shader_program__detach(shader_program_t* self, shader_object_t* shader_object) {
    glDetachShader(self->id, shader_object->id);
}

bool shader_program__link(shader_program_t* self) {
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

void shader_program__bind(shader_program_t* self) {
    glUseProgram(self->id);
}

vertex_specification_t vertex_specification(gl_type_t component_type, gl_channel_count_t component_channel_count, bool normalized) {
    ASSERT(
        (component_type != GL_TYPE_R32) ||
        (component_type == GL_TYPE_R32 && !normalized)
    );
    return (vertex_specification_t) {
        .component_type       = component_type,
        .number_of_components = gl_channel_count__to_size(component_channel_count),
        .normalized           = normalized ? GL_TRUE : GL_FALSE
    };
}

vertex_stream_specification_t vertex_stream_specification(uint32_t number_of_vertices, primitive_type_t primitive_type, uint32_t starting_vertex_offset) {
    return (vertex_stream_specification_t) {
        .number_of_vertices     = number_of_vertices,
        .primitive_type         = primitive_type,
        .starting_vertex_offset = starting_vertex_offset
    };
}

bool geometry_object__create(geometry_object_t* self) {
    memset(self, 0, sizeof(*self));

    glCreateVertexArrays(1, &self->id);
    if (self->id == 0) {
        return false;
    }

    return true;
}

void geometry_object__destroy(geometry_object_t* self) {
    glDeleteVertexArrays(1, &self->id);
}

bool geometry_object__attach_vertex(
    geometry_object_t* self,
    gl_buffer_t* gl_buffer,
    vertex_specification_t vertex_specification,
    uint32_t offset_of_vertex_in_gl_buffer,
    uint32_t stride_to_next_vertex_in_gl_buffer
) {
    ASSERT(vertex_specification.number_of_components > 0);
    ASSERT(offset_of_vertex_in_gl_buffer + (vertex_specification.number_of_components - 1) * stride_to_next_vertex_in_gl_buffer + gl_type__to_size(vertex_specification.component_type) <= gl_buffer->size);

    geometry_object__bind(self);

    GLint max_number_of_vertex_attributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_number_of_vertex_attributes);
    int32_t vertex_attribute_to_enable = -1;
    for (int32_t vertex_attribute_index = 0; vertex_attribute_index < max_number_of_vertex_attributes; ++vertex_attribute_index) {
        GLint result = 0;
        glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_ENABLED, &result);
        if (result == 0) {
            vertex_attribute_to_enable = vertex_attribute_index;
            break ;
        }
    }
    if (vertex_attribute_to_enable == -1) {
        debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_ERROR, "cannot enable more vertex attributes for geometry object");
        geometry_object__unbind(self);
        return false;
    }
    gl_buffer__bind(gl_buffer);
    glEnableVertexAttribArray(vertex_attribute_to_enable);
    glVertexAttribPointer(
        vertex_attribute_to_enable,
        vertex_specification.number_of_components,
        gl_type__to_gl(vertex_specification.component_type),
        vertex_specification.normalized,
        stride_to_next_vertex_in_gl_buffer,
        (void*) (size_t) offset_of_vertex_in_gl_buffer
    );

    geometry_object__unbind(self);
    gl_buffer__unbind(gl_buffer);

    /**
     * Separate the vertex buffer source from the vertex format:
     * 
     * Vertex attributes are a state of the vertex array
     * Vertex buffer bindings is a state of the vertex buffer
     * 
     * // set a binding that a vertex attribute uses to reference a buffer
     * glVertexArrayAttribBinding(
     *  vao,                           // Vertex array object id
     *  vertex_array_attrib_index,     // Vertex array's vertex attribute to use to create the association
     *  vertex_buffer_binding_index    // Vertex buffer binding with which to associate the vertex attribute with
     * );
     * 
     * // bind vertex buffer to vao 
     * glVertexArrayVertexBuffer(
     *  vao,
     *  binding index,
     *  buffer id,
     *  offset,
     *  stride
     * );
     * 
     * glVertexArrayAttribFormat(
     *  vao,
     *  attrib index,
     *  n of components,
     *  type,
     *  normalized,
     *  relativeoffset
     * );
     * 
     * glEnableVertexAttribArray();
    */

    return true;
}

bool geometry_object__attach_index_buffer(geometry_object_t* self, gl_buffer_t* index_buffer) {
    ASSERT(index_buffer->target == GL_ELEMENT_ARRAY_BUFFER);
    self->has_index_buffer = true;

    geometry_object__bind(self);
    gl_buffer__bind(index_buffer);

    geometry_object__unbind(self);
    gl_buffer__unbind(index_buffer);

    return true;
}

void geometry_object__detach_index_buffer(geometry_object_t* self) {
    self->has_index_buffer = false;
}

void geometry_object__draw(geometry_object_t* self, shader_program_t* shader, vertex_stream_specification_t vertex_stream_specification) {
    geometry_object__bind(self);
    shader_program__bind(shader);

    if (self->has_index_buffer) {
        glDrawElementsBaseVertex(
            primitive_type__to_gl(vertex_stream_specification.primitive_type),
            vertex_stream_specification.number_of_vertices,
            GL_UNSIGNED_INT,
            0,
            vertex_stream_specification.starting_vertex_offset
        );
    } else {
        glDrawArrays(
            primitive_type__to_gl(vertex_stream_specification.primitive_type),
            vertex_stream_specification.starting_vertex_offset,
            vertex_stream_specification.number_of_vertices
        );
    }

    geometry_object__unbind(self);
}

void gl__viewport(int32_t bottom_left_x, int32_t bottom_left_y, uint32_t width_px, uint32_t height_px) {
    glViewport(bottom_left_x, bottom_left_y, width_px, height_px);
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
