#include "build.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "helper_macros.h"

#include "build_driver_impl.c"

#define COMPILE_FLAG "-c"
#define LINK_FLAG    "-l"

const char* options[] = {
    COMPILE_FLAG,
    LINK_FLAG
};

int main(int argc, char* argv[]) {
    c_compiler = compiler__create("/usr/bin/gcc");
    cpp_compiler = compiler__create("/usr/bin/g++");

    if (argc < 2) {
        fprintf(stderr, "usage: build_driver_bin <module_name> [options]\n");
        fprintf(stderr, "module names:\n");
        for (uint32_t module_dir_index = 0; module_dir_index < ARRAY_SIZE(supported_modules); ++module_dir_index) {
            fprintf(stderr, "  %s\n", supported_modules[module_dir_index].dir);
        }
        fprintf(stderr, "options:\n");
        for (uint32_t option_index = 0; option_index < ARRAY_SIZE(options); ++option_index) {
            fprintf(stderr, "  %s\n", options[option_index]);
        }
        exit(1);
    }
    const char* module_dir = argv[1];
    const char* option = 0;
    if (argc > 2) {
        option = argv[2];
    }
    int32_t is_link_option = 0;
    if (
        !option ||
        strcmp(option, LINK_FLAG) == 0
    ) {
        is_link_option = 1;
    }

    supported_module_t* supported_module = 0;
    for (uint32_t module_dir_index = 0; module_dir_index < ARRAY_SIZE(supported_modules); ++module_dir_index) {
        if (strcmp(supported_modules[module_dir_index].dir, module_dir) == 0) {
            supported_module = &supported_modules[module_dir_index];
            break ;
        }
    }

    if (!supported_module) {
        fprintf(stderr, "compiler for module '%s' is not supported\n", module_dir);
        exit(1);
    }

    supported_module->module = module__create(module_dir, c_compiler);
    supported_module->is_link_option = is_link_option;
    supported_module__init_and_compile_wrapper(supported_module);

    module__wait_for_compilation(supported_module->module);

    if (is_link_option) {
        module__link(supported_module->module, c_compiler);
    }

    compiler__destroy(c_compiler);
    compiler__destroy(cpp_compiler);

    return 0;
}
