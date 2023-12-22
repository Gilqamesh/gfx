#include "build.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "helper_macros.h"

#include "build_driver_impl.c"

int main(int argc, char* argv[]) {
    build_driver__init();

    compiler_t compiler = compiler__create("/usr/bin/gcc");

    if (argc != 2) {
        fprintf(stderr, "usage: [build_driver_bin] <module_name>\n");
        fprintf(stderr, "module names:\n");
        for (uint32_t module_name_index = 0; module_name_index < ARRAY_SIZE(module_names); ++module_name_index) {
            fprintf(stderr, "  %s\n", module_names[module_name_index]);
        }
        exit(1);
    }

    const char* module = argv[1];
    module_wrapper_t* module_wrapper = build_driver__find_module_wrapper(module);
    if (!module_wrapper->is_compiled) {
        module_wrapper->compiler__compile_module(compiler, module_wrapper);
    } else {
        fprintf(stderr, "compiler for module '%s' is not supported\n", module);
    }

    build_driver__wait_for_compilations();

    compiler__destroy(compiler);

    return 0;
}
