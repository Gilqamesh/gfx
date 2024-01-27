typedef struct image {
    uint8_t* data;
    uint32_t w;
    uint32_t h;
} image_t;

struct game {
    double         time;
    cursor_t       cursor;
    image_t        window_icon_image;
    image_t        cursor_image;

    window_t       window;

    glm::vec3 position;
    glm::vec3 orientation;
    glm::vec3 up;

    geometry_object_t         geometry;
    shader_program_t          vs_program;
    shader_program_t          fs_program;
    shader_program_pipeline_t shader;
    gl_buffer_t               vertex_buffer;
    // gl_buffer_t       color_buffer;
    // gl_buffer_t               texture_buffer;
    gl_buffer_t               uniform_block_buffer;
    texture_t                 texture;
    texture_sampler_t         texture_sampler_1;
    texture_sampler_t         texture_sampler_2;
};

static bool game__load_images(game_t self);
static bool game__init_game_objects(game_t self);
static bool game__load_shaders(game_t self);
static bool game__init_textures(game_t self);
static bool shader__create_from_file(shader_object_t* self, const char* path, shader_type_t shader_type);
static void shader_program__vs_predraw_callback(shader_program_t* self, void* data);
static void shader_program__fs_predraw_callback(shader_program_t* self, void* data);

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
    gl_buffer__create(&self->uniform_block_buffer, 0, 160, GL_BUFFER_TYPE_UNIFORM, GL_BUFFER_ACCESS_TYPE_WRITE);

    gl__set_cull_mode(CULL_MODE_DISABLED);
    const float vertices[] = {
        1.0, -1.0f,
        0.0, 1.0f,
        -1.0f, -1.0f,
        // 0.0f, 0.0f, 1.0f,
        // 0.0f, 1.0f, 0.0f,
        // 0.0f, 1.0f, 1.0f
        1.5, 1.5,
        1.5, -0.5,
        -0.5, -0.5,
        -0.5, 1.5
    };
    // const float colors[] = {
    //     0.0f, 0.0f, 1.0f,
    //     0.0f, 1.0f, 0.0f,
    //     0.0f, 1.0f, 1.0f
    // };

    gl_buffer__create(&self->vertex_buffer, vertices, sizeof(vertices), GL_BUFFER_TYPE_VERTEX, GL_BUFFER_ACCESS_TYPE_READ);
    // gl_buffer__create(&self->color_buffer, colors, sizeof(colors), GL_BUFFER_TYPE_VERTEX, GL_BUFFER_ACCESS_TYPE_READ);

    // geometry_object__create_from_file(&self->geometry, "game/models/1.g_modelformat");

    geometry_object__create(&self->geometry);

    const uint32_t pos_attribute_index = 0;
    // const uint32_t col_attribute_index = 1;
    const uint32_t tex_attribute_index = 1;
    const uint32_t pos_vertex_binding_index = 0;
    // const uint32_t col_vertex_binding_index = 1;
    const uint32_t tex_vertex_binding_index = 1;

    geometry_object__enable_vertex_attribute_format(&self->geometry, pos_attribute_index, true);
    // geometry_object__enable_vertex_attribute_format(&self->geometry, col_attribute_index, true);
    geometry_object__enable_vertex_attribute_format(&self->geometry, tex_attribute_index, true);

    geometry_object__define_vertex_attribute_format(&self->geometry, pos_attribute_index, GL_TYPE_R32, GL_CHANNEL_COUNT_2, false, 0);
    // geometry_object__define_vertex_attribute_format(&self->geometry, col_attribute_index, GL_TYPE_R32, GL_CHANNEL_COUNT_3, false, 2);
    geometry_object__define_vertex_attribute_format(&self->geometry, tex_attribute_index, GL_TYPE_R32, GL_CHANNEL_COUNT_2, false, 0);

    geometry_object__set_vertex_buffer_for_binding(&self->geometry, &self->vertex_buffer, pos_vertex_binding_index, 0, sizeof(float) * 2);
    // geometry_object__set_vertex_buffer_for_binding(&self->geometry, &self->vertex_buffer, col_vertex_binding_index, 6 * sizeof(float), sizeof(float) * 3);
    geometry_object__set_vertex_buffer_for_binding(&self->geometry, &self->vertex_buffer, tex_vertex_binding_index, 6 * sizeof(float), sizeof(float) * 2);

    geometry_object__associate_binding(&self->geometry, pos_attribute_index, pos_vertex_binding_index);
    // geometry_object__associate_binding(&self->geometry, tex_attribute_index, col_vertex_binding_index);
    geometry_object__associate_binding(&self->geometry, tex_attribute_index, tex_vertex_binding_index);

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

        shader_program__set_predraw_callback(&self->vs_program, &shader_program__vs_predraw_callback);
        shader_program__set_predraw_callback(&self->fs_program, &shader_program__fs_predraw_callback);

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
    // const uint32_t texture_width = 256;
    // const uint32_t texture_height = 256;
    // const uint32_t texture_channels = 3;
    // gl_buffer__create(
    //     &self->texture_buffer,
    //     0, texture_width * texture_height * texture_channels * sizeof(float),
    //     GL_BUFFER_TYPE_TEXTURE, GL_BUFFER_ACCESS_TYPE_WRITE
    // );

    // if (!texture__create(
    //     &self->texture,
    //     TEXTURE_TYPE_BUFFER, GL_TYPE_R32, texture_channels, true, 1,
    //     0, texture_width, texture_height, 0
    // )) {
    //     return false;
    // }
    // float* texture_buffer_data = (float*) gl_buffer__map(&self->texture_buffer, self->texture_buffer.size, 0);
    // ASSERT(texture_buffer_data);
    // for (uint32_t height = 0; height < texture_height; ++height) {
    //     float* texel = texture_buffer_data;
    //     for (uint32_t width = 0; width < texture_width; ++width) {
    //         for (uint32_t channel = 0; channel < texture_channels; ++channel) {
    //             *texel++ = 1.0f / (channel + 1);
    //         }
    //     }
    // }
    // gl_buffer__unmap(&self->texture_buffer);
    // texture__attach_buffer(&self->texture, &self->texture_buffer, 0, self->texture_buffer.size);

    const uint32_t wood_indices = 3;

    for (uint32_t wood_index = 0; wood_index < wood_indices; ++wood_index) {
        int number_of_channels_per_pixel;
        image_t image;
        char texture_path[256];
        snprintf(texture_path, ARRAY_SIZE(texture_path), "game/textures/wood%u.jpg", wood_index);
        image.data = stbi_load(texture_path, (int32_t*) &image.w, (int32_t*) &image.h, &number_of_channels_per_pixel, 0);
        if (!image.data) {
            return false;
        };

        if (wood_index == 0) {
            if (!texture__create(
                &self->texture,
                TEXTURE_TYPE_2D_ARRAY, GL_TYPE_U8, (gl_channel_count_t) number_of_channels_per_pixel, true, 1,
                0, image.w, image.h, wood_indices
            )) {
                return false;
            }
        }
        texture__write(&self->texture, 0, 0, wood_index, image.data, image.w, image.h, 1);

        free(image.data);
    }

    if (!texture_sampler__create(&self->texture_sampler_1)) {
        return false;
    }
    if (!texture_sampler__create(&self->texture_sampler_2)) {
        return false;
    }
    texture_sampler__set_wrapping(&self->texture_sampler_1, WRAP_DIRECTION_WIDTH | WRAP_DIRECTION_HEIGHT, WRAP_TYPE_MIRROR_ONCE);
    texture_sampler__set_wrapping(&self->texture_sampler_2, WRAP_DIRECTION_WIDTH | WRAP_DIRECTION_HEIGHT, WRAP_TYPE_CLAMP_TO_EDGE);
    // texture_sampler__set_wrapping_border_color(&self->texture_sampler_2, 0.0f, 1.0f, 0.0f, 1.0f);

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
    char* buffer = (char*) malloc(file_size + 1);
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

static void shader_program__vs_predraw_callback(shader_program_t* self, void* data) {
    game_t game = (game_t) data;

    uint32_t vs_common_location = 0;
    if (!shader_program__get_uniform_block_location(self, "COMMON", &vs_common_location)) {
        return ;
    }
    const uint32_t vs_common_binding = 0;
    shader_program__set_uniform_block_binding(self, vs_common_location, vs_common_binding);
    const uint32_t vs_common_members_count = 2;
    const char* vs_common_member_names[] = {
        "COMMON.offset",
        "COMMON.mvp",
    };
    uint32_t vs_common_member_indices[vs_common_members_count];
    shader_program__get_uniform_block_member_locations(
        self,
        vs_common_members_count, vs_common_member_names, vs_common_member_indices
    );
    uint32_t vs_common_member_offsets[2];
    shader_program__get_uniform_block_info(
        self,
        UNIFORM_BLOCK_INFO_OFFSET,
        vs_common_members_count, vs_common_member_indices, vs_common_member_offsets
    );
    uint32_t vs_common_member_matrix_strides[2];
    shader_program__get_uniform_block_info(
        self,
        UNIFORM_BLOCK_INFO_MATRIX_STRIDE,
        vs_common_members_count, vs_common_member_indices, vs_common_member_matrix_strides
    );
    uint8_t* vs_common_data = (uint8_t*) gl_buffer__map(&game->uniform_block_buffer, game->uniform_block_buffer.size, 0);
    // mat4_t model_m = mat4__id();
    glm::mat4 view_m = glm::lookAt(game->position, glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    uint32_t window_width;
    uint32_t window_height;
    window__get_windowed_state_content_area(game->window, 0, 0, &window_width, &window_height);
    const uint32_t degrees = 60;
    const float radians = (float) M_PI * (float) degrees / 180.0f;
    glm::mat4 proj_m = glm::perspective(radians, (float) window_width / (float) window_height, 0.1f, 10.0f);
    // glm::mat4 proj_m = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f);
    // mat4_t proj_m = mat4__perspective_frustum(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 10.0f);
    // mat4_t proj_m = mat4__perspective_aspect(M_PI / 2.0f, 1, 1, 1.0f, 10.0f);
    // mat4_t proj_m = mat4__perspective_frustum(-2.0f, 2.0f, -2.0f, 2.0f, -10.0f, 100.0f);
    // mat4_t proj_m = mat4__orthographic(-2.0f, 2.0f, -2.0f, 2.0f, -10.0f, 100.0f);
    // mat4_t mvp = proj_m;
    // glm::mat4 mvp = view_m;
    glm::mat4 mvp = proj_m * view_m;
    // mvp = mat4__mul(mvp, );
    for (uint32_t row = 0; row < 4; ++row) {
        uint32_t offset = vs_common_member_offsets[1] + row * vs_common_member_matrix_strides[1];
        for (uint32_t col = 0; col < 4; ++col) {
            *((float*) (vs_common_data + offset)) = mvp[col][row];
            offset += sizeof(float);
        }
    }
    gl_buffer__unmap(&game->uniform_block_buffer);
    gl_buffer__bind(&game->uniform_block_buffer, vs_common_binding, 0, game->uniform_block_buffer.size);

    uint32_t subroutine_index = ((uint32_t) game->time) % 2 == 0 ? 1 : 2;
    shader_program__set_uniform_subroutine(self, SHADER_TYPE_VERTEX, subroutine_index);
}

static void shader_program__fs_predraw_callback(shader_program_t* self, void* data) {
    game_t game = (game_t) data;
    (void) game;

    const uint32_t texture_unit = 0;

    texture_sampler_t* sampler = ((uint32_t) game->time) % 2 == 0 ? &game->texture_sampler_2 : &game->texture_sampler_1;
    texture__bind(&game->texture, sampler, texture_unit);

    // (void) self;
    uint32_t texture_sampler_location = 0;
    if (!shader_program__get_uniform_location(self, "texture_sampler", &texture_sampler_location)) {
        return ;
    }
    shader_program__set_uniform_i(self, texture_sampler_location, texture_unit);
}
