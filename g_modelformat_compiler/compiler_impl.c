struct         compiler;
struct         decompiler;
struct         file_header;
struct         chunk_header;
enum           chunk_header_type;
struct         chunk_position_header;
struct         chunk_normal_header;
struct         chunk_texture_2d_header;
struct         chunk_index_header;
typedef struct compiler                compiler_t;
typedef struct decompiler              decompiler_t;
typedef struct file_header             file_header_t;
typedef struct chunk_header            chunk_header_t;
typedef enum   chunk_header_type       chunk_header_type_t;
typedef struct chunk_position_header   chunk_position_header_t;
typedef struct chunk_normal_header     chunk_normal_header_t;
typedef struct chunk_texture_2d_header chunk_texture_2d_header_t;
typedef struct chunk_material_header   chunk_material_header_t;
typedef struct chunk_index_header      chunk_index_header_t;

struct compiler {
    scanner_t     scanner;
    token_t       token_cur;
    token_t       token_prev;

    int panic;

    str_builder_t chunk;
};

struct decompiler {
    const char* start;
    const char* cur;
    const char* end;

    int panic;

    str_builder_t chunk;
};

struct file_header {
    uint32_t version_major;
    uint32_t version_minor;
    uint32_t n_of_chunks;
    uint32_t header_size;
};

enum chunk_header_type {
    CHUNK_HEADER_TYPE_POSITION,
    CHUNK_HEADER_TYPE_NORMAL,
    CHUNK_HEADER_TYPE_TEXTURE_2D,
    CHUNK_HEADER_TYPE_MATERIAL,
    CHUNK_HEADER_TYPE_INDEX
};

struct chunk_header {
    uint32_t type;
    uint32_t size_including_header;
};

struct chunk_position_header {
    chunk_header_t chunk_header;
    uint32_t n_of_positions;
    // header is followed by 'n_of_positions' of { r32 x, r32 y, r32 z } tuples
};

struct chunk_normal_header {
    chunk_header_t chunk_header;
    uint32_t n_of_normals;
    // header is followed by 'n_of_normals' of { r32 x, r32 y, r32 z }, the resulting vector is normalized
};

struct chunk_texture_2d_header {
    chunk_header_t chunk_header;
    uint32_t n_of_textures;
    //! TODO: figure out this association
    uint32_t texture_file_index;
    // header is followed by 'n_of_textures' of { r32 u, r32 v }, the r32 values are clamped to the range of [0, 1]
};

struct g_modelformat_chunk_material_header {
    chunk_header_t chunk_header;
    uint32_t n_of_materials;
};

struct chunk_index_header {
    chunk_header_t chunk_header;
    uint32_t n_of_indices;
    // header is followed by 'n_of_indices' of u32 indices
};

static void compiler__create(compiler_t* self, const char* source, size_t source_len);
static size_t compiler__emitted(compiler_t* self);
static int compiler__is_at_end(compiler_t* self);
static token_t compiler__eat(compiler_t* self);
static token_t compiler__peak(compiler_t* self);
static int compiler__eat_err(compiler_t* self, token_type_t token_type, const char* format, ...);
static void compiler__err(compiler_t* self, const char* format, ...);
static void compiler__verr(compiler_t* self, const char* format, va_list ap);

static void compiler__emit(compiler_t* self, const void* in, size_t size);
static void compiler__femit(compiler_t* self, const char* format, ...);
static void compiler__vfemit(compiler_t* self, const char* format, va_list ap);
static void compiler__patch(compiler_t* self, size_t patch_at, const void* in, size_t size);

static void compiler__emit_chunk(compiler_t* self);
static void compiler__emit_chunk_position(compiler_t* self);
static void compiler__emit_chunk_normal(compiler_t* self);
static void compiler__emit_chunk_texture_2d(compiler_t* self);
static void compiler__emit_chunk_material(compiler_t* self);
static void compiler__emit_chunk_index(compiler_t* self);

static void decompiler__create(decompiler_t* self, const char* source, size_t source_len);
static int decompiler__is_at_end(decompiler_t* self);
static const void* decompiler__eat(decompiler_t* self, size_t len);
static const void* decompiler__peak(decompiler_t* self, size_t offset_ahead);
static void decompiler__err(decompiler_t* self, const char* format, ...);
static void decompiler__verr(decompiler_t* self, const char* format, va_list ap);

static void decompiler__emit(decompiler_t* self, const void* in, size_t size);
static void decompiler__femit(decompiler_t* self, const char* format, ...);
static void decompiler__vfemit(decompiler_t* self, const char* format, va_list ap);
static void decompiler__patch(decompiler_t* self, size_t patch_at, const void* in, size_t size);

static void decompiler__emit_chunk(decompiler_t* self);
static void decompiler__emit_chunk_position(decompiler_t* self);
static void decompiler__emit_chunk_normal(decompiler_t* self);
static void decompiler__emit_chunk_texture_2d(decompiler_t* self);
static void decompiler__emit_chunk_material(decompiler_t* self);
static void decompiler__emit_chunk_index(decompiler_t* self);

static void compiler__create(compiler_t* self, const char* source, size_t source_len) {
    memset(self, 0, sizeof(*self));

    scanner__create(&self->scanner, source, source_len);
    str_builder__create(&self->chunk);
}

static size_t compiler__emitted(compiler_t* self) {
    return str_builder__len(&self->chunk);
}

static int compiler__is_at_end(compiler_t* self) {
    return self->token_cur.type == TOKEN_EOF;
}

static token_t compiler__eat(compiler_t* self) {
    self->token_prev = self->token_cur;
    self->token_cur = scanner__next_token(&self->scanner);
    while (self->token_cur.type == TOKEN_COMMENT) {
        self->token_cur = scanner__next_token(&self->scanner);
    }

    return self->token_prev;
}

static token_t compiler__peak(compiler_t* self) {
    return self->token_cur;
}

static int compiler__eat_err(compiler_t* self, token_type_t token_type, const char* format, ...) {
    token_t token = compiler__eat(self);
    if (token.type != token_type) {
        va_list ap;
        va_start(ap, format);
        compiler__verr(self, format, ap);
        va_end(ap);
        return 1;
    }

    return 0;
}

static void compiler__err(compiler_t* self, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    compiler__verr(self, format, ap);
    va_end(ap);
}

static void compiler__verr(compiler_t* self, const char* format, va_list ap) {
    if (self->panic) {
        return ;
    }
    self->panic = 1;

    const char* line_start = self->token_prev.lexeme;
    while (line_start > self->scanner.source_start) {
        if (*(line_start - 1) == '\n') {
            break ;
        }
        --line_start;
    }
    const char* line_end = self->token_prev.lexeme;
    while (line_end < self->scanner.source_end) {
        if (
            (line_end < self->scanner.source_end && *line_end == '\n') ||
            (line_end + 1 < self->scanner.source_end && *line_end == '\r' && *(line_end + 1) == '\n')
        ) {
            break ;
        }
        ++line_end;
    }
    const char* error_at = self->token_prev.lexeme;
    fprintf(
        stderr,
        "error: %u:%u - %u:%u: ",
        self->token_cur.line_s, self->token_cur.col_s,
        self->token_cur.line_e, self->token_cur.col_e
    );
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n%.*s\n", (uint32_t) (line_end - line_start), line_start);
    fprintf(stderr, "%*c^\n", (uint32_t) (error_at - line_start), ' ');
    fflush(stderr);
}

static void compiler__emit(compiler_t* self, const void* in, size_t size) {
    str_builder__append(&self->chunk, in, size);
}

static void compiler__femit(compiler_t* self, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    compiler__vfemit(self, format, ap);
    va_end(ap);
}

static void compiler__vfemit(compiler_t* self, const char* format, va_list ap) {
    str_builder__vfappend(&self->chunk, format, ap);
}

static void compiler__patch(compiler_t* self, size_t patch_at, const void* in, size_t size) {
    str_builder__patch(&self->chunk, patch_at, in, size);
}

static void compiler__emit_chunk(compiler_t* self) {
    token_t token = compiler__eat(self);
    switch (token.type) {
    case TOKEN_POSITION:   compiler__emit_chunk_position(self); break ;
    case TOKEN_NORMAL:     compiler__emit_chunk_normal(self); break ;
    case TOKEN_TEXTURE_2D: compiler__emit_chunk_texture_2d(self); break ;
    case TOKEN_MATERIAL:   compiler__emit_chunk_material(self); break ;
    case TOKEN_INDEX:      compiler__emit_chunk_index(self); break ;
    default: compiler__err(self, "expected chunk");
    }
}

static void compiler__emit_chunk_position(compiler_t* self) {
    if (compiler__eat_err(self, TOKEN_LEFT_PAREN, "expected left paren")) {
        return ;
    }

    chunk_position_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_POSITION,
            .size_including_header = sizeof(chunk_position_header_t)
        },
        .n_of_positions = 0
    };
    size_t header_pos = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    while (
        !compiler__is_at_end(self) &&
        compiler__peak(self).type != TOKEN_RIGHT_PAREN
    ) {
        token_t token = compiler__eat(self);
        if (token.type != TOKEN_NUMBER) {
            compiler__err(self, "expected x value for position");
            return ;
        }
        float x = (float) strntod(token.lexeme, token.lexeme_len);

        token = compiler__eat(self);
        if (token.type != TOKEN_NUMBER) {
            compiler__err(self, "expected y value for position");
            return ;
        }
        float y = (float) strntod(token.lexeme, token.lexeme_len);

        token = compiler__eat(self);
        if (token.type != TOKEN_NUMBER) {
            compiler__err(self, "expected z value for position");
            return ;
        }
        float z = (float) strntod(token.lexeme, token.lexeme_len);

        compiler__emit(self, &x, sizeof(x));
        compiler__emit(self, &y, sizeof(y));
        compiler__emit(self, &z, sizeof(z));

        ++header.n_of_positions;
    }

    if (compiler__eat_err(self, TOKEN_RIGHT_PAREN, "expected right paren")) {
        return ;
    }

    compiler__patch(self, header_pos, &header, sizeof(header));
}

static void compiler__emit_chunk_normal(compiler_t* self) {
    if (compiler__eat_err(self, TOKEN_LEFT_PAREN, "expected left paren")) {
        return ;
    }

    chunk_normal_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_NORMAL,
            .size_including_header = sizeof(chunk_normal_header_t)
        },
        .n_of_normals = 0
    };
    size_t header_pos = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    while (
        !compiler__is_at_end(self) &&
        compiler__peak(self).type != TOKEN_RIGHT_PAREN
    ) {
        token_t token = compiler__eat(self);
        if (token.type != TOKEN_NUMBER) {
            compiler__err(self, "expected x value for normal");
            return ;
        }
        float x = (float) strntod(token.lexeme, token.lexeme_len);

        token = compiler__eat(self);
        if (token.type != TOKEN_NUMBER) {
            compiler__err(self, "expected y value for normal");
            return ;
        }
        float y = (float) strntod(token.lexeme, token.lexeme_len);

        token = compiler__eat(self);
        if (token.type != TOKEN_NUMBER) {
            compiler__err(self, "expected z value for normal");
            return ;
        }
        float z = (float) strntod(token.lexeme, token.lexeme_len);

        vec3_t v = vec3(x, y, z);
        vec3__norm_self(&v);

        compiler__emit(self, &v._[0], sizeof(v._[0]));
        compiler__emit(self, &v._[1], sizeof(v._[1]));
        compiler__emit(self, &v._[2], sizeof(v._[2]));

        ++header.n_of_normals;
    }

    if (compiler__eat_err(self, TOKEN_RIGHT_PAREN, "expected right paren")) {
        return ;
    }

    compiler__patch(self, header_pos, &header, sizeof(header));
}

static void compiler__emit_chunk_texture_2d(compiler_t* self) {
    assert(false && "implement texture file index");

    if (compiler__eat_err(self, TOKEN_LEFT_PAREN, "expected left paren")) {
        return ;
    }

    chunk_texture_2d_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_POSITION,
            .size_including_header = sizeof(chunk_texture_2d_header_t)
        },
        .n_of_textures = 0,
        .texture_file_index = 0
    };
    size_t header_pos = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    while (
        !compiler__is_at_end(self) &&
        compiler__peak(self).type != TOKEN_RIGHT_PAREN
    ) {
        token_t token = compiler__eat(self);
        if (token.type != TOKEN_NUMBER) {
            compiler__err(self, "expected u value for normal");
            return ;
        }
        float x = (float) strntod(token.lexeme, token.lexeme_len);

        token = compiler__eat(self);
        if (token.type != TOKEN_NUMBER) {
            compiler__err(self, "expected v value for normal");
            return ;
        }
        float y = (float) strntod(token.lexeme, token.lexeme_len);

        vec2_t v = vec2(x, y);
        v = vec2__clamp(v, vec2(0.0f, 0.0f), vec2(1.0f, 1.0f));

        compiler__emit(self, &v._[0], sizeof(v._[0]));
        compiler__emit(self, &v._[1], sizeof(v._[1]));

        ++header.n_of_textures;
    }

    if (compiler__eat_err(self, TOKEN_RIGHT_PAREN, "expected right paren")) {
        return ;
    }

    compiler__patch(self, header_pos, &header, sizeof(header));
}

static void compiler__emit_chunk_material(compiler_t* self) {
    (void) self;
    assert(false);
}

static void compiler__emit_chunk_index(compiler_t* self) {
    if (compiler__eat_err(self, TOKEN_LEFT_PAREN, "expected left paren")) {
        return ;
    }

    chunk_index_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_INDEX,
            .size_including_header = sizeof(chunk_index_header_t)
        },
        .n_of_indices = 0
    };
    size_t header_pos = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    while (
        !compiler__is_at_end(self) &&
        compiler__peak(self).type != TOKEN_RIGHT_PAREN
    ) {
        token_t token = compiler__eat(self);
        uint32_t index = (uint32_t) strntod(token.lexeme, token.lexeme_len);

        compiler__emit(self, &index, sizeof(index));

        ++header.n_of_indices;
    }

    if (compiler__eat_err(self, TOKEN_RIGHT_PAREN, "expected right paren")) {
        return ;
    }

    compiler__patch(self, header_pos, &header, sizeof(header));
}

static void decompiler__create(decompiler_t* self, const char* source, size_t source_len) {
    memset(self, 0, sizeof(*self));
    
    self->start = source;
    self->cur   = source;
    self->end   = source + source_len;

    str_builder__create(&self->chunk);
}

static int decompiler__is_at_end(decompiler_t* self) {
    return self->cur == self->end;
}

static const void* decompiler__peak(decompiler_t* self, size_t offset_ahead) {
    const char* result = self->cur + offset_ahead;
    if (self->end < result) {
        result = 0;
    }
    return result;
}

static const void* decompiler__eat(decompiler_t* self, size_t len) {
    const char* result = self->cur;
    self->cur += len;
    if (self->end < self->cur) {
        self->cur = self->end;
        result = 0;
    }
    return result;
}

static void  decompiler__err(decompiler_t* self, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    decompiler__verr(self, format, ap);
    va_end(ap);
}

static void  decompiler__verr(decompiler_t* self, const char* format, va_list ap) {
    if (self->panic) {
        return ;
    }
    self->panic = 1;

    fprintf(stderr, "error: ");
    vfprintf(stderr, format, ap);
    fprintf(stderr, "\n");
}

static void decompiler__emit(decompiler_t* self, const void* in, size_t size) {
    str_builder__append(&self->chunk, in, size);
}

static void decompiler__femit(decompiler_t* self, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    decompiler__vfemit(self, format, ap);
    va_end(ap);
}

static void decompiler__vfemit(decompiler_t* self, const char* format, va_list ap) {
    str_builder__vfappend(&self->chunk, format, ap);
}

static void decompiler__patch(decompiler_t* self, size_t patch_at, const void* in, size_t size) {
    str_builder__patch(&self->chunk, patch_at, in, size);
}

static void decompiler__emit_chunk(decompiler_t* self) {
    chunk_header_t* chunk_header = (chunk_header_t*) decompiler__peak(self, 0);
    if (!chunk_header) {
        decompiler__err(self, "expected chunk, but found eof");
        return ;
    }

    switch (chunk_header->type) {
    case CHUNK_HEADER_TYPE_POSITION: decompiler__emit_chunk_position(self); break ;
    case CHUNK_HEADER_TYPE_NORMAL: decompiler__emit_chunk_normal(self); break ;
    case CHUNK_HEADER_TYPE_TEXTURE_2D: decompiler__emit_chunk_texture_2d(self); break ;
    case CHUNK_HEADER_TYPE_MATERIAL: decompiler__emit_chunk_material(self); break ;
    case CHUNK_HEADER_TYPE_INDEX: decompiler__emit_chunk_index(self); break ;
    default: decompiler__err(self, "unexpected chunk type");
    }
}

static void decompiler__emit_chunk_position(decompiler_t* self) {
    const chunk_position_header_t* header = (const chunk_position_header_t*) decompiler__eat(self, sizeof(*header));
    if (!header) {
        decompiler__err(self, "expected position chunk");
        return ;
    }
    decompiler__femit(self, "positions (\n");
    size_t current_offset_ahead = sizeof(*header);
    for (uint32_t position_index = 0; position_index < header->n_of_positions; ++position_index) {
        const uint32_t expected_position_dimensions = 3;
        for (uint32_t position_dimension_index = 0; position_dimension_index < expected_position_dimensions; ++position_dimension_index) {
            float* dimension_value = (float*) decompiler__peak(self, current_offset_ahead);
            if (!dimension_value) {
                decompiler__err(self, "expected %u more values for position, got: %u", expected_position_dimensions, position_dimension_index);
                return ;
            }
            current_offset_ahead += sizeof(float);
            decompiler__femit(self, "%f ", *dimension_value);
        }
        decompiler__femit(self, "\n");
    }
    decompiler__femit(self, ")\n");
}

static void decompiler__emit_chunk_normal(decompiler_t* self) {
    const chunk_normal_header_t* header = (const chunk_normal_header_t*) decompiler__eat(self, sizeof(*header));
    if (!header) {
        decompiler__err(self, "expected normal chunk");
        return ;
    }
    decompiler__femit(self, "positions (\n");
    size_t current_offset_ahead = sizeof(*header);
    for (uint32_t normal_index = 0; normal_index < header->n_of_normals; ++normal_index) {
        const uint32_t expected_normal_dimensions = 2;
        for (uint32_t position_dimension_index = 0; position_dimension_index < expected_normal_dimensions; ++position_dimension_index) {
            float* dimension_value = (float*) decompiler__peak(self, current_offset_ahead);
            if (!dimension_value) {
                decompiler__err(self, "expected %u more values for normal, got: %u", expected_normal_dimensions, position_dimension_index);
                return ;
            }
            current_offset_ahead += sizeof(float);
            decompiler__femit(self, "%f ", *dimension_value);
        }
        decompiler__femit(self, "\n");
    }
    decompiler__femit(self, ")\n");
}

static void decompiler__emit_chunk_texture_2d(decompiler_t* self) {
    assert(false);
    (void) self;
}

static void decompiler__emit_chunk_material(decompiler_t* self) {
    assert(false);
    (void) self;
}

static void decompiler__emit_chunk_index(decompiler_t* self) {
    const chunk_index_header_t* header = (const chunk_index_header_t*) decompiler__eat(self, sizeof(*header));
    if (!header) {
        decompiler__err(self, "expected index chunk");
        return ;
    }
    decompiler__femit(self, "indices (\n");
    size_t current_offset_ahead = sizeof(*header);
    const uint32_t index_newline_divisor = 3;
    for (uint32_t index_index = 0; index_index < header->n_of_indices; ++index_index) {
        uint32_t* dimension_value = (uint32_t*) decompiler__peak(self, current_offset_ahead);
        if (!dimension_value) {
            decompiler__err(self, "expected %u indices, got: %u", header->n_of_indices, index_index);
            return ;
        }
        current_offset_ahead += sizeof(uint32_t);
        decompiler__femit(self, "%u ", *dimension_value);
        if (index_index > 0 && index_index % index_newline_divisor == 0) {
            decompiler__femit(self, "\n");
        }
    }
    decompiler__femit(self, ")\n");
}
