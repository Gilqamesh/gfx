struct compiler {
    const char*  path;
};

struct module_file {
    const char*  src;

    uint32_t cflags_top;
    uint32_t cflags_size;
    char**   cflags;

    uint32_t lflags_top;
    uint32_t lflags_size;
    char**   lflags;

    /**
     * 0  - hasn't been compiled yet
     * >0 - compiling
     * <0 - compiled
    */
    int          compiling_pid;
};

struct module {
    const char* dir;

    uint32_t       files_top;
    uint32_t       files_size;
    module_file_t* files;

    uint32_t        dependencies_top;
    uint32_t        dependencies_size;
    struct module** dependencies;

    /**
     * 0  - hasn't been compiled yet
     * >0 - compiled
     * <0 - compiling
    */
    int             is_compiled;
};

struct program {
    const char* binary_name;

    uint32_t  modules_top;
    uint32_t  modules_size;
    module_t* modules;
};

static void module_file__vprepend_cflag(module_file_t self, const char* cflag_format, va_list ap);
static void module_file__vprepend_lflag(module_file_t self, const char* lflag_format, va_list ap);
static void module_file__vappend_cflag(module_file_t self, const char* cflag_format, va_list ap);
static void module_file__vappend_lflag(module_file_t self, const char* lflag_format, va_list ap);
static module_file_t module_file__create(const char* dir, const char* src);
static void module_file__destroy(module_file_t self);

static void module_file__vprepend_cflag(module_file_t self, const char* cflag_format, va_list ap) {
    module_file__vappend_cflag(self, cflag_format, ap);

    assert(self->cflags_top >= 2);
    char* tmp = self->cflags[self->cflags_top - 2];
    for (uint32_t cflag_index = self->cflags_top - 2; cflag_index > 0; --cflag_index) {
        self->cflags[cflag_index] = self->cflags[cflag_index - 1];
    }
    self->cflags[0] = tmp;
}

static void module_file__vprepend_lflag(module_file_t self, const char* lflag_format, va_list ap) {
    module_file__vappend_lflag(self, lflag_format, ap);

    assert(self->cflags_top >= 2);
    char* tmp = self->lflags[self->lflags_top - 2];
    for (uint32_t lflag_index = self->lflags_top - 2; lflag_index > 0; --lflag_index) {
        self->lflags[lflag_index] = self->lflags[lflag_index - 1];
    }
    self->lflags[0] = tmp;
}

static void module_file__vappend_cflag(module_file_t self, const char* cflag_format, va_list ap) {
    ARRAY_ENSURE_TOP(self->cflags, self->cflags_top, self->cflags_size);

    self->cflags[self->cflags_top] = 0;
    assert(self->cflags_top > 0);
    vasprintf(&self->cflags[self->cflags_top - 1], cflag_format, ap);
    ++self->cflags_top;
}

static void module_file__vappend_lflag(module_file_t self, const char* lflag_format, va_list ap) {
    ARRAY_ENSURE_TOP(self->lflags, self->lflags_top, self->lflags_size);

    self->lflags[self->lflags_top] = 0;
    assert(self->lflags_top > 0);
    vasprintf(&self->lflags[self->lflags_top - 1], lflag_format, ap);
    ++self->lflags_top;
}

static module_file_t module_file__create(const char* dir, const char* src) {
    uint32_t src_len = strlen(src);

    module_file_t result = calloc(1, sizeof(*result));
    if (!result) {
        return 0;
    }

    result->src = src;

    ARRAY_ENSURE_TOP(result->cflags, result->cflags_top, result->cflags_size);
    result->cflags[result->cflags_top++] = 0;
    ARRAY_ENSURE_TOP(result->lflags, result->lflags_top, result->lflags_size);
    result->lflags[result->lflags_top++] = 0;

    module_file__append_cflag(result, "-o");
    assert(src_len > 2);
    module_file__append_cflag(result, "%s/%.*s.o", dir, src_len - 2, src);
    module_file__append_cflag(result, "-c");
    module_file__append_cflag(result, "%s/%s", dir, src);

    return result;
}

static void module_file__destroy(module_file_t self) {
    if (self->cflags) {
        for (uint32_t cflag_index = 0; cflag_index < self->cflags_size; ++cflag_index) {
            free(self->cflags[cflag_index]);
        }
        free(self->cflags);
    }

    if (self->lflags) {
        for (uint32_t lflag_index = 0; lflag_index < self->lflags_size; ++lflag_index) {
            free(self->lflags[lflag_index]);
        }
        free(self->lflags);
    }

    free(self);
}
