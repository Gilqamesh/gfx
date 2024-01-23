struct         compiler;
struct         decompiler;

struct         chunk_header;
enum           chunk_header_type;

struct         chunk_version_header;
struct         chunk_list_header;
struct         chunk_position_header;
struct         chunk_normal_header;
struct         chunk_texture_2d_header;
struct         chunk_index_header;
struct         chunk_geometry_header;

enum           chunk_list_type;
struct         chunk_list_emit_parameters;

typedef struct compiler                 compiler_t;
typedef struct decompiler               decompiler_t;

typedef struct chunk_header             chunk_header_t;
typedef enum   chunk_header_type        chunk_header_type_t;

typedef struct chunk_version_header          chunk_version_header_t;
typedef struct chunk_list_header             chunk_list_header_t;
typedef struct chunk_position_header         chunk_position_header_t;
typedef struct chunk_normal_header           chunk_normal_header_t;
typedef struct chunk_texture_2d_header       chunk_texture_2d_header_t;
typedef struct chunk_material_header         chunk_material_header_t;
typedef struct chunk_index_header            chunk_index_header_t;
typedef struct chunk_geometry_header         chunk_geometry_header_t;

typedef enum   chunk_list_type               chunk_list_type_t;
typedef struct chunk_list_emit_parameters    chunk_list_emit_parameters_t;

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

    uint32_t chunk_depth;
    int panic;

    str_builder_t chunk;
};

enum chunk_header_type {
    CHUNK_HEADER_TYPE_VERSION,
    CHUNK_HEADER_TYPE_LIST,
    CHUNK_HEADER_TYPE_POSITION,
    CHUNK_HEADER_TYPE_NORMAL,
    CHUNK_HEADER_TYPE_TEXTURE_2D,
    CHUNK_HEADER_TYPE_MATERIAL,
    CHUNK_HEADER_TYPE_INDEX,
    CHUNK_HEADER_TYPE_GEOMETRY,
};

struct chunk_header {
    uint32_t type;
    uint32_t number_of_subchunks; // any chunk can contain any number of subchunks
    uint32_t size_including_header;
};

struct chunk_version_header {
    chunk_header_t chunk_header;
    // header is followed by a single list chunk of { u32 major, u32 minor } and then 'number_of_subchunks' number of chunks
};

struct chunk_list_header {
    //! NOTE: 'number_of_chunks' from the base must be 0
    chunk_header_t chunk_header;
    //! TODO: add here a unique identifier, so that lists can be referenced
    //! NOTE: could also just use the 'number_of_chunks' from the base
    uint32_t elements_type;
    uint32_t n_of_elements;
    // header is followed by 'n_of_elements' of 32-bit values
};

struct chunk_position_header {
    chunk_header_t chunk_header;
    // header is followed by 'number_of_subchunks' number of list chunks of { r32 x, r32 y, r32 z } tuples
};

struct chunk_normal_header {
    chunk_header_t chunk_header;
    // header is followed by 'number_of_subchunks' number of list chunks of { r32 x, r32 y, r32 z } tuples, the resulting tuples are normalized
};

struct chunk_texture_2d_header {
    chunk_header_t chunk_header;
    //! TODO: figure out this association
    uint32_t texture_file_index;
    // header is followed by list chunks of { r32 u, r32 v } tuples, the r32 values within the tuple are clamped to the range of [0, 1]
};

struct chunk_material_header {
    chunk_header_t chunk_header;
};

struct chunk_index_header {
    chunk_header_t chunk_header;
    // header is followed by list chunks of tuples of u32 values
};

struct chunk_geometry_header {
    chunk_header_t chunk_header;
};

enum chunk_list_type {
    CHUNK_LIST_TYPE_S32,
    CHUNK_LIST_TYPE_R32
};

struct chunk_list_emit_parameters {
    //! NOTE: UINT32_MAX if any amount of elements are allowed
    uint32_t expected_n_of_elements;
    chunk_list_type_t expected_type;
    void (*tuple_processor)(chunk_list_type_t tuples_type, void* tuples, uint32_t tuples_size);
};

static void compiler__create(compiler_t* self, const char* source, size_t source_len);
static size_t compiler__emitted(compiler_t* self);
static int compiler__is_at_end(compiler_t* self);
static token_t compiler__eat(compiler_t* self);
static token_t compiler__ate(compiler_t* self);
static token_t compiler__peak(compiler_t* self);
static int compiler__eat_err(compiler_t* self, token_type_t token_type, const char* format, ...);
static void compiler__err(compiler_t* self, const char* format, ...);
static void compiler__verr(compiler_t* self, const char* format, va_list ap);

static void compiler__emit(compiler_t* self, const void* in, size_t size);
static void compiler__femit(compiler_t* self, const char* format, ...);
static void compiler__vfemit(compiler_t* self, const char* format, va_list ap);
static void compiler__patch(compiler_t* self, size_t patch_at, const void* in, size_t size);

static void compiler__emit_chunk(compiler_t* self);
static void compiler__emit_chunk_version(compiler_t* self);
static void compiler__emit_chunk_list(compiler_t* self, chunk_list_emit_parameters_t* params);
static void compiler__emit_chunk_position(compiler_t* self);
static void compiler__emit_chunk_normal(compiler_t* self);
static void compiler__emit_chunk_texture_2d(compiler_t* self);
static void compiler__emit_chunk_material(compiler_t* self);
static void compiler__emit_chunk_index(compiler_t* self);
static void compiler__emit_chunk_geometry(compiler_t* self);

static void tuple__normalize(chunk_list_type_t list_type, void* tuples, uint32_t tuples_size);
static void tuple__clamp(chunk_list_type_t list_type, void* tuples, uint32_t tuples_size);

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
static void decompiler__emit_chunk_version(decompiler_t* self);
static void decompiler__emit_chunk_list(decompiler_t* self, chunk_list_emit_parameters_t* params);
static void decompiler__emit_chunk_position(decompiler_t* self);
static void decompiler__emit_chunk_normal(decompiler_t* self);
static void decompiler__emit_chunk_texture_2d(decompiler_t* self);
static void decompiler__emit_chunk_material(decompiler_t* self);
static void decompiler__emit_chunk_index(decompiler_t* self);
static void decompiler__emit_chunk_geometry(decompiler_t* self);

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

static token_t compiler__ate(compiler_t* self) {
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
    if (str_builder__len(&self->chunk) < patch_at + size) {
        return ;
    }

    str_builder__patch(&self->chunk, patch_at, in, size);
}

static void compiler__emit_chunk(compiler_t* self) {
    if (compiler__eat_err(self, TOKEN_LEFT_PAREN, "expected left paren at the start of chunk")) {
        return ;
    }

    token_t token = compiler__eat(self);
    switch (token.type) {
        case TOKEN_VERSION:    compiler__emit_chunk_version(self);    break ;
        case TOKEN_S32: {
            chunk_list_emit_parameters_t params = {
                .expected_n_of_elements = UINT32_MAX,
                .expected_type = CHUNK_LIST_TYPE_S32
            };
            compiler__emit_chunk_list(self, &params);
        } break ;
        case TOKEN_R32: {
            chunk_list_emit_parameters_t params = {
                .expected_n_of_elements = UINT32_MAX,
                .expected_type = CHUNK_LIST_TYPE_R32
            };
            compiler__emit_chunk_list(self, &params);
        } break ;
        case TOKEN_POSITION:   compiler__emit_chunk_position(self);   break ;
        case TOKEN_NORMAL:     compiler__emit_chunk_normal(self);     break ;
        case TOKEN_TEXTURE_2D: compiler__emit_chunk_texture_2d(self); break ;
        case TOKEN_MATERIAL:   compiler__emit_chunk_material(self);   break ;
        case TOKEN_INDEX:      compiler__emit_chunk_index(self);      break ;
        case TOKEN_GEOMETRY:   compiler__emit_chunk_geometry(self);   break ;
        default: {
            compiler__err(self, "chunk type is not recognized");
            return ;
        }
    }

    token = compiler__ate(self);
    if (token.type != TOKEN_RIGHT_PAREN) {
        compiler__err(self, "expected right paren at the end of chunk");
    }
}

static void compiler__emit_chunk_version(compiler_t* self) {
    assert(compiler__ate(self).type == TOKEN_VERSION);

    chunk_version_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_VERSION
        }
    };
    const size_t header_at_start = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    chunk_list_emit_parameters_t version_list_params = {
        .expected_n_of_elements = 2,
        .expected_type = CHUNK_LIST_TYPE_S32
    };

    if (compiler__eat_err(self, TOKEN_LEFT_PAREN, "expected left paren at the start of chunk")) {
        return ;
    }
    compiler__emit_chunk_list(self, &version_list_params);
    ++header.chunk_header.number_of_subchunks;

    while (
        !compiler__is_at_end(self) &&
        !self->panic &&
        compiler__peak(self).type != TOKEN_RIGHT_PAREN
    ) {
        compiler__emit_chunk(self);
        ++header.chunk_header.number_of_subchunks;
    }   

    const size_t header_at_end = compiler__emitted(self);
    header.chunk_header.size_including_header = header_at_end - header_at_start;
    compiler__patch(self, header_at_start, &header, sizeof(header));

    if (compiler__eat_err(self, TOKEN_RIGHT_PAREN, "expected right paren at the end of chunk")) {
        return ;
    }
}

static void compiler__emit_chunk_list(compiler_t* self, chunk_list_emit_parameters_t* params) {
    assert(compiler__ate(self).type == TOKEN_LEFT_PAREN);
    size_t elements_stride = 0;
    switch (params->expected_type) {
    case CHUNK_LIST_TYPE_S32: elements_stride = sizeof(int32_t); break ;
    case CHUNK_LIST_TYPE_R32: elements_stride = sizeof(float); break ;
    default: assert(false);
    }

    chunk_list_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_LIST
        },
        .elements_type = params->expected_type,
    };
    const size_t header_at_start = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    const size_t elements_memory_size = 256 * sizeof(int32_t);
    char elements_memory[elements_memory_size];
    str_builder_t elements;
    str_builder__create_static(&elements, elements_memory, elements_memory_size);

    token_t token = compiler__eat(self);
    while (
        !compiler__is_at_end(self) &&
        token.type != TOKEN_RIGHT_PAREN
    ) {
        if (header.n_of_elements == params->expected_n_of_elements) {
            compiler__err(self, "too many elements in list, expected: %u", params->expected_n_of_elements);
            return ;
        }

        if (token.type == TOKEN_S32) {
            int32_t value = token__to_s32(&token);
            if (params->expected_type == CHUNK_LIST_TYPE_R32) {
                float fvalue = (float) value;
                str_builder__append(&elements, &fvalue, sizeof(fvalue));
            } else {
                str_builder__append(&elements, &value, sizeof(value));
            }
        } else if (token.type == TOKEN_R32) {
            float value = token__to_r32(&token);
            if (params->expected_type == CHUNK_LIST_TYPE_S32) {
                int32_t ivalue = (int32_t) value;
                str_builder__append(&elements, &ivalue, sizeof(ivalue));
            } else {
                str_builder__append(&elements, &value, sizeof(value));
            }
        } else {
            compiler__err(self, "unexpected type in list chunk");
            return ;
        }

        token = compiler__eat(self);
        ++header.n_of_elements;
    }

    if (
        params->expected_n_of_elements != UINT32_MAX &&
        header.n_of_elements != params->expected_n_of_elements
    ) {
        compiler__err(
            self, "not enough elements in list, expected: %u, got: %u",
            params->expected_n_of_elements,
            header.n_of_elements
        );
    }

    char* elements_begin = str_builder__str(&elements);

    if (params->tuple_processor) {
        params->tuple_processor(params->expected_type, elements_begin, header.n_of_elements);
    }

    char* elements_cur   = elements_begin;
    char* elements_end   = elements_begin + str_builder__len(&elements);
    while (elements_cur < elements_end) {
        compiler__emit(self, elements_cur, elements_stride);
        elements_cur += elements_stride;
    }

    const size_t header_at_end = compiler__emitted(self);
    header.chunk_header.size_including_header = header_at_end - header_at_start;
    compiler__patch(self, header_at_start, &header, sizeof(header));

    if (token.type != TOKEN_RIGHT_PAREN) {
        compiler__err(self, "expected right paren at the end of chunk");
    }
}

static void compiler__emit_chunk_position(compiler_t* self) {
    assert(compiler__ate(self).type == TOKEN_POSITION);

    chunk_position_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_POSITION
        }
    };
    const size_t header_at_start = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    chunk_list_emit_parameters_t version_list_params = {
        .expected_n_of_elements = 3,
        .tuple_processor = 0,
        .expected_type = CHUNK_LIST_TYPE_R32
    };

    token_t token = compiler__eat(self);
    while (
        !compiler__is_at_end(self) &&
        !self->panic &&
        token.type != TOKEN_RIGHT_PAREN
    ) {
        compiler__emit_chunk_list(self, &version_list_params);
        ++header.chunk_header.number_of_subchunks;
        token = compiler__eat(self);
    }

    const size_t header_at_end = compiler__emitted(self);
    header.chunk_header.size_including_header = header_at_end - header_at_start;
    compiler__patch(self, header_at_start, &header, sizeof(header));

    if (token.type != TOKEN_RIGHT_PAREN) {
        compiler__err(self, "expected right paren at the end of chunk");
    }
}

static void compiler__emit_chunk_normal(compiler_t* self) {
    assert(compiler__ate(self).type == TOKEN_NORMAL);

    assert(false);
    chunk_normal_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_NORMAL
        }
    };
    const size_t header_at_start = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    chunk_list_emit_parameters_t version_list_params = {
        .expected_n_of_elements = 3,
        .tuple_processor = &tuple__normalize,
        .expected_type = CHUNK_LIST_TYPE_R32
    };

    token_t token = compiler__eat(self);
    while (
        !compiler__is_at_end(self) &&
        !self->panic &&
        token.type != TOKEN_RIGHT_PAREN
    ) {
        compiler__emit_chunk_list(self, &version_list_params);
        ++header.chunk_header.number_of_subchunks;
        token = compiler__eat(self);
    }

    const size_t header_at_end = compiler__emitted(self);
    header.chunk_header.size_including_header = header_at_end - header_at_start;
    compiler__patch(self, header_at_start, &header, sizeof(header));

    if (token.type != TOKEN_RIGHT_PAREN) {
        compiler__err(self, "expected right paren at the end of chunk");
    }
}

static void compiler__emit_chunk_texture_2d(compiler_t* self) {
    assert(compiler__ate(self).type == TOKEN_TEXTURE_2D);

    assert(false && "implement texture file index");

    chunk_texture_2d_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_TEXTURE_2D
        }
    };
    const size_t header_at_start = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    chunk_list_emit_parameters_t version_list_params = {
        .expected_n_of_elements = 4,
        .tuple_processor = &tuple__clamp,
        .expected_type = CHUNK_LIST_TYPE_R32
    };
    token_t token = compiler__eat(self);
    while (
        !compiler__is_at_end(self) &&
        !self->panic &&
        token.type != TOKEN_RIGHT_PAREN
    ) {
        compiler__emit_chunk_list(self, &version_list_params);
        ++header.chunk_header.number_of_subchunks;
        token = compiler__eat(self);
    }

    const size_t header_at_end = compiler__emitted(self);
    header.chunk_header.size_including_header = header_at_end - header_at_start;
    compiler__patch(self, header_at_start, &header, sizeof(header));

    if (token.type != TOKEN_RIGHT_PAREN) {
        compiler__err(self, "expected right paren at the end of chunk");
    }
}

static void compiler__emit_chunk_material(compiler_t* self) {
    assert(compiler__ate(self).type == TOKEN_MATERIAL);

    (void) self;
    assert(false);

    chunk_material_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_MATERIAL
        }
    };
    const size_t header_at_start = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    const size_t header_at_end = compiler__emitted(self);
    header.chunk_header.size_including_header = header_at_end - header_at_start;
    compiler__patch(self, header_at_start, &header, sizeof(header));

    // if (token.type != TOKEN_RIGHT_PAREN) {
    //     compiler__err(self, "expected right paren at the end of chunk");
    // }
}

static void compiler__emit_chunk_index(compiler_t* self) {
    assert(compiler__ate(self).type == TOKEN_INDEX);

    chunk_index_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_INDEX
        }
    };
    const size_t header_at_start = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    chunk_list_emit_parameters_t version_list_params = {
        .expected_n_of_elements = UINT32_MAX,
        .tuple_processor = 0,
        .expected_type = CHUNK_LIST_TYPE_S32
    };
    token_t token = compiler__eat(self);
    while (
        !compiler__is_at_end(self) &&
        !self->panic &&
        token.type != TOKEN_RIGHT_PAREN
    ) {
        compiler__emit_chunk_list(self, &version_list_params);
        ++header.chunk_header.number_of_subchunks;
        token = compiler__eat(self);
    }

    const size_t header_at_end = compiler__emitted(self);
    header.chunk_header.size_including_header = header_at_end - header_at_start;
    compiler__patch(self, header_at_start, &header, sizeof(header));

    if (token.type != TOKEN_RIGHT_PAREN) {
        compiler__err(self, "expected right paren at the end of chunk");
    }
}

static void compiler__emit_chunk_geometry(compiler_t* self) {
    assert(compiler__ate(self).type == TOKEN_GEOMETRY);

    chunk_geometry_header_t header = {
        .chunk_header = {
            .type = CHUNK_HEADER_TYPE_GEOMETRY
        }
    };
    const size_t header_at_start = compiler__emitted(self);
    compiler__emit(self, &header, sizeof(header));

    while (
        !compiler__is_at_end(self) &&
        !self->panic &&
        compiler__peak(self).type != TOKEN_RIGHT_PAREN
    ) {
        compiler__emit_chunk(self);
        ++header.chunk_header.number_of_subchunks;
    }

    const size_t header_at_end = compiler__emitted(self);
    header.chunk_header.size_including_header = header_at_end - header_at_start;
    compiler__patch(self, header_at_start, &header, sizeof(header));

    if (compiler__eat_err(self, TOKEN_RIGHT_PAREN, "expected right paren at the end of chunk")) {
        return ;
    }
}

static void tuple__normalize(chunk_list_type_t tuples_type, void* tuples, uint32_t tuples_size) {
    assert(tuples_size > 0);
    assert(tuples_type == CHUNK_LIST_TYPE_R32);

    //! TODO: handle overflow
    double len = 0.0;
    float* tuple_cur = (float*) tuples;
    float* tuple_end = (float*) tuples + tuples_size;
    while (tuple_cur < tuple_end) {
        len += *tuple_cur * *tuple_cur;
        ++tuple_cur;
    }
    len /= tuples_size;

    tuple_cur = (float*) tuples;
    while (tuple_cur < tuple_end) {
        *tuple_cur /= len;
        ++tuple_cur;
    }
}

static void tuple__clamp(chunk_list_type_t tuples_type, void* tuples, uint32_t tuples_size) {
    assert(tuples_type == CHUNK_LIST_TYPE_R32);

    float* tuple_cur = (float*) tuples;
    float* tuple_end = (float*) tuples + tuples_size;
    while (tuple_cur < tuple_end) {
        *tuple_cur = *tuple_cur < 0.0f ? 0.0f : *tuple_cur > 1.0f ? 1.0f : *tuple_cur;
        ++tuple_cur;
    }
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
    case CHUNK_HEADER_TYPE_VERSION: decompiler__emit_chunk_version(self); break ;
    case CHUNK_HEADER_TYPE_LIST: {
        chunk_list_emit_parameters_t params = {
            .expected_n_of_elements = UINT32_MAX,
            .expected_type = CHUNK_LIST_TYPE_R32
        };
        decompiler__emit_chunk_list(self, &params);
    } break ;
    case CHUNK_HEADER_TYPE_POSITION: decompiler__emit_chunk_position(self); break ;
    case CHUNK_HEADER_TYPE_NORMAL: decompiler__emit_chunk_normal(self); break ;
    case CHUNK_HEADER_TYPE_TEXTURE_2D: decompiler__emit_chunk_texture_2d(self); break ;
    case CHUNK_HEADER_TYPE_MATERIAL: decompiler__emit_chunk_material(self); break ;
    case CHUNK_HEADER_TYPE_INDEX: decompiler__emit_chunk_index(self); break ;
    case CHUNK_HEADER_TYPE_GEOMETRY: decompiler__emit_chunk_geometry(self); break ;
    default: decompiler__err(self, "unexpected chunk type");
    }
}

static void decompiler__emit_chunk_version(decompiler_t* self) {
    decompiler__femit(self, "%*.*s(version\n", self->chunk_depth * 2, self->chunk_depth * 2, " ");
    ++self->chunk_depth;

    const chunk_version_header_t* header = (const chunk_version_header_t*) decompiler__eat(self, sizeof(*header));
    if (!header) {
        decompiler__err(self, "expected version chunk");
        return ;
    }

    uint32_t subchunk_index = 0;

    chunk_list_emit_parameters_t params = {
        .expected_n_of_elements = 2,
        .expected_type = CHUNK_LIST_TYPE_S32
    };
    decompiler__emit_chunk_list(self, &params);
    ++subchunk_index;

    while (
        subchunk_index < header->chunk_header.number_of_subchunks && 
        !self->panic
    ) {
        decompiler__emit_chunk(self);
        ++subchunk_index;
    }

    assert(self->chunk_depth > 0);
    --self->chunk_depth;
    decompiler__femit(self, "%*.*s)\n", self->chunk_depth * 2, self->chunk_depth * 2, " ");
}

static void decompiler__emit_chunk_list(decompiler_t* self, chunk_list_emit_parameters_t* params) {
    decompiler__femit(self, "%*.*s(", self->chunk_depth * 2, self->chunk_depth * 2, " ");

    const chunk_list_header_t* header = (const chunk_list_header_t*) decompiler__eat(self, sizeof(*header));
    if (!header) {
        decompiler__err(self, "expected list chunk");
        return ;
    }

    if (header->elements_type != params->expected_type) {
        decompiler__err(self, "diff in expected list types");
        return ;
    }

    uint32_t element_index = 0;
    while (
        element_index < header->n_of_elements &&
        !self->panic
    ) {
        if (element_index == params->expected_n_of_elements) {
            decompiler__err(self, "too many elements in list, expected: %u", params->expected_n_of_elements);
            return ;
        }

        if (params->expected_type == CHUNK_LIST_TYPE_S32) {
            int32_t* value = (int32_t*) decompiler__eat(self, sizeof(*value));
            if (!value) {
                decompiler__err(self, "expected %u more values in list, got: %u", header->n_of_elements, element_index);
                return ;
            }
            decompiler__femit(self, "%d", *value);
        } else if (params->expected_type == CHUNK_LIST_TYPE_R32) {
            float* value = (float*) decompiler__eat(self, sizeof(*value));
            if (!value) {
                decompiler__err(self, "expected %u more values in list, got: %u", header->n_of_elements, element_index);
                return ;
            }
            decompiler__femit(self, "%f", *value);
        } else {
            decompiler__err(self, "unrecognized list type");
            return ;
        }

        if (element_index + 1 < header->n_of_elements) {
            decompiler__femit(self, " ");
        }
        ++element_index;
    }

    decompiler__femit(self, ")\n");
}

static void decompiler__emit_chunk_position(decompiler_t* self) {
    decompiler__femit(self, "%*.*s(positions\n", self->chunk_depth * 2, self->chunk_depth * 2, " ");
    ++self->chunk_depth;

    const chunk_position_header_t* header = (const chunk_position_header_t*) decompiler__eat(self, sizeof(*header));
    if (!header) {
        decompiler__err(self, "expected position chunk");
        return ;
    }
    uint32_t subchunk_index = 0;
    chunk_list_emit_parameters_t params = {
        .expected_n_of_elements = 3,
        .expected_type = CHUNK_LIST_TYPE_R32
    };
    while (
        subchunk_index < header->chunk_header.number_of_subchunks &&
        !self->panic
    ) {
        decompiler__emit_chunk_list(self, &params);
        ++subchunk_index;
    }

    assert(self->chunk_depth > 0);
    --self->chunk_depth;
    decompiler__femit(self, "%*.*s)\n", self->chunk_depth * 2, self->chunk_depth * 2, " ");
}

static void decompiler__emit_chunk_normal(decompiler_t* self) {
    decompiler__femit(self, "%*.*s(normals\n", self->chunk_depth * 2, self->chunk_depth * 2, " ");
    ++self->chunk_depth;

    const chunk_normal_header_t* header = (const chunk_normal_header_t*) decompiler__eat(self, sizeof(*header));
    if (!header) {
        decompiler__err(self, "expected normal chunk");
        return ;
    }
    uint32_t subchunk_index = 0;
    chunk_list_emit_parameters_t params = {
        .expected_n_of_elements = 3,
        .expected_type = CHUNK_LIST_TYPE_R32
    };
    while (
        subchunk_index < header->chunk_header.number_of_subchunks &&
        !self->panic
    ) {
        decompiler__emit_chunk_list(self, &params);
        ++subchunk_index;
    }

    assert(self->chunk_depth > 0);
    --self->chunk_depth;
    decompiler__femit(self, "%*.*s)\n", self->chunk_depth * 2, self->chunk_depth * 2, " ");
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
    decompiler__femit(self, "%*.*s(indices\n", self->chunk_depth * 2, self->chunk_depth * 2, " ");
    ++self->chunk_depth;

    const chunk_index_header_t* header = (const chunk_index_header_t*) decompiler__eat(self, sizeof(*header));
    if (!header) {
        decompiler__err(self, "expected index chunk");
        return ;
    }
    uint32_t subchunk_index = 0;
    chunk_list_emit_parameters_t params = {
        .expected_n_of_elements = UINT32_MAX,
        .expected_type = CHUNK_LIST_TYPE_S32
    };
    while (
        subchunk_index < header->chunk_header.number_of_subchunks &&
        !self->panic
    ) {
        decompiler__emit_chunk_list(self, &params);
        ++subchunk_index;
    }

    assert(self->chunk_depth > 0);
    --self->chunk_depth;
    decompiler__femit(self, "%*.*s)\n", self->chunk_depth * 2, self->chunk_depth * 2, " ");
}

static void decompiler__emit_chunk_geometry(decompiler_t* self) {
    decompiler__femit(self, "%*.*s(geometry\n", self->chunk_depth * 2, self->chunk_depth * 2, " ");
    ++self->chunk_depth;

    const chunk_geometry_header_t* header = (const chunk_geometry_header_t*) decompiler__eat(self, sizeof(*header));
    if (!header) {
        decompiler__err(self, "expected geometry chunk");
        return ;
    }

    uint32_t subchunk_index = 0;
    while (
        subchunk_index < header->chunk_header.number_of_subchunks &&
        !self->panic
    ) {
        decompiler__emit_chunk(self);
        ++subchunk_index;
    }

    assert(self->chunk_depth > 0);
    --self->chunk_depth;
    decompiler__femit(self, "%*.*s)\n", self->chunk_depth * 2, self->chunk_depth * 2, " ");
}
