#include "compiler.h"

#include "file.h"
#include "libc.h"
#include "vec_math.h"
#include "helper_macros.h"
#include "scan.h"

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "compiler_impl.c"

str_builder_t compile(const char* ascii_source, size_t ascii_source_len, int* result) {
    *result = 0;

    compiler_t compiler;
    compiler__create(&compiler, ascii_source, ascii_source_len);
    compiler__eat(&compiler);


    if (compiler__eat_err(&compiler, TOKEN_VERSION, "expected version token")) {
        *result = 1;
        return compiler.chunk;
    }

    token_t token = compiler__eat(&compiler);
    if (token.type != TOKEN_NUMBER) {
        compiler__err(&compiler, "expected version number");
        *result = 1;
        return compiler.chunk;
    }
    float version = (float) strntod(token.lexeme, token.lexeme_len);
    file_header_t file_header = {
        .version_major = (uint32_t) version,
        .version_minor = (uint32_t) ((version - (float) (uint32_t) version) * 100.0f),
        .n_of_chunks = 0,
        .header_size = sizeof(file_header)
    };
    compiler__emit(&compiler, &file_header, sizeof(file_header));

    while (!compiler__is_at_end(&compiler)) {
        compiler__emit_chunk(&compiler);
        ++file_header.n_of_chunks;
        if (compiler.panic) {
            *result = 1;
            return compiler.chunk;
        }
    }

    compiler__patch(&compiler, 0, &file_header, sizeof(file_header));

    (void) compiler__femit;

    return compiler.chunk;
}

str_builder_t decompile(const char* binary_source, size_t binary_source_len, int* result) {
    decompiler_t decompiler;
    decompiler__create(&decompiler, binary_source, binary_source_len);

    const file_header_t* file_header = (const file_header_t*) decompiler__eat(&decompiler, sizeof(*file_header));
    if (!file_header) {
        decompiler__err(&decompiler, "expected file header");
        *result = 1;
        return decompiler.chunk;
    }

    decompiler__femit(&decompiler, "version %u.%u\n", file_header->version_major, file_header->version_minor);

    uint32_t chunks_left = file_header->n_of_chunks;
    while (
        chunks_left-- > 0 &&
        !decompiler__is_at_end(&decompiler)
    ) {
        decompiler__emit_chunk(&decompiler);
        if (decompiler.panic) {
            *result = 1;
            return decompiler.chunk;
        }
    }

    (void) decompiler__emit;
    (void) decompiler__patch;

    return decompiler.chunk;
}
