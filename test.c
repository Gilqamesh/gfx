#include <stdio.h>
#include "system.h"

int main() {
    system__init();

    uint64_t start = system__get_tick();

    system__usleep(13);

    uint64_t end = system__get_tick();

    printf("Time passed: %lfus\n", (end - start) / 1000.0);

    return 0;
}
