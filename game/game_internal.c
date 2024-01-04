struct         image;
typedef struct image image_t;

struct image {
    uint8_t* data;
    uint32_t w;
    uint32_t h;
};

struct game {
    cursor_t       cursor;
    image_t        window_icon_image;
    image_t        cursor_image;

    geometry_object_t geometry;
    shader_program_t  shader;
    gl_buffer_t       vertex_buffer;
};

static bool game__load_images(game_t self);
static bool game__init_game_objects(game_t self);
static bool game__load_shaders(game_t self);
static bool shader__create_from_file(shader_object_t* self, const char* path, shader_type_t shader_type);

static bool game__load_images(game_t self) {
    int number_of_channels_per_pixel;
    self->window_icon_image.data = stbi_load("game/textures/icon.png", (int32_t*) &self->window_icon_image.w, (int32_t*) &self->window_icon_image.h, &number_of_channels_per_pixel, 0);
    if (!self->window_icon_image.data) {
        return false;
    }
    self->cursor_image.data = stbi_load("game/textures/cursor.png", (int32_t*) &self->cursor_image.w, (int32_t*) &self->cursor_image.h, &number_of_channels_per_pixel, 0);
    if (!self->cursor_image.data) {
        return false;
    }

    return true;
}

static bool game__init_game_objects(game_t self) {
    // gl__set_cull_mode(CULL_MODE_DISABLED);
    const float vertices[] = {
        0.5, -0.5,
        0.0, 0.5,
        -0.5, -0.5
    };

    gl_buffer__create(&self->vertex_buffer, vertices, sizeof(vertices), GL_BUFFER_TYPE_VERTEX, GL_BUFFER_ACCESS_TYPE_READ);

    if (!game__load_shaders(self)) {
        return false;
    }

    geometry_object__create_from_file(&self->geometry, "game/models/1.g_modelformat");

    const uint32_t attribute_index = 0;
    const uint32_t vertex_binding_index = 0;
    geometry_object__create(&self->geometry);
    geometry_object__define_vertex_attribute_format(&self->geometry, attribute_index, GL_TYPE_R32, GL_CHANNEL_COUNT_2, false);
    geometry_object__enable_vertex_attribute_format(&self->geometry, attribute_index, true);
    geometry_object__set_vertex_buffer_for_binding(&self->geometry, &self->vertex_buffer, vertex_binding_index, 0, sizeof(float) * 2);
    geometry_object__associate_binding(&self->geometry, attribute_index, vertex_binding_index);

    return true;
}

static bool game__load_shaders(game_t self) {
    shader_object_t vertex_shader;
    shader_object_t fragment_shader;
    if (!shader__create_from_file(&vertex_shader, "game/shaders/vertex/1.glsl", SHADER_TYPE_VERTEX)) {
        return false;
    }
    if (!shader__create_from_file(&fragment_shader, "game/shaders/fragment/1.glsl", SHADER_TYPE_FRAGMENT)) {
        return false;
    }

    if (!shader_program__create(&self->shader)) {
        return false;
    }

    shader_program__attach(&self->shader, &vertex_shader);
    shader_program__attach(&self->shader, &fragment_shader);
    if (!shader_program__link(&self->shader)) {
        return false;
    }

    shader_program__detach(&self->shader, &vertex_shader);
    shader_program__detach(&self->shader, &fragment_shader);

    shader_object__destroy(&vertex_shader);
    shader_object__destroy(&fragment_shader);

    return true;
}

static bool shader__create_from_file(shader_object_t* self, const char* path, shader_type_t shader_type) {
    size_t file_size = 0;
    if (!file__size(path, &file_size)) {
        return false;
    }
    file_t file;
    if (!file__open(&file, path, FILE_ACCESS_MODE_READ, FILE_CREATION_MODE_OPEN)) {
        return false;
    }
    char* buffer = malloc(file_size + 1);
    if (!buffer) {
        file__close(&file);
        return false;
    }
    size_t read_bytes = 0;
    if (!file__read(&file, buffer, file_size, &read_bytes)) {
        file__close(&file);
        free(buffer);
        return false;
    }
    ASSERT(read_bytes == file_size);
    buffer[read_bytes] = '\0';
    file__close(&file);

    bool result = shader_object__create(
        self,
        shader_type,
        buffer
    );

    free(buffer);

    return result;
}
