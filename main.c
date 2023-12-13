#include "app.h"
#include "debug.h"

int main(int argc, char* argv[]) {
    if (!debug__init_module()) {
        return false;
    }
    // debug__set_message_type_availability(DEBUG_INFO, false);

    app_t app = app__create(argc, argv);
    if (app) {
        app__run(app);
        app__destroy(app);
    }

    debug__deinit_module();

    return 0;
}
