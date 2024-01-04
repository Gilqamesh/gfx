#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "file.h"
#include "str_builder.h"
#include "helper_macros.h"

static int compile_or_decompile_file(const char* dst_path, const char* src_path, str_builder_t (*compiler)(const char* source, size_t source_len, int* result));

static int compile_or_decompile_file(const char* dst_path, const char* src_path, str_builder_t (*compiler)(const char* source, size_t source_len, int* result)) {
    size_t src_file_size = 0;
    if (!file__size(src_path, &src_file_size)) {
        return 1;
    }

    file_t src_file;
    if (!file__open(&src_file, src_path, FILE_ACCESS_MODE_READ, FILE_CREATION_MODE_OPEN)) {
        return 1;
    }

    char* src_buffer = malloc(src_file_size + 1);
    size_t bytes_read = 0;
    if (!file__read(&src_file, src_buffer, src_file_size, &bytes_read)) {
        file__close(&src_file);
        free(src_buffer);
        return 1;
    }
    assert(bytes_read <= src_file_size);
    src_buffer[bytes_read] = '\0';

    file_t dst_file;
    if (!file__open(&dst_file, dst_path, FILE_ACCESS_MODE_WRITE, FILE_CREATION_MODE_CREATE)) {
        file__close(&src_file);
        free(src_buffer);
        return 1;
    }

    int result;
    str_builder_t str = compiler(src_buffer, bytes_read, &result);
    file__write(&dst_file, str_builder__str(&str), str_builder__len(&str), 0);

    file__close(&dst_file);
    file__close(&src_file);
    free(src_buffer);
    str_builder__destroy(&str);

    return result;
}

#define COMPILE_FLAG   "-c"
#define DECOMPILE_FLAG "-d"

const char* options[] = {
    COMPILE_FLAG,
    DECOMPILE_FLAG
};

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "usage: bin <dst_file> <src_file> <option> \n");
        fprintf(stderr, "options:\n");
        for (size_t option_index = 0; option_index < ARRAY_SIZE(options); ++option_index) {
            fprintf(stderr, "  %s\n", options[option_index]);
        }
        return 1;
    }

    const char* option = argv[3];
    const char* dst_file = argv[1];
    const char* src_file = argv[2];
    int result = 0;
    if (strcmp(option, COMPILE_FLAG) == 0) {
        result = compile_or_decompile_file(dst_file, src_file, &compile);
    } else if (strcmp(option, DECOMPILE_FLAG) == 0) {
        result = compile_or_decompile_file(dst_file, src_file, &decompile);
    } else {
        assert(false);
    }

    if (!result) {
        fprintf(stdout, "success\n");
    }

    return result;
}
