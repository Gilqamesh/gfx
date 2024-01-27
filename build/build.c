#include "build.h"

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

#include "helper_macros.h"

extern char** environ;

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

module_t module__create(const char* dir, compiler_t compiler) {
    module_t result = calloc(1, sizeof(*result));

    result->dir = dir;
    result->compiler = compiler;
    ARRAY_ENSURE_TOP(result->lflags, result->lflags_top, result->lflags_size);
    result->lflags[result->lflags_top++] = 0;

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

    if (self->lflags) {
        for (uint32_t lflag_index = 0; lflag_index < self->lflags_size; ++lflag_index) {
            free(self->lflags[lflag_index]);
        }
        free(self->lflags);
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

void module__prepend_lflag(module_t self, const char* lflag_format, ...) {
    va_list ap;
    va_start(ap, lflag_format);

    module__vprepend_lflag(self, lflag_format, ap);

    va_end(ap);
}

void module__append_lflag(module_t self, const char* lflag_format, ...) {
    va_list ap;
    va_start(ap, lflag_format);

    module__vappend_lflag(self, lflag_format, ap);

    va_end(ap);
}

void module__add_dependency(module_t self, module_t dependency) {
    ARRAY_ENSURE_TOP(self->dependencies, self->dependencies_top, self->dependencies_size);

    self->dependencies[self->dependencies_top++] = dependency;
}

int32_t module__is_dependency(module_t self, module_t dependency) {
    for (uint32_t dependency_index = 0; dependency_index < self->dependencies_top; ++dependency_index) {
        if (self->dependencies[dependency_index] == dependency) {
            return 1;
        }
    }

    return 0;
}

void module__compile(module_t self) {
    if (self->is_compiled != 0) {
        // either compiling or compiled
        return ;
    }

    self->is_compiled = -1;
    for (uint32_t file_index = 0; file_index < self->files_top; ++file_index) {
        module_file_t module_file = self->files[file_index];
        if (module_file->compiling_pid != 0) {
            // either compiling or compiled
            continue ;
        }

        module_file__prepend_cflag(module_file, self->compiler->path);
        module_file__append_dependency_includes(module_file, self);
        for (uint32_t cflag_index = 0; cflag_index < module_file->cflags_top - 1; ++cflag_index) {
            printf("%s ", module_file->cflags[cflag_index]);
        }
        printf("\n");
        pid_t pid = fork();
        if (pid == -1) {
            perror(0);
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            execve(self->compiler->path, (char* const*) module_file->cflags, environ);
            perror(0);
            exit(EXIT_FAILURE);
        }

        module_file->compiling_pid = pid;
    }
}

int32_t module__is_compiled(module_t self) {
    return self->is_compiled;
}

void module__wait_for_compilation(module_t self) {
    if (module__is_compiled(self) > 0) {
        return ;
    }

    for (uint32_t dependency_index = 0; dependency_index < self->dependencies_top; ++dependency_index) {
        module_t dependency = self->dependencies[dependency_index];
        module__wait_for_compilation(dependency);
    }

    for (uint32_t file_index = 0; file_index < self->files_top; ++file_index) {
        module_file_t file = self->files[file_index];
        assert(file->compiling_pid > 0);
        int32_t compilation_result = 0;
        pid_t file_pid = waitpid(file->compiling_pid, &compilation_result, 0);
        assert(file_pid == file->compiling_pid);
        if (WIFEXITED(compilation_result)) {
            compilation_result = WEXITSTATUS(compilation_result);
        }
        file->compiling_pid = -1;
        compilation_result = 1;
    }
    self->is_compiled = 1;
}

int32_t module__link(module_t self, compiler_t compiler) {
    if (self->is_linked != 0) {
        // is linkining or linked
        return self->is_linked;
    }
    self->is_linked = -1;

    module_t fake_module = module__create(self->dir, compiler);
    module__append_lflag(fake_module, "%s", compiler->path);
    module__append_lflag(fake_module, "-o");
    // module__append_lflag(fake_module, "%s.out", self->dir);
    module__append_lflag(fake_module, "a.out", self->dir);
    module__collect_file_lflags(self, fake_module, 1);
    module__collect_file_lflags(self, fake_module, 0);
    module__collect_lib_lflags(self, fake_module, 1);
    module__collect_lib_lflags(self, fake_module, 0);

    for (uint32_t arg_index = 0; arg_index < fake_module->lflags_top - 1; ++arg_index) {
        printf("%s ", fake_module->lflags[arg_index]);
    }
    printf("\n");
    pid_t pid = fork();
    if (pid == -1) {
        exit(EXIT_FAILURE);
    }
    if (pid == 0) {
        execve(fake_module->lflags[0], (char* const*) fake_module->lflags, environ);
        perror(0);
        exit(EXIT_FAILURE);
    }
    int32_t compilation_result = -1;
    waitpid(pid, &compilation_result, 0);
    if (WIFEXITED(compilation_result)) {
        compilation_result = WEXITSTATUS(compilation_result);
    }

    self->is_linked = 1;

    module__destroy(fake_module);

    return compilation_result;
}
