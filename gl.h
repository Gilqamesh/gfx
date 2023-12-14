#ifndef GL_H
# define GL_H

# include <stdint.h>
# include <stdbool.h>

#include <glad/glad.h>

void gl__viewport(int32_t bottom_left_x, int32_t bottom_left_y, uint32_t width_px, uint32_t height_px);

/********************************************************************************
 * Module API
 ********************************************************************************/

bool gl__init(GLADloadproc symbol_loader);
void gl__deinit();

/********************************************************************************
 * Buffer API
 ********************************************************************************/

struct         gl_buffer;
enum           gl_buffer_type;
enum           gl_buffer_usage_type;
enum           gl_bufer_freq_type;
struct         attached_buffer;
enum           attached_buffer_type;
struct         attached_buffer_color;
struct         attached_buffer_depth;
struct         attached_buffer_stencil;
struct         attached_buffer_depth_stencil;
typedef struct gl_buffer                     gl_buffer_t;
typedef enum   gl_buffer_type                gl_buffer_type_t;
typedef enum   gl_buffer_usage_type          gl_buffer_usage_type_t;
typedef enum   gl_bufer_freq_type            gl_bufer_freq_type_t;
typedef struct attached_buffer               attached_buffer_t;
typedef enum   attached_buffer_type          attached_buffer_type_t;
typedef struct attached_buffer_color         attached_buffer_color_t;
typedef struct attached_buffer_depth         attached_buffer_depth_t;
typedef struct attached_buffer_stencil       attached_buffer_stencil_t;
typedef struct attached_buffer_depth_stencil attached_buffer_depth_stencil_t;

struct gl_buffer {
    uint32_t size;
    uint32_t target;
    uint32_t usage_type;
    uint32_t id;
};

enum gl_buffer_type {
    GL_BUFFER_TYPE_VERTEX,
    GL_BUFFER_TYPE_INDEX,
    GL_BUFFER_TYPE_TEXTURE,
    GL_BUFFER_TYPE_TRANSFORM_FEEDBACK,
    GL_BUFFER_TYPE_FRAMEBUFFER,

    _GL_BUFFER_TYPE_SIZE
};

enum gl_buffer_usage_type {
    GL_BUFFER_USAGE_TYPE_WRITE /* user writes to the buffer,    gl reads from it           */,
    GL_BUFFER_USAGE_TYPE_READ  /* user reads from the buffer,   gl writes to it            */,
    GL_BUFFER_USAGE_TYPE_COPY  /* user neither writes or reads, gl writes/reads to/from it */ ,

    _GL_BUFFER_USAGE_TYPE_SIZE
};

enum gl_bufer_freq_type {
    GL_BUFFER_FREQ_TYPE_STATIC  /* set data once                      */,
    GL_BUFFER_FREQ_TYPE_DYNAMIC /* set data occasionally              */,
    GL_BUFFER_FREQ_TYPE_STREAM  /* set data every or nearly every use */,

    _GL_BUFFER_FREQ_TYPE_SIZE
};

enum attached_buffer_type {
    ATTACHED_BUFFER_TYPE_COLOR,
    ATTACHED_BUFFER_TYPE_STENCIL,
    ATTACHED_BUFFER_TYPE_DEPTH,
    ATTACHED_BUFFER_TYPE_DEPTH_STENCIL,

    _ATTACHED_BUFFER_TYPE_SIZE
};

struct attached_buffer {
    attached_buffer_type_t type;
};

struct attached_buffer_color {
    attached_buffer_t base;
};

struct attached_buffer_depth {
    attached_buffer_t base;
};

struct attached_buffer_stencil {
    attached_buffer_t base;
};

struct attached_buffer_depth_stencil {
    attached_buffer_t base;
};

const gl_buffer_t* gl_buffer__default_framebuffer();

void gl_buffer__create(
    gl_buffer_t* self, const char* debug_name,
    const void* data, uint32_t size,
    gl_buffer_type_t buffer_type, gl_buffer_usage_type_t usage_type, gl_bufer_freq_type_t freq_type
);
void gl_buffer__destroy(gl_buffer_t* self);

void gl_buffer__bind(gl_buffer_t* self);
void gl_buffer__unbind(gl_buffer_t* self);

/**
 * @brief Copies the entirety of one buffer to another
 * @param dst the buffer to copy to
 * @param src the buffer to copy from
 * @note dst and src cannot be the same
 * @note size(dst) >= size(src)
*/
void gl_buffer__copy(gl_buffer_t* dst, gl_buffer_t* src);

void gl_buffer__set_attached_buffer(gl_buffer_t* self, attached_buffer_t* attached_buffer);
attached_buffer_t* gl_buffer__get_attached_buffer(gl_buffer_t* self, attached_buffer_type_t* attached_buffer_type);

void attached_buffer_color__cleariv(attached_buffer_color_t* self, int32_t r, int32_t g, int32_t b, int32_t a);
void attached_buffer_color__clearuv(attached_buffer_color_t* self, uint32_t r, uint32_t g, uint32_t b, uint32_t a);
void attached_buffer_color__clearfv(attached_buffer_color_t* self, float r, float g, float b, float a);

void attached_buffer_stencil__cleariv(attached_buffer_stencil_t* self, int32_t r, int32_t g, int32_t b, int32_t a);

void attached_buffer_depth__clearfv(attached_buffer_depth_t* self, float r, float g, float b, float a);

void attached_buffer_depth_stencil__clearfi(attached_buffer_depth_stencil_t* self, float depth, int32_t stencil);

// todo: https://www.khronos.org/opengl/wiki/Framebuffer_Object
// todo: maybe merge with Buffer API

#endif // GL_H
