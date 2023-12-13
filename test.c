#include <stdio.h>
#include "system.h"

int main() {
    system__init();

    uint32_t us_to_sleep = 1;
    for (uint32_t i = 0; i < 100; ++i, us_to_sleep += 10) {
        double start = system__get_time();

        system__usleep(us_to_sleep);

        double end = system__get_time();

        const uint64_t expected = us_to_sleep;
        const double actual     = (end - start) * 1000000.0;
        printf("Expected: %uus, actual: %lfus, diff: %lf\n", expected, actual, expected - actual);
    }

    return 0;
}
