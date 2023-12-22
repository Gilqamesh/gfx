#define DEBUG_MODULE        "debug"
#define TP_MODULE           "transport_protocol"
#define GAME_SERVER_MODULE  "game_server"
#define SYSTEM_MODULE       "system"
#define GAME_MODULE         "game"

const char* module_names[] = {
    DEBUG_MODULE,
    TP_MODULE,
    GAME_SERVER_MODULE,
    SYSTEM_MODULE,
    GAME_MODULE
};

typedef struct module_wrapper {
    module_t module;
    void    (*compiler__compile_module)(compiler_t self, struct module_wrapper* module_wrapper);

    /**
     * 0  - hasn't been compiled yet
     * >0 - compiled
    */
    int     is_compiled;
} module_wrapper_t;

typedef struct build_driver {
    // todo: change this to hash map for quick find lookup
    uint32_t          module_wrappers_size;
    uint32_t          module_wrappers_top;
    module_wrapper_t* module_wrappers;
} build_driver_t;

static build_driver_t build_driver;

static void              build_driver__init();
static void              build_driver__add_module_wrapper(const char* module_dir, void (*compiler__compile_module)(compiler_t self, module_wrapper_t* module_wrapper));
static module_wrapper_t* build_driver__find_module_wrapper(const char* module_dir);
static void              build_driver__wait_for_compilations();

static void module_file__add_common_cflags(module_file_t module_file);
static void module_file__add_debug_cflags(module_file_t module_file);
static void module_file__add_release_cflags(module_file_t module_file);

static void compiler__ensure_module_dependency(compiler_t self, module_wrapper_t* module_wrapper, const char* dependency_module_dir);
static void compiler__compile_debug_module(compiler_t self, module_wrapper_t* module_wrapper);
static void compiler__compile_game_server_module(compiler_t self, module_wrapper_t* module_wrapper);
static void compiler__compile_transport_protocol_module(compiler_t self, module_wrapper_t* module_wrapper);
static void compiler__compile_system_module(compiler_t self, module_wrapper_t* module_wrapper);
static void compiler__compile_game_module(compiler_t self, module_wrapper_t* module_wrapper);

static void build_driver__init() {
    build_driver__add_module_wrapper(DEBUG_MODULE, &compiler__compile_debug_module);
    build_driver__add_module_wrapper(TP_MODULE, &compiler__compile_transport_protocol_module);
    build_driver__add_module_wrapper(GAME_SERVER_MODULE, &compiler__compile_game_server_module);
    build_driver__add_module_wrapper(SYSTEM_MODULE, &compiler__compile_system_module);
    build_driver__add_module_wrapper(GAME_MODULE, &compiler__compile_game_module);
}

static void build_driver__add_module_wrapper(const char* module_dir, void (*compiler__compile_module)(compiler_t self, module_wrapper_t* module_wrapper)) {
    ARRAY_ENSURE_TOP(build_driver.module_wrappers, build_driver.module_wrappers_top, build_driver.module_wrappers_size);
    module_wrapper_t* module_wrapper = &build_driver.module_wrappers[build_driver.module_wrappers_top];
    module_wrapper->module = module__create(module_dir);
    module_wrapper->compiler__compile_module = compiler__compile_module;
    ++build_driver.module_wrappers_top;
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

static void compiler__compile_debug_module(compiler_t self, module_wrapper_t* module_wrapper) {
    module_file_t debug_file = module__add_file(module_wrapper->module, "debug.c");

    module_file__add_common_cflags(debug_file);
    module_file__add_debug_cflags(debug_file);
    (void) module_file__add_release_cflags;

    module__compile(module_wrapper->module, self);
}

static void compiler__compile_game_server_module(compiler_t self, module_wrapper_t* module_wrapper) {
    module_file_t game_server_file = module__add_file(module_wrapper->module, "game_server.c");
    module_file__add_common_cflags(game_server_file);
    module_file__add_debug_cflags(game_server_file);

    module_file_t game_server_driver_file = module__add_file(module_wrapper->module, "game_server_driver.c");
    module_file__add_common_cflags(game_server_driver_file);
    module_file__add_debug_cflags(game_server_driver_file);

    compiler__ensure_module_dependency(self, module_wrapper, TP_MODULE);
    compiler__ensure_module_dependency(self, module_wrapper, SYSTEM_MODULE);
    compiler__ensure_module_dependency(self, module_wrapper, DEBUG_MODULE);
    compiler__ensure_module_dependency(self, module_wrapper, GAME_MODULE);

    module__compile(module_wrapper->module, self);
}

static void compiler__compile_transport_protocol_module(compiler_t self, module_wrapper_t* module_wrapper) {
    module_file_t udp_tp_file = module__add_file(module_wrapper->module, "udp_protocol.c");

    module_file__add_common_cflags(udp_tp_file);
    module_file__add_debug_cflags(udp_tp_file);

    module__compile(module_wrapper->module, self);
}

static void compiler__compile_system_module(compiler_t self, module_wrapper_t* module_wrapper) {
    module_file_t system_file = module__add_file(module_wrapper->module, "system.c");

    module_file__add_common_cflags(system_file);
    module_file__add_debug_cflags(system_file);

    module_file_t thread_file = module__add_file(module_wrapper->module, "thread.c");

    module_file__add_common_cflags(thread_file);
    module_file__add_debug_cflags(thread_file);

    module__compile(module_wrapper->module, self);
}

static void compiler__compile_game_module(compiler_t self, module_wrapper_t* module_wrapper) {
    module_file_t game_file = module__add_file(module_wrapper->module, "game.c");

    module_file__add_common_cflags(game_file);
    module_file__add_debug_cflags(game_file);

    module__compile(module_wrapper->module, self);
}

static void build_driver__wait_for_compilations() {
    for (uint32_t module_wrapper_index = 0; module_wrapper_index < build_driver.module_wrappers_top; ++module_wrapper_index) {
        module__wait_for_compilation(build_driver.module_wrappers[module_wrapper_index].module);
    }
}

static module_wrapper_t* build_driver__find_module_wrapper(const char* module_dir) {
    for (uint32_t module_wrapper_index = 0; module_wrapper_index < build_driver.module_wrappers_top; ++module_wrapper_index) {
        module_wrapper_t* module_wrapper = &build_driver.module_wrappers[module_wrapper_index];
        if (strcmp(module_dir, module__dir(module_wrapper->module)) == 0) {
            return module_wrapper;
        }
    }

    return 0;
}

static void compiler__ensure_module_dependency(compiler_t self, module_wrapper_t* module_wrapper, const char* dependency_module_dir) {
    module_wrapper_t* dependency_wrapper = build_driver__find_module_wrapper(dependency_module_dir);
    assert(dependency_wrapper);
    if (!dependency_wrapper->is_compiled) {
        dependency_wrapper->compiler__compile_module(self, dependency_wrapper);
    }

    module__add_dependency(module_wrapper->module, dependency_wrapper->module);
}
