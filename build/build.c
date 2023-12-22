#include "build.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "helper_macros.h"

#include "build_impl.c"

compiler_t compiler__create(const char* path) {
    compiler_t result = calloc(1, sizeof(*result));
    if (!result) {
        return 0;
    }

    result->path = path;

    return result;
}

void compiler__destroy(compiler_t self) {
    free(self);
}

void module_file__prepend_cflag(module_file_t self, const char* cflag_format, ...) {
    va_list ap;
    va_start(ap, cflag_format);

    module_file__vprepend_cflag(self, cflag_format, ap);

    va_end(ap);
}

void module_file__append_cflag(module_file_t self, const char* cflag_format, ...) {
    va_list ap;
    va_start(ap, cflag_format);

    module_file__vappend_cflag(self, cflag_format, ap);

    va_end(ap);
}

void module_file__prepend_lflag(module_file_t self, const char* lflag_format, ...) {
    va_list ap;
    va_start(ap, lflag_format);

    module_file__vprepend_lflag(self, lflag_format, ap);

    va_end(ap);
}

void module_file__append_lflag(module_file_t self, const char* lflag_format, ...) {
    va_list ap;
    va_start(ap, lflag_format);

    module_file__vappend_lflag(self, lflag_format, ap);

    va_end(ap);
}

module_t module__create(const char* dir) {
    module_t result = calloc(1, sizeof(*result));

    result->dir = dir;

    return result;
}

void module__destroy(module_t self) {
    if (self->files) {
        for (uint32_t file_index = 0; file_index < self->files_top; ++file_index) {
            module_file__destroy(self->files[file_index]);
        }
        free(self->files);
    }

    if (self->dependencies) {
        free(self->dependencies);
    }

    free(self);
}

const char* module__dir(module_t self) {
    return self->dir;
}

module_file_t module__add_file(module_t self, const char* src) {
    module_file_t result = module_file__create(self->dir, src);
    if (!result) {
        return 0;
    }

    ARRAY_ENSURE_TOP(self->files, self->files_top, self->files_size);

    self->files[self->files_top++] = result;

    return result;
}

void module__add_dependency(module_t self, module_t dependency) {
    ARRAY_ENSURE_TOP(self->dependencies, self->dependencies_top, self->dependencies_size);

    self->dependencies[self->dependencies_top++] = dependency;
}

void module__compile(module_t self, compiler_t compiler) {
    if (self->is_compiled > 0) {
        // already compiled
        return ;
    }

    self->is_compiled = -1;
    for (uint32_t file_index = 0; file_index < self->files_top; ++file_index) {
        module_file_t module_file = self->files[file_index];
        if (module_file->compiling_pid < 0) {
            // already compiled
            continue ;
        }

        module_file__prepend_cflag(module_file, compiler->path);
        for (uint32_t dependency_index = 0; dependency_index < self->dependencies_top; ++dependency_index) {
            module_file__append_cflag(module_file, "-I%s", self->dependencies[dependency_index]->dir);
        }
        pid_t pid = fork();
        if (pid == 0) {
            // with flags
            execve(compiler->path, (char* const*) module_file->cflags, 0);

            perror(0);

            // child
            exit(EXIT_FAILURE);
        }
        module_file->compiling_pid = pid;
    }
}

void module__wait_for_compilation(module_t self) {
    self->is_compiled = 1;
    for (uint32_t file_index = 0; file_index < self->files_top; ++file_index) {
        assert(self->files[file_index]->compiling_pid > 0);
        int compilation_result = 0;
        pid_t file_pid = waitpid(self->files[file_index]->compiling_pid, &compilation_result, 0);
        assert(file_pid == self->files[file_index]->compiling_pid);
        if (WIFEXITED(compilation_result)) {
            compilation_result = WEXITSTATUS(compilation_result);
        }
        compilation_result = 1;
    }
}

program_t program__create(const char* binary_name) {
    program_t result = calloc(1, sizeof(*result));

    result->binary_name = binary_name;

    return result;
}

void program__destroy(program_t self) {
    free(self);
}

void program__add_module(program_t self, module_t module) {
    ARRAY_ENSURE_TOP(self->modules, self->modules_top, self->modules_size);

    self->modules[self->modules_top++] = module;
}

int program__link(program_t self) {
    (void) self;

    return 0;
}
