struct         image;
typedef struct image image_t;

struct image {
    uint8_t* data;
    uint32_t w;
    uint32_t h;
};

struct game {
    double         time;
    cursor_t       cursor;
    image_t        window_icon_image;
    image_t        cursor_image;

    geometry_object_t         geometry;
    shader_program_t          vs_program;
    shader_program_t          fs_program;
    shader_program_pipeline_t shader;
    gl_buffer_t               vertex_buffer;
    // gl_buffer_t       color_buffer;
    texture_t                 texture;
    texture_sampler_t         texture_sampler_1;
    texture_sampler_t         texture_sampler_2;
};

static bool game__load_images(game_t self);
static bool game__init_game_objects(game_t self);
static bool game__load_shaders(game_t self);
static bool game__init_textures(game_t self);
static bool shader__create_from_file(shader_object_t* self, const char* path, shader_type_t shader_type);
static void shader_program__vs_callback(shader_program_t* self, void* data);
static void shader_program__fs_callback(shader_program_t* self, void* data);

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
        -0.5, -0.5,
        0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f
    };
    // const float colors[] = {
    //     0.0f, 0.0f, 1.0f,
    //     0.0f, 1.0f, 0.0f,
    //     0.0f, 1.0f, 1.0f
    // };

    gl_buffer__create(&self->vertex_buffer, vertices, sizeof(vertices), GL_BUFFER_TYPE_VERTEX, GL_BUFFER_ACCESS_TYPE_READ);
    // gl_buffer__create(&self->color_buffer, colors, sizeof(colors), GL_BUFFER_TYPE_VERTEX, GL_BUFFER_ACCESS_TYPE_READ);

    if (!game__load_shaders(self)) {
        return false;
    }

    geometry_object__create_from_file(&self->geometry, "game/models/1.g_modelformat");

    geometry_object__create(&self->geometry);

    const uint32_t pos_attribute_index = 0;
    const uint32_t col_attribute_index = 1;
    const uint32_t pos_vertex_binding_index = 0;
    const uint32_t col_vertex_binding_index = 1;

    geometry_object__enable_vertex_attribute_format(&self->geometry, pos_attribute_index, true);
    geometry_object__enable_vertex_attribute_format(&self->geometry, col_attribute_index, true);

    geometry_object__define_vertex_attribute_format(&self->geometry, pos_attribute_index, GL_TYPE_R32, GL_CHANNEL_COUNT_2, false, 0);
    geometry_object__define_vertex_attribute_format(&self->geometry, col_attribute_index, GL_TYPE_R32, GL_CHANNEL_COUNT_3, false, 2);

    geometry_object__set_vertex_buffer_for_binding(&self->geometry, &self->vertex_buffer, pos_vertex_binding_index, 0, sizeof(float) * 2);
    geometry_object__set_vertex_buffer_for_binding(&self->geometry, &self->vertex_buffer, col_vertex_binding_index, 6 * sizeof(float), sizeof(float) * 3);

    geometry_object__associate_binding(&self->geometry, pos_attribute_index, pos_vertex_binding_index);
    geometry_object__associate_binding(&self->geometry, col_attribute_index, col_vertex_binding_index);

    return true;
}

static bool game__load_shaders(game_t self) {
    /**
     * Shader binary file format: [format][binary]
     *                                    ^------^ binary size
    */
    // const char* shader_binary_path = "game/shaders/bin/1";
    // if (file__exists(shader_binary_path)) {
    //     file_t file;
    //     if (!file__open(&file, shader_binary_path, FILE_ACCESS_MODE_READ, FILE_CREATION_MODE_OPEN)) {
    //         return false;
    //     }
    //     size_t file_size = 0;
    //     if (!file__size(shader_binary_path, &file_size)) {
    //         return false;
    //     }
    //     shader_program_binary_t shader_program_binary;
    //     assert(file_size >= sizeof(shader_program_binary.binary_size));
    //     shader_program_binary.binary_size = file_size - sizeof(shader_program_binary.binary_size);
    //     shader_program_binary.binary = malloc(shader_program_binary.binary_size);
    //     if (!file__read(&file, &shader_program_binary.format, sizeof(shader_program_binary.binary_size), 0)) {
    //         return false;
    //     }
    //     if (!file__read(&file, shader_program_binary.binary, shader_program_binary.binary_size, 0)) {
    //         return false;
    //     }
    //     file__close(&file);

    //     if (!shader_program__create_from_shader_program_binary(&self->shader, &shader_program_binary)) {
    //         return false;
    //     }
    // } else {

        shader_object_t vertex_shader;
        shader_object_t fragment_shader;
        if (!shader__create_from_file(&vertex_shader, "game/shaders/vertex/1.glsl", SHADER_TYPE_VERTEX)) {
            return false;
        }
        if (!shader__create_from_file(&fragment_shader, "game/shaders/fragment/1.glsl", SHADER_TYPE_FRAGMENT)) {
            return false;
        }

        if (!shader_program__create(&self->vs_program)) {
            return false;
        }
        if (!shader_program__create(&self->fs_program)) {
            return false;
        }

        shader_program__set_predraw_callback(&self->vs_program, &shader_program__vs_callback);
        shader_program__set_predraw_callback(&self->fs_program, &shader_program__fs_callback);

        shader_program__attach(&self->vs_program, &vertex_shader);
        shader_program__attach(&self->fs_program, &fragment_shader);
        if (!shader_program__link(&self->vs_program)) {
            return false;
        }
        if (!shader_program__link(&self->fs_program)) {
            return false;
        }

        shader_program__detach(&self->vs_program, &vertex_shader);
        shader_program__detach(&self->fs_program, &fragment_shader);

        shader_object__destroy(&vertex_shader);
        shader_object__destroy(&fragment_shader);

        if (!shader_program_pipeline__create(&self->shader)) {
            return false;
        }

        shader_program_pipeline__set(&self->shader, &self->vs_program);
        shader_program_pipeline__set(&self->shader, &self->fs_program);

        // shader_program_binary_t shader_program_binary;
        // if (!shader_program_binary__create(&shader_program_binary, &self->shader)) {
        //     return false;
        // }

        // file_t file;
        // if (!file__open(&file, shader_binary_path, FILE_ACCESS_MODE_WRITE, FILE_CREATION_MODE_CREATE)) {
        //     return false;
        // }
        // if (!file__write(&file, &shader_program_binary.format, sizeof(shader_program_binary.format), 0)) {
        //     return false;
        // }
        // if (!file__write(&file, &shader_program_binary.binary, shader_program_binary.binary_size, 0)) {
        //     return false;
        // }
        // file__close(&file);

        // shader_program_binary__destroy(&shader_program_binary);
    // }

    return true;
}

static bool game__init_textures(game_t self) {
    int number_of_channels_per_pixel;
    image_t image;
    image.data = stbi_load("game/textures/wood.jpg", (int32_t*) &image.w, (int32_t*) &image.h, &number_of_channels_per_pixel, 0);
    if (!image.data) {
        return false;
    }

    if (!texture__create(
        &self->texture,
        TEXTURE_TYPE_2D, GL_TYPE_U8, number_of_channels_per_pixel, 1,
        image.data, image.w, image.h, 0
    )) {
        return false;
    }
    free(image.data);

    if (!texture_sampler__create(&self->texture_sampler_1)) {
        return false;
    }
    if (!texture_sampler__create(&self->texture_sampler_2)) {
        return false;
    }
    texture_sampler__set_wrapping(&self->texture_sampler_2, WRAP_DIRECTION_WIDTH, WRAP_TYPE_CLAMP_TO_BORDER);

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

static void shader_program__vs_callback(shader_program_t* self, void* data) {
    game_t game =  (game_t) data;
    uint32_t subroutine_index = ((uint32_t) game->time) % 2 == 0 ? 1 : 2;
    shader_program__set_uniform_subroutine(self, SHADER_TYPE_VERTEX, subroutine_index);
}

static void shader_program__fs_callback(shader_program_t* self, void* data) {
    game_t game =  (game_t) data;

    (void) game;
    (void) self;
}
