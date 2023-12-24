struct         lflag;
typedef struct lflag lflag_t;

struct compiler {
    const char*  path;
};

struct module_file {
    const char*  src;

    uint32_t cflags_top;
    uint32_t cflags_size;
    char**   cflags;

    /**
     * 0  - hasn't been compiled yet
     * >0 - compiling
     * <0 - compiled
    */
    int32_t compiling_pid;
};

struct lflag {
    char*   _;
    int32_t index;
};

struct module {
    const char* dir;

    uint32_t       files_top;
    uint32_t       files_size;
    module_file_t* files;

    uint32_t        dependencies_top;
    uint32_t        dependencies_size;
    struct module** dependencies;

    uint32_t lflags_top;
    uint32_t lflags_size;
    char**   lflags;

    /**
     * Can be used for anything like circular dependency check
     * 'transient_flag_is_used' must be cleared when done using it
    */
    int32_t         transient_flag;
    int32_t         transient_flag_is_used;

    /**
     * 0  - hasn't been compiled yet
     * >0 - compiled
     * <0 - compiling
    */
    int32_t         is_compiled;

    /**
     * 0  - hasn't been linked yet
     * >0 - linked
     * <0 - linking
    */
    int32_t is_linked;
};

static void module_file__vprepend_cflag(module_file_t self, const char* cflag_format, va_list ap);
static void module_file__vappend_cflag(module_file_t self, const char* cflag_format, va_list ap);
static void module__vprepend_lflag(module_t self, const char* lflag_format, va_list ap);
static void module__vappend_lflag(module_t self, const char* lflag_format, va_list ap);
static module_file_t module_file__create(const char* dir, const char* src);
static void module_file__destroy(module_file_t self);
static void module_file__append_dependency_includes_helper(module_file_t self, module_t module, int32_t explore);
static void module_file__append_dependency_includes(module_file_t self, module_t module);
static void module__collect_lib_lflags(module_t self, module_t append_to, int32_t explore);
static void module__collect_file_lflags(module_t self, module_t append_to, int32_t explore);

static int32_t my_vasprintf(char** strp, const char* format, va_list ap);
static int32_t my_asprintf(char** strp, const char* format, ...);

static void module_file__vprepend_cflag(module_file_t self, const char* cflag_format, va_list ap) {
    module_file__vappend_cflag(self, cflag_format, ap);

    assert(self->cflags_top >= 2);
    char* tmp = self->cflags[self->cflags_top - 2];
    for (uint32_t cflag_index = self->cflags_top - 2; cflag_index > 0; --cflag_index) {
        self->cflags[cflag_index] = self->cflags[cflag_index - 1];
    }
    self->cflags[0] = tmp;
}

static void module_file__vappend_cflag(module_file_t self, const char* cflag_format, va_list ap) {
    ARRAY_ENSURE_TOP(self->cflags, self->cflags_top, self->cflags_size);

    self->cflags[self->cflags_top] = 0;
    assert(self->cflags_top > 0);
    my_vasprintf(&self->cflags[self->cflags_top - 1], cflag_format, ap);
    ++self->cflags_top;
}

static void module__vprepend_lflag(module_t self, const char* lflag_format, va_list ap) {
    module__vappend_lflag(self, lflag_format, ap);

    assert(self->lflags_top >= 2);
    char* tmp = self->lflags[self->lflags_top - 2];
    for (uint32_t lflag_index = self->lflags_top - 2; lflag_index > 0; --lflag_index) {
        self->lflags[lflag_index] = self->lflags[lflag_index - 1];
    }
    self->lflags[0] = tmp;
}

static void module__vappend_lflag(module_t self, const char* lflag_format, va_list ap) {
    char* str = 0;
    my_vasprintf(&str, lflag_format, ap);
    assert(str);
    uint32_t start_index = 0;
    uint32_t end_index   = 0;
    for (uint32_t str_index = 0; str[str_index]; ++str_index, ++end_index) {
        if (str[str_index] == ' ') {
            if (end_index != start_index) {
                ARRAY_ENSURE_TOP(self->lflags, self->lflags_top, self->lflags_size);
                self->lflags[self->lflags_top] = 0;
                my_asprintf(&self->lflags[self->lflags_top - 1], "%.*s", end_index - start_index, str + start_index);
                ++self->lflags_top;

                start_index = end_index + 1;
            } else {
                ++start_index;
            }
        }
    }
    if (end_index != start_index) {
        ARRAY_ENSURE_TOP(self->lflags, self->lflags_top, self->lflags_size);
        self->lflags[self->lflags_top] = 0;
        asprintf(&self->lflags[self->lflags_top - 1], "%.*s", end_index - start_index, str + start_index);
        ++self->lflags_top;
    }
    free(str);
}

static module_file_t module_file__create(const char* dir, const char* src) {
    module_file_t result = calloc(1, sizeof(*result));
    if (!result) {
        return 0;
    }

    result->src = src;

    ARRAY_ENSURE_TOP(result->cflags, result->cflags_top, result->cflags_size);
    result->cflags[result->cflags_top++] = 0;

    module_file__append_cflag(result, "-o");
    uint32_t src_len = strlen(src);
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

    free(self);
}

static void module_file__append_dependency_includes_helper(module_file_t self, module_t module, int32_t explore) {
    if (explore) {
        if (module->transient_flag == 1) {
            return ;
        }
        assert(!module->transient_flag_is_used);

        /**
         * 1 - visited
         * 0 - not visited0
        */
        module->transient_flag = 1;
        module->transient_flag_is_used = 1;

        module_file__append_cflag(self, "-I%s", module->dir);

        for (uint32_t dependency_index = 0; dependency_index < module->dependencies_top; ++dependency_index) {
            module_t dependency = module->dependencies[dependency_index];
            module_file__append_dependency_includes_helper(self, dependency, explore);
        }
    } else {
        if (module->transient_flag == 0) {
            return ;
        }
        assert(module->transient_flag_is_used);

        for (uint32_t dependency_index = 0; dependency_index < module->dependencies_top; ++dependency_index) {
            module_t dependency = module->dependencies[dependency_index];
            module_file__append_dependency_includes_helper(self, dependency, explore);
        }

        module->transient_flag         = 0;
        module->transient_flag_is_used = 0;
    }
}

static void module_file__append_dependency_includes(module_file_t self, module_t module) {
    for (uint32_t dependency_index = 0; dependency_index < module->dependencies_top; ++dependency_index) {
        module_t dependency = module->dependencies[dependency_index];
        module_file__append_dependency_includes_helper(self, dependency, 1);
    }
    for (uint32_t dependency_index = 0; dependency_index < module->dependencies_top; ++dependency_index) {
        module_t dependency = module->dependencies[dependency_index];
        module_file__append_dependency_includes_helper(self, dependency, 0);
    }
}

static void module__collect_lib_lflags(module_t self, module_t append_to, int32_t explore) {
    if (explore) {
        if (self->transient_flag) {
            return ;
        }
        assert(!self->transient_flag_is_used);
        /**
         * 1 - visited
         * 0 - not visited
        */
        self->transient_flag = 1;
        self->transient_flag_is_used = 1;

        for (uint32_t dependency_index = 0; dependency_index < self->dependencies_top; ++dependency_index) {
            module_t dependency = self->dependencies[dependency_index];
            module__collect_lib_lflags(dependency, append_to, explore);
        }

        for (size_t lflag_index = 0; lflag_index < self->lflags_top - 1; ++lflag_index) {
            int32_t found = 0;
            for (uint32_t append_to_lflag_index = 3; append_to_lflag_index < append_to->lflags_top - 1; ++append_to_lflag_index) {
                if (strcmp(append_to->lflags[append_to_lflag_index], self->lflags[lflag_index]) == 0) {
                    found = 1;
                    break ;
                }
            }
            if (!found) {
                module__append_lflag(append_to, self->lflags[lflag_index]);
            }
        }
    } else {
        if (self->transient_flag == 0) {
            return ;
        }
        assert(self->transient_flag_is_used);

        for (uint32_t dependency_index = 0; dependency_index < self->dependencies_top; ++dependency_index) {
            module_t dependency = self->dependencies[dependency_index];
            module__collect_lib_lflags(dependency, 0, explore);
        }

        self->transient_flag         = 0;
        self->transient_flag_is_used = 0;
    }
}

static void module__collect_file_lflags(module_t self, module_t append_to, int32_t explore) {
    if (explore) {
        if (self->transient_flag) {
            return ;
        }
        assert(!self->transient_flag_is_used);
        /**
         * 1 - visited
         * 0 - not visited
        */
        self->transient_flag = 1;
        self->transient_flag_is_used = 1;

        for (uint32_t dependency_index = 0; dependency_index < self->dependencies_top; ++dependency_index) {
            module_t dependency = self->dependencies[dependency_index];
            module__collect_file_lflags(dependency, append_to, explore);
        }

        for (uint32_t file_index = 0; file_index < self->files_top; ++file_index) {
            module_file_t file = self->files[file_index];
            uint32_t src_len = strlen(file->src);
            assert(src_len > 2);
            module__append_lflag(append_to, "%s/%.*s.o", self->dir, src_len - 2, file->src);
        }
    } else {
        if (self->transient_flag == 0) {
            return ;
        }
        assert(self->transient_flag_is_used);

        for (uint32_t dependency_index = 0; dependency_index < self->dependencies_top; ++dependency_index) {
            module_t dependency = self->dependencies[dependency_index];
            module__collect_file_lflags(dependency, 0, explore);
        }

        self->transient_flag         = 0;
        self->transient_flag_is_used = 0;
    }
}

static int32_t my_vasprintf(char** strp, const char* format, va_list ap) {
    const uint32_t max_cflag_len = 256;
    char* str = malloc(max_cflag_len);
    const int32_t bytes_needed = vsnprintf(str, max_cflag_len, format, ap);
    assert(bytes_needed >= 0);
    assert(bytes_needed < (int32_t) max_cflag_len);
    *strp = malloc(bytes_needed + 1);
    strncpy(*strp, str, bytes_needed + 1);
    free(str);

    return bytes_needed;
}

static int32_t my_asprintf(char** strp, const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    int32_t result = my_vasprintf(strp, format, ap);

    va_end(ap);

    return result;
}
