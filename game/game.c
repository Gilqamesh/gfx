#include "game.h"

#include "debug.h"
#include "helper_macros.h"
#include "system.h"

#include <stdlib.h>

#include "game_internal.c"

game_t game__create() {
    game_t result = calloc(1, sizeof(*result));
    if (!result) {
        return 0;
    }

    // todo: game__update tests to measure the upper-bound for game__update_upper_bound?

    return result;
}

void game__destroy(game_t self) {
    free(self);
}

double game__update_upper_bound(game_t self) {
    (void) self;
    
    return 120.0 / 1000000.0;
}

void game__update(game_t self, double s) {

    system__usleep(100);

    (void) self;
    (void) s;
}
