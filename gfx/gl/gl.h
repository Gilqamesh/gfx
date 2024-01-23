#ifndef GL_H
# define GL_H

# include <stdint.h>
# include <stdbool.h>

/********************************************************************************
 * Module API
 ********************************************************************************/

bool gl__init_context();

uint32_t gl__number_of_extensions();
const char* gl__get_extension_str(uint32_t index);

/********************************************************************************
 * Buffer API
 ********************************************************************************/

// todo: implement https://www.khronos.org/opengl/wiki/Framebuffer_Object, and maybe create separate API from it

struct         gl_buffer; // Immutable buffer
enum           gl_buffer_type;
enum           gl_buffer_access_type;
enum           gl_type;
enum           gl_channel_count;
struct         attached_buffer;
enum           attached_buffer_type;
struct         attached_buffer_color;
struct         attached_buffer_depth;
struct         attached_buffer_stencil;
struct         attached_buffer_depth_stencil;
typedef struct gl_buffer                     gl_buffer_t;
typedef enum   gl_buffer_type                gl_buffer_type_t;
typedef enum   gl_buffer_access_type         gl_buffer_access_type_t;
typedef enum   gl_type                       gl_type_t;
typedef enum   gl_channel_count              gl_channel_count_t;
typedef struct attached_buffer               attached_buffer_t;
typedef enum   attached_buffer_type          attached_buffer_type_t;
typedef struct attached_buffer_color         attached_buffer_color_t;
typedef struct attached_buffer_depth         attached_buffer_depth_t;
typedef struct attached_buffer_stencil       attached_buffer_stencil_t;
typedef struct attached_buffer_depth_stencil attached_buffer_depth_stencil_t;

struct gl_buffer {
    uint32_t size;
    uint32_t target;
    uint32_t access; // todo: currently used for both creation and mapping, but their api have separate flags
    uint32_t id;
    bool     is_bound;
};

enum gl_buffer_type {
    GL_BUFFER_TYPE_VERTEX,
    GL_BUFFER_TYPE_INDEX,
    GL_BUFFER_TYPE_TEXTURE,
    GL_BUFFER_TYPE_UNIFORM,
    GL_BUFFER_TYPE_TRANSFORM_FEEDBACK,
    GL_BUFFER_TYPE_FRAMEBUFFER,

    _GL_BUFFER_TYPE_SIZE
};

enum gl_buffer_access_type {
    /**
     * @brief Client can only write to the buffer
    */
    GL_BUFFER_ACCESS_TYPE_WRITE,

    /**
     * @brief Client can only read from the buffer
    */
    GL_BUFFER_ACCESS_TYPE_READ,

    /**
     * @brief Client can read from or write to the buffer
    */
    GL_BUFFER_ACCESS_TYPE_WRITE_READ,

    // todo: Add coherent and persistent

    _GL_BUFFER_ACCESS_TYPE_SIZE
};

enum gl_type {
    GL_TYPE_U8,
    GL_TYPE_U16,
    GL_TYPE_U32,
    GL_TYPE_S8,
    GL_TYPE_S16,
    GL_TYPE_S32,
    GL_TYPE_R32
};

enum gl_channel_count {
    GL_CHANNEL_COUNT_1 = 1,
    GL_CHANNEL_COUNT_2,
    GL_CHANNEL_COUNT_3,
    GL_CHANNEL_COUNT_4
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

/**
 * @param data used to initialize the buffer, if NULL, the buffer will be uninitialized
*/
void gl_buffer__create(
    gl_buffer_t* self,
    const void* data, uint32_t size,
    gl_buffer_type_t buffer_type, gl_buffer_access_type_t access_type
);
void gl_buffer__destroy(gl_buffer_t* self);

uint32_t gl_buffer__size(gl_buffer_t* self);

/**
 * @brief Client-side write
 * @note Write access must have been defined during buffer creation
*/
void gl_buffer__write(gl_buffer_t* self, const void* data, uint32_t size, uint32_t offset);

/**
 * @brief Copies the entirety of one buffer to another
 * @param dst the buffer to copy to
 * @param src the buffer to copy from
 * @note dst and src cannot be the same
 * @note size(dst) >= size(src)
*/
void gl_buffer__copy(
    gl_buffer_t* dst, uint32_t dst_offset,
    gl_buffer_t* src, uint32_t src_offset,
    uint32_t size
);

/**
 * @note If data is NULL, clears with 0s
 * TODO: See if it can be used to clear the color buffer of framebuffer and remove attached_buffer_color__clear* if it does
*/
void gl_buffer__clear(
    gl_buffer_t* self,
    const void* data, gl_channel_count_t data_channel_count, gl_type_t data_type,
    uint32_t size_to_clear, uint32_t offset_to_clear_from
);

/**
 * @brief Maps the buffer's data into client address space to allow for direct modification
 * @note Use it for example in order to avoid creating client side copies and then writing it into the buffer
 * @note Write/Read access for the client depends on how usage was defined when the buffer was created
 * @note If writing, to let the server know that the client is finished writing
 *       either call flush implicitly (in which case persistent access must have been set during buffer creation)
 *       or call the unmap version of this function
*/
void* gl_buffer__map(gl_buffer_t* self, uint32_t size, uint32_t offset);

/**
 * @brief Unmaps and flushes the buffer
*/
void gl_buffer__unmap(gl_buffer_t* self);

void gl_buffer__set_attached_buffer(gl_buffer_t* self, attached_buffer_t* attached_buffer);
attached_buffer_t* gl_buffer__get_attached_buffer(gl_buffer_t* self, attached_buffer_type_t* attached_buffer_type);

void attached_buffer_color__cleariv(attached_buffer_color_t* self, int32_t r, int32_t g, int32_t b, int32_t a);
void attached_buffer_color__clearuv(attached_buffer_color_t* self, uint32_t r, uint32_t g, uint32_t b, uint32_t a);
void attached_buffer_color__clearfv(attached_buffer_color_t* self, float r, float g, float b, float a);

void attached_buffer_stencil__cleariv(attached_buffer_stencil_t* self, int32_t r, int32_t g, int32_t b, int32_t a);

void attached_buffer_depth__clearfv(attached_buffer_depth_t* self, float r, float g, float b, float a);

void attached_buffer_depth_stencil__clearfi(attached_buffer_depth_stencil_t* self, float depth, int32_t stencil);

/********************************************************************************
 * Uniform API
 ********************************************************************************/



/********************************************************************************
 * Shader API
 ********************************************************************************/

enum           shader_type;
struct         shader_object;
struct         shader_program;
struct         shader_program_pipeline;
struct         shader_program_binary;
typedef enum   shader_type             shader_type_t;
typedef struct shader_object           shader_object_t;
typedef struct shader_program          shader_program_t;
typedef struct shader_program_pipeline shader_program_pipeline_t;
typedef struct shader_program_binary   shader_program_binary_t;
typedef void   (*shader_program_predraw_callback_t)(shader_program_t*, void* user_data);

enum shader_type {
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FRAGMENT,
    SHADER_TYPE_TESS_CONTROL,
    SHADER_TYPE_TESS_EVAL,
    SHADER_TYPE_GEOMETRY,
    SHADER_TYPE_COMPUTE
};

/**
 * @brief Shader stage object file
*/
struct shader_object {
    uint32_t id;
    uint32_t type;
};

/**
 * @brief Combination of linked shader stages
*/
struct shader_program {
    uint32_t id;
    uint32_t type;

    shader_program_predraw_callback_t predraw_callback;
};

/**
 * @brief Complete program that can run on the GPU
*/
struct shader_program_pipeline {
    uint32_t          id;

    uint32_t          programs_top;
    shader_program_t* programs[8];
};

struct shader_program_binary {
    uint32_t format;
    uint32_t binary_size;
    void*    binary;
};

//! @brief Creates and compiles a shader object
bool shader_object__create(shader_object_t* self, shader_type_t type, const char* source);
void shader_object__destroy(shader_object_t* self);

bool shader_program__create(shader_program_t* self);
bool shader_program__create_from_shader_program_binary(shader_program_t* self, shader_program_binary_t* shader_program_binary);
void shader_program__destroy(shader_program_t* self);

void shader_program__attach(shader_program_t* self, shader_object_t* shader_object);

//! @note use it on the shader objects after linking (whether successful or not) the shader program
void shader_program__detach(shader_program_t* self, shader_object_t* shader_object);

/**
 * @brief Link the program to its final state, once all the shader objects are attached to it
*/
bool shader_program__link(shader_program_t* self);

/**
 * @brief Callback that is used when the program is used for drawing
 * @note Use the callback to set up uniforms for example
*/
void shader_program__set_predraw_callback(shader_program_t* self, shader_program_predraw_callback_t predraw_callback);

/**
 * @brief Get index location for a uniform subroutine from a shader
 * @note Could also just use "layout (index = n)" to enforce an index for a subroutine in the shader
*/
uint32_t shader_program__get_uniform_subroutine(shader_program_t* self, shader_type_t type, const char* name);

/**
 * @note Must have the program bound before using this
*/
void shader_program__set_uniform_subroutine(shader_program_t* self, shader_type_t type, uint32_t index);

bool shader_program_pipeline__create(shader_program_pipeline_t* self);
void shader_program_pipeline__destroy(shader_program_pipeline_t* self);

void shader_program_pipeline__set(shader_program_pipeline_t* self, shader_program_t* shader_program);
void shader_program_pipeline__bind(shader_program_pipeline_t* self);

bool shader_program_binary__create(shader_program_binary_t* self, shader_program_t* linked_shader_program);
void shader_program_binary__destroy(shader_program_binary_t* self);

/********************************************************************************
 * Vertex API
 ********************************************************************************/

enum           primitive_type;
struct         vertex_stream_specification;
typedef enum   primitive_type               primitive_type_t;
typedef struct vertex_stream_specification  vertex_stream_specification_t;

struct vertex_specification {
    gl_type_t          component_type;
    gl_channel_count_t number_of_components;
    bool               normalized;
};

enum primitive_type {
    /**
     * @brief given a vertex stream of 6 vertices, its interpretation is:
     * {0}, {1}, {2}, {3}, {4}, {5}, {6}
    */
    PRIMITIVE_TYPE_POINT,

    /**
     * @brief given a vertex stream of 6 vertices, its interpretation is:
     * {0, 1}, {2, 3}, {4, 5}, vertex 6 is ignored
    */
    PRIMITIVE_TYPE_LINE,

    /**
     * @brief given a vertex stream of 6 vertices, its interpretation is:
     * {0, 1}, {1, 2}, ..., {5, 6}
    */
    PRIMITIVE_TYPE_LINE_STRIP,

    /**
     * @brief given a vertex stream of 6 vertices, its interpretation is:
     * {0, 1}, {1, 2}, ..., {5, 6}, {6, 1}
    */
    PRIMITIVE_TYPE_LINE_LOOP,

    /**
     * @brief given a vertex stream of 6 vertices, its interpretation is:
     * {0, 1, 2}, {3, 4, 5}
    */
    PRIMITIVE_TYPE_TRIANGLE,        

    /**
     * @brief given a vertex stream of 6 vertices, its interpretation is:
     * {0, 1, 2}, {2, 3, 1}, ..., {5, 6, 4}
     * @note the face direction is determined by the winding of the first triangle
    */
    PRIMITIVE_TYPE_TRIANGLE_STRIP,

    /**
     * @brief given a vertex stream of 6 vertices, its interpretation is:
     * {0, 1, 2}, {0, 2, 3}, {0, 3, 4}, ..., {0, 5, 6}
     * @note every triangle is formed with the first index
    */
    PRIMITIVE_TYPE_TRIANGLE_FAN,

    /**
     * @brief use when tessellation shader is active
    */
    PRIMITIVE_TYPE_PATCHES,

    _PRIMITIVE_TYPE_SIZE
};

/**
 * @brief Ordered list of vertices, as well as an ordered list of primitives
*/
struct vertex_stream_specification {
    primitive_type_t primitive_type;
    uint32_t number_of_vertices;
    uint32_t starting_vertex;
    uint32_t number_of_instances;
    uint32_t starting_instace;
};

//! @param number_of_vertices number of vertices in the stream
//! @param primitive_type allows to interpret the ordered list of vertices as an ordered list of primitives
vertex_stream_specification_t vertex_stream_specification(
    primitive_type_t primitive_type,
    uint32_t number_of_vertices, uint32_t starting_vertex,
    uint32_t number_of_instances, uint32_t starting_instace
);

/********************************************************************************
 * Texture API
 ********************************************************************************/

struct         texture;
enum           texture_type;
struct         texture_sampler;
enum           filter_stretch_type;
enum           filter_sample_type;
enum           wrap_type;
enum           wrap_direction;
typedef struct texture             texture_t;
typedef enum   texture_type        texture_type_t;
typedef struct texture_sampler     texture_sampler_t;
typedef enum   filter_stretch_type filter_stretch_type_t;
typedef enum   filter_sample_type  filter_sample_type_t;
typedef enum   wrap_type           wrap_type_t;
typedef enum   wrap_direction      wrap_direction_t;

struct texture {
    uint32_t id;
};

enum texture_type {
                                /* GLSL SAMPLER UNIFORM TYPE */
    TEXTURE_TYPE_1D             /* sampler1D                 */,
    TEXTURE_TYPE_1D_ARRAY       /* sampler1DArray            */,
    TEXTURE_TYPE_2D             /* sampler2D                 */,
    TEXTURE_TYPE_2D_ARRAY       /* sampler2DArray            */,
    TEXTURE_TYPE_3D             /* sampler3D                 */,
    TEXTURE_TYPE_CUBE_MAP       /* samplerCube               */,
    TEXTURE_TYPE_CUBE_MAP_ARRAY /* samplerCubeArray          */

    /**
     * ex.: TEXTURE_TYPE_2D texture with sampler bound on texture unit 1
     *      layout (binding = 1) uniform sampler2D texture_sampler;
     *      texture(texture_sampler, ivec2 texel coordinate);
    */
};

struct texture_sampler {
    uint32_t id;
};

enum filter_stretch_type {
    FILTER_STRETCH_TYPE_MINIFICATION,
    FILTER_STRETCH_TYPE_MAGNIFICATION
};

enum filter_sample_type {
    FILTER_SAMPLE_TYPE_NEAREST,
    FILTER_SAMPLE_TYPE_LINEAR
};

enum wrap_type {
    WRAP_TYPE_REPEAT,
    WRAP_TYPE_MIRRORED_REPEAT,
    WRAP_TYPE_CLAMP_TO_EDGE   /*  */,
    WRAP_TYPE_CLAMP_TO_BORDER /* taken from a preset border color */
};

enum wrap_direction {
    WRAP_DIRECTION_WIDTH  = 1 << 0 /* affects all types of textures */,
    WRAP_DIRECTION_HEIGHT = 1 << 1 /* affects 2D and 3D textures */,
    WRAP_DIRECTION_DEPTH  = 1 << 2 /* affects only 3D textures */,
};

bool texture__create(
    texture_t* self,
    texture_type_t texture_type, gl_type_t format_type, gl_channel_count_t format_channel_count, uint32_t mipmap_levels,
    void* data, uint32_t width, uint32_t height, uint32_t depth
);

bool texture_sampler__create(texture_sampler_t* self);
void texture_sampler__set_filtering(texture_sampler_t* self, filter_stretch_type_t stretch_type, filter_sample_type_t sample_type);
void texture_sampler__set_wrapping(texture_sampler_t* self, wrap_direction_t bitwise_direction, wrap_type_t wrap_type);

//! @brief Set color for when WRAP_TYPE_CLAMP_TO_BORDER is enabled for wrapping
void texture_sampler__set_wrapping_border_color(texture_sampler_t* self, float red, float green, float blue, float alpha);

void texture__bind(texture_t* self, texture_sampler_t* sampler, uint32_t texture_unit);

/********************************************************************************
 * Geometry API
 ********************************************************************************/

struct         geometry_object;
typedef struct geometry_object geometry_object_t;

struct geometry_object {
    uint32_t id;
    bool     has_index_buffer;
};

void geometry_object__create(geometry_object_t* self);
bool geometry_object__create_from_file(geometry_object_t* self, const char* file_path);
void geometry_object__destroy(geometry_object_t* self);

/***
 * @brief Define vertex attribute format independent of the vertex buffer
 * @param instance_divisor Frequency on how often new vertices are fetched for instanced drawing, 0 means it's fetched based on the vertex index
*/
void geometry_object__define_vertex_attribute_format(geometry_object_t* self, uint32_t attribute_index, gl_type_t components_type, gl_channel_count_t number_of_components, bool normalize, uint32_t instance_divisor);
void geometry_object__enable_vertex_attribute_format(geometry_object_t* self, uint32_t attribute_index, bool enable);

/**
 * @brief Supplies the vertex buffer for the vertex attribute to fetch from
 * @param buffer vertex buffer to be used for the atribute
 * @param vertex_binding_index unique index (not the vertex buffer index, nor the vertex attribute index)
 * @param offset offset to the first vertex in the buffer
 * @param stride distance between vertices in the buffer
*/
void geometry_object__set_vertex_buffer_for_binding(geometry_object_t* self, gl_buffer_t* buffer, uint32_t vertex_binding_index, uint32_t offset, uint32_t stride);

void geometry_object__associate_binding(geometry_object_t* self, uint32_t attribute_index, uint32_t vertex_binding_index);

bool geometry_object__attach_index_buffer(geometry_object_t* self, gl_buffer_t* index_buffer);
void geometry_object__detach_index_buffer(geometry_object_t* self);

/**
 * @note If index buffer is attached, drawing will be indexed
 * @note When drawing is indexed, the vertex stream specification specifies the number of indices from the index buffer, the other parameters remain the same within the specification
 * @param shader_callback_data User data that is passed for shader program callbacks
*/
void geometry_object__draw(geometry_object_t* self, shader_program_pipeline_t* shader, vertex_stream_specification_t vertex_stream_specification, void* shader_callback_data);

/********************************************************************************
 * Context state API
 ********************************************************************************/

enum         polygon_rasterization_mode;
enum         cull_mode;
typedef enum polygon_rasterization_mode polygon_rasterization_mode_t;
typedef enum cull_mode                  cull_mode_t;

enum polygon_rasterization_mode {
    /**
     * @brief Vertices are drawn as points and are not connected
    */
    POLYGON_RASTERIZATION_MODE_POINT,

    /**
     * @brief Vertices are connected with lines, but triangles won't fill
    */
    POLYGON_RASTERIZATION_MODE_LINE,

    /**
     * @brief Vertices that form a triangle will be filled
    */
    POLYGON_RASTERIZATION_MODE_FILL
};

enum cull_mode {
    /**
     * @brief No culling is applied to front and back facing triangles
    */
    CULL_MODE_DISABLED,

    /**
     * @brief Culling is applied only to front-facing triangles
    */
    CULL_MODE_FRONT,

    /**
     * @brief Culling is applied only to back-facing triangles, this is the default
    */
    CULL_MODE_BACK,

    /**
     * @brief Culling is applied to both back and front-facing triangles
    */
    CULL_MODE_FRONT_BACK
};

void gl__viewport(int32_t bottom_left_x, int32_t bottom_left_y, uint32_t width_px, uint32_t height_px);
void gl__scissor(int32_t bottom_left_x, int32_t bottom_left_y, uint32_t width_px, uint32_t height_px);

/**
 * @brief Affects the global point size of Point polygons
 * @note Use gl_PointSize in glsl shader instead of this
*/
void  gl__set_point_size(float size);
float gl__get_point_size();
float gl__get_point_size_min();
float gl__get_point_size_max();

/**
 * @brief Set the context's state for how polygons are drawn
*/
void gl__set_polygon_mode(polygon_rasterization_mode_t mode);

/**
 * @brief Get the context's state for how polygons are drawn
*/
polygon_rasterization_mode_t gl__get_polygon_mode();

/**
 * @brief Set the context's cull state for how triangles are culled
 * @note Point and line primitives are not affected, as they do not have geometric area
*/

void gl__set_cull_mode(cull_mode_t mode);
/**
 * @brief Get the context's cull state for how triangles are culled
 * @note Point and line primitives are not affected, as they do not have geometric area
*/
cull_mode_t gl__get_cull_mode();

#endif // GL_H
