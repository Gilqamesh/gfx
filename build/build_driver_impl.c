struct         supported_module;
enum           supported_module_name;
typedef struct supported_module      supported_module_t;
typedef enum   supported_module_name supported_module_name_t;

enum supported_module_name {
    COMMON_MODULE,
    DEBUG_MODULE,
    TP_MODULE,
    GAME_SERVER_MODULE,
    GAME_CLIENT_MODULE,
    SYSTEM_MODULE,
    GAME_MODULE,
    GFX_MODULE,
    G_MODELFORMAT_COMPILER_MODULE,

    _SUPPORTED_MODULE_NAME_SIZE
};

struct supported_module {
    module_t module;
    const char* dir;
    int32_t is_link_option;
    void (*supported_module__init_and_compile)(supported_module_t* self);
};

static supported_module_t* supported_modules__ensure_module(supported_module_name_t supported_module_name);

static void supported_module__init_and_compile_wrapper(supported_module_t* self);
static void module__add_supported_dependency(module_t self, supported_module_name_t supported_dependency_name);

static void module_file__add_common_cflags(module_file_t module_file);
static void module_file__add_debug_cflags(module_file_t module_file);
static void module_file__add_release_cflags(module_file_t module_file);

static void supported_module__init_common_module(supported_module_t* self);
static void supported_module__init_debug_module(supported_module_t* self);
static void supported_module__init_game_server_module(supported_module_t* self);
static void supported_module__init_game_client_module(supported_module_t* self);
static void supported_module__init_transport_protocol_module(supported_module_t* self);
static void supported_module__init_system_module(supported_module_t* self);
static void supported_module__init_game_module(supported_module_t* self);
static void supported_module__init_gfx_module(supported_module_t* self);
static void supported_module__init_g_modelformat_compiler_module(supported_module_t* self);

static compiler_t compiler;

static supported_module_t supported_modules[_SUPPORTED_MODULE_NAME_SIZE] = {
    {
        .dir = "common",
        .supported_module__init_and_compile = &supported_module__init_common_module
    },
    {
        .dir = "debug",
        .supported_module__init_and_compile = &supported_module__init_debug_module
    },
    {
        .dir = "transport_protocol",
        .supported_module__init_and_compile = &supported_module__init_transport_protocol_module
    },
    {
        .dir = "game_server",
        .supported_module__init_and_compile = &supported_module__init_game_server_module
    },
    {
        .dir = "game_client",
        .supported_module__init_and_compile = &supported_module__init_game_client_module
    },
    {
        .dir = "system",
        .supported_module__init_and_compile = &supported_module__init_system_module
    },
    {
        .dir = "game",
        .supported_module__init_and_compile = &supported_module__init_game_module
    },
    {
        .dir = "gfx",
        .supported_module__init_and_compile = &supported_module__init_gfx_module
    },
    {
        .dir = "g_modelformat_compiler",
        .supported_module__init_and_compile = &supported_module__init_g_modelformat_compiler_module
    }
};

static supported_module_t* supported_modules__ensure_module(supported_module_name_t supported_module_name) {
    assert(supported_module_name < _SUPPORTED_MODULE_NAME_SIZE);
    supported_module_t* supported_module = &supported_modules[supported_module_name];
    if (!supported_module->module) {
        supported_module->module = module__create(supported_module->dir);
    }
    assert(supported_module->module);

    return supported_module;
}

static void module_file__add_common_cflags(module_file_t self) {
    module_file__append_cflag(self, "-Wall");
    module_file__append_cflag(self, "-Wextra");
    module_file__append_cflag(self, "-Werror");
    module_file__append_cflag(self, "-Icommon");
}

static void module_file__add_debug_cflags(module_file_t self) {
    module_file__append_cflag(self, "-DDEBUG");
    module_file__append_cflag(self, "-g");
    module_file__append_cflag(self, "-O0");
}

static void module_file__add_release_cflags(module_file_t self) {
    module_file__append_cflag(self, "-DRELEASE");
    module_file__append_cflag(self, "-O3");
    module_file__append_cflag(self, "-Wno-unused-function");
}

static void supported_module__init_common_module(supported_module_t* self) {
    module_file_t hash_set_file = module__add_file(self->module, "hash_set.c");

    module_file__add_common_cflags(hash_set_file);
    module_file__add_debug_cflags(hash_set_file);
    
    module_file_t hash_map_file = module__add_file(self->module, "hash_map.c");
    module_file__add_common_cflags(hash_map_file);
    module_file__add_debug_cflags(hash_map_file);

    module_file_t str_builder_file = module__add_file(self->module, "str_builder.c");
    module_file__add_common_cflags(str_builder_file);
    module_file__add_debug_cflags(str_builder_file);

    module_file_t file_file = module__add_file(self->module, "file.c");
    module_file__add_common_cflags(file_file);
    module_file__add_debug_cflags(file_file);

    module_file_t libc_file = module__add_file(self->module, "libc.c");
    module_file__add_common_cflags(libc_file);
    module_file__add_debug_cflags(libc_file);

    module__append_lflag(self->module, "-lm");

    (void) module_file__add_release_cflags;

    // TODO: add module level cflags that needs to apply to modules who depend on this module
//     module_file__append_cflag(self->module, "-Wall");
//     module_file__append_cflag(self->module, "-Wextra");
//     module_file__append_cflag(self->module, "-Werror");
//     module_file__append_cflag(self->module, "-Icommon");

// #if defined(DEBUG)
//     module_file__append_cflag(self->module, "-DDEBUG");
//     module_file__append_cflag(self->module, "-g");
//     module_file__append_cflag(self->module, "-O0");
// #elif defined(RELEASE)
//     module_file__append_cflag(self->module, "-DRELEASE");
//     module_file__append_cflag(self->module, "-O3");
//     module_file__append_cflag(self->module, "-Wno-unused-function");
// #else
// # error "build mode can either be debug or release"
// #endif
}

static void supported_module__init_debug_module(supported_module_t* self) {
    module_file_t debug_file = module__add_file(self->module, "debug.c");

    module_file__add_common_cflags(debug_file);
    module_file__add_debug_cflags(debug_file);

    module__add_supported_dependency(self->module, COMMON_MODULE);
}

static void supported_module__init_game_server_module(supported_module_t* self) {
    module_file_t game_server_file = module__add_file(self->module, "game_server.c");
    module_file__add_common_cflags(game_server_file);
    module_file__add_debug_cflags(game_server_file);

    if (self->is_link_option) {
        module_file_t game_server_driver_file = module__add_file(self->module, "game_server_driver.c");
        module_file__add_common_cflags(game_server_driver_file);
        module_file__add_debug_cflags(game_server_driver_file);
    }

    module__add_supported_dependency(self->module, TP_MODULE);
    module__add_supported_dependency(self->module, SYSTEM_MODULE);
    module__add_supported_dependency(self->module, DEBUG_MODULE);
    module__add_supported_dependency(self->module, GAME_MODULE);
}

static void supported_module__init_game_client_module(supported_module_t* self) {
    module_file_t game_client_file = module__add_file(self->module, "game_client.c");
    module_file__add_common_cflags(game_client_file);
    module_file__add_debug_cflags(game_client_file);

    if (self->is_link_option) {
        module_file_t game_client_driver_file = module__add_file(self->module, "game_client_driver.c");
        module_file__add_common_cflags(game_client_driver_file);
        module_file__add_debug_cflags(game_client_driver_file);
    }

    module__add_supported_dependency(self->module, TP_MODULE);
    module__add_supported_dependency(self->module, SYSTEM_MODULE);
    module__add_supported_dependency(self->module, DEBUG_MODULE);
    module__add_supported_dependency(self->module, GAME_MODULE);
    module__add_supported_dependency(self->module, GFX_MODULE);
}

static void supported_module__init_transport_protocol_module(supported_module_t* self) {
    module_file_t tp_file = module__add_file(self->module, "tp.c");

    module_file__add_common_cflags(tp_file);
    module_file__add_debug_cflags(tp_file);

    module_file_t packet_file = module__add_file(self->module, "packet.c");

    module_file__add_common_cflags(packet_file);
    module_file__add_debug_cflags(packet_file);

    module__add_supported_dependency(self->module, DEBUG_MODULE);
}

static void supported_module__init_system_module(supported_module_t* self) {
    module_file_t system_file = module__add_file(self->module, "system.c");

    module_file__add_common_cflags(system_file);
    module_file__add_debug_cflags(system_file);

    module_file_t thread_file = module__add_file(self->module, "thread.c");

    module_file__add_common_cflags(thread_file);
    module_file__add_debug_cflags(thread_file);
}

static void supported_module__init_game_module(supported_module_t* self) {
    module_file_t game_file = module__add_file(self->module, "game.c");

    module_file__add_common_cflags(game_file);
    module_file__add_debug_cflags(game_file);

    module__append_lflag(self->module, "-lm");

    module__add_supported_dependency(self->module, DEBUG_MODULE);
    module__add_supported_dependency(self->module, SYSTEM_MODULE);
    module__add_supported_dependency(self->module, GFX_MODULE);
}

static void supported_module__init_gfx_module(supported_module_t* self) {
    module_file_t gfx_file = module__add_file(self->module, "gfx.c");

    module_file__add_common_cflags(gfx_file);
    module_file__add_debug_cflags(gfx_file);

#if defined(VULKAN)
    // module_file__append_cflag(gfx_file, "-DVULKAN");

#elif defined(OPENGL)
    // module_file__append_cflag(gfx_file, "-DOPENGL");
    module_file_t glad_file = module__add_file(self->module, "gl/glad/src/glad.c");
    
    module_file__add_common_cflags(glad_file);
    module_file__add_debug_cflags(glad_file);

    module_file__append_cflag(glad_file, "-Igfx/gl/glad/include");
#else
# error "graphics backend must either be defined to "opengl" or "vulkan""
#endif

    module_file__append_cflag(gfx_file, GLFW_CFLAGS);
    module__append_lflag(self->module, GLFW_LFLAGS);

    module_file__append_cflag(gfx_file, GFX_BACKEND_CFLAGS);
    module__append_lflag(self->module, GFX_BACKEND_LFLAGS);

    module__add_supported_dependency(self->module, DEBUG_MODULE);
}

static void supported_module__init_g_modelformat_compiler_module(supported_module_t* self) {
    module_file_t compiler_file = module__add_file(self->module, "compiler.c");

    module_file__add_common_cflags(compiler_file);
    module_file__add_debug_cflags(compiler_file);

    module_file_t scan_file = module__add_file(self->module, "scan.c");

    module_file__add_common_cflags(scan_file);
    module_file__add_debug_cflags(scan_file);

    if (self->is_link_option) {
        module_file_t driver_file = module__add_file(self->module, "driver.c");
        module_file__add_common_cflags(driver_file);
        module_file__add_debug_cflags(driver_file);
    }

    module__add_supported_dependency(self->module, COMMON_MODULE);
}

static void supported_module__init_and_compile_wrapper(supported_module_t* self) {
    if (module__is_compiled(self->module)) {
        return ;
    }

    self->supported_module__init_and_compile(self);

    module__compile(self->module, compiler);
}

static void module__add_supported_dependency(module_t self, supported_module_name_t supported_dependency_name) {
    supported_module_t* supported_dependency = supported_modules__ensure_module(supported_dependency_name);
    if (module__is_dependency(self, supported_dependency->module)) {
        return ;
    }
    module__add_dependency(self, supported_dependency->module);
    supported_module__init_and_compile_wrapper(supported_dependency);
}
