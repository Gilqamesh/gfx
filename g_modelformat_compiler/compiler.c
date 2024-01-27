#include "compiler.h"

#include "file.h"
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

    while (
        !compiler__is_at_end(&compiler) &&
        !compiler.panic
    ) {
        compiler__emit_chunk(&compiler);
    }

    if (compiler.panic) {
        *result = 1;
    }

    (void) compiler__peak;
    (void) compiler__femit;

    return compiler.chunk;
}

str_builder_t decompile(const char* binary_source, size_t binary_source_len, int* result) {
    decompiler_t decompiler;
    decompiler__create(&decompiler, binary_source, binary_source_len);

    while (
        !decompiler__is_at_end(&decompiler) &&
        !decompiler.panic
    ) {
        decompiler__emit_chunk(&decompiler);
    }

    if (decompiler.panic) {
        *result = 1;
    }

    (void) decompiler__emit;
    (void) decompiler__patch;

    return decompiler.chunk;
}
