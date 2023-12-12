#include "app.h"
#include "debug.h"

int main(int argc, char* argv[]) {
    if (!debug__init_module()) {
        return false;
    }
    // debug__set_message_type_availability(DEBUG_INFO, false);

    app_t app;
    if (app__create(&app, argc, argv)) {
        app__run(&app);
        app__destroy(&app);
    }

    debug__deinit_module();

    return 0;
}
