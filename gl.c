#include "gl.h"

#include "debug.h"
#include "helper_macros.h"

#include <glad/glad.h>
#include <string.h>

#include "gl_internal.c"

bool gl__init(GLADloadproc symbol_loader) {
    if (!gladLoadGLLoader(symbol_loader)) {
        debug__write_and_flush(DEBUG_MODULE_GL, DEBUG_ERROR, "gladLoadGLLoader failed to load opengl function pointers");
        return false;
    }

    glEnable(GL_DEBUG_OUTPUT);
    // note: may want this
    // glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(&gl__error_message_callback, 0);

    return true;
}

void gl__deinit() {
}

void gl__viewport(int32_t bottom_left_x, int32_t bottom_left_y, uint32_t width_px, uint32_t height_px) {
    glViewport(bottom_left_x, bottom_left_y, width_px, height_px);
}

// void gl__clear_color(float red_normalized, float green_normalized, float blue_normalized, float alpha_normalized, buffer_types_t buffers_to_clean) {
//     glClearColor(red_normalized, green_normalized, blue_normalized, alpha_normalized);
//     GLbitfield bitfield =
//         (buffers_to_clean & BUFFER_COLOR) / BUFFER_COLOR * GL_COLOR_BUFFER_BIT |
//         (buffers_to_clean & BUFFER_DEPTH) / BUFFER_DEPTH * GL_DEPTH_BUFFER_BIT |
//         (buffers_to_clean & BUFFER_STENCIL) / BUFFER_STENCIL * GL_STENCIL_BUFFER_BIT;
//     glClear(bitfield);
// }

const gl_buffer_t* gl_buffer__default_framebuffer() {
    return default_frame_buffer;
}

void gl_buffer__create(
    gl_buffer_t* self, const char* debug_name,
    const void* data, uint32_t size,
    gl_buffer_type_t buffer_type, gl_buffer_usage_type_t usage_type, gl_bufer_freq_type_t freq_type
) {
    ASSERT(data);

    self->target      = gl_buffer_type__to_gl(buffer_type);
    self->usage_type  = gl_buffer_usage_type__to_gl(usage_type, freq_type);
    self->size        = size;

    if (self->target == GL_FRAMEBUFFER) {
        glGenFramebuffers(1, &self->id);
    } else {
        glGenBuffers(1, &self->id);
        gl_buffer__bind(self);
        glBufferData(self->target, size, data, self->usage_type);
    }

    glObjectLabel(gl_object_label__from_buffer_type(buffer_type), self->id, -1, debug_name);
}

void gl_buffer__destroy(gl_buffer_t* self) {
    if (self->target == GL_FRAMEBUFFER) {
        glDeleteFramebuffers(1, &self->id);
    } else {
        glDeleteBuffers(1, &self->id);
    }
}

void gl_buffer__bind(gl_buffer_t* self) {
    if (self->target == GL_FRAMEBUFFER) {
        glBindFramebuffer(GL_FRAMEBUFFER, self->id);
    } else {
        glBindBuffer(self->target, self->id);
    }
}

void gl_buffer__unbind(gl_buffer_t* self) {
    if (self->target == GL_FRAMEBUFFER) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        glBindBuffer(self->target, 0);
    }
}

void gl_buffer__copy(gl_buffer_t* dst, gl_buffer_t* src) {
    ASSERT(src->target != GL_FRAMEBUFFER && dst->target != GL_FRAMEBUFFER);
    glCopyBufferSubData(src->target, dst->target, 0, 0, src->size);
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
