#include <stdio.h>
#include <stdint.h>
#include "system.h"
#include "glfw.h"

#include <x86intrin.h>

static void system_api_test() {
    uint32_t us_to_sleep = 1;
    for (uint32_t i = 0; i < 100; ++i, us_to_sleep += 10) {
        double start = system__get_time();

        system__usleep(us_to_sleep);

        double end = system__get_time();

        const uint64_t expected = us_to_sleep;
        const double actual     = (end - start) * 1000000.0;
        printf("Expected: %luus, actual: %lfus, diff: %lf\n", expected, actual, expected - actual);
    }
}

static void compare_api_test() {
    uint64_t total_time_glfw_get_time = 0;
    uint64_t total_time_system_get_time = 0;
    const uint32_t number_of_iters = 100;
    for (uint32_t i = 0; i < number_of_iters; ++i) {
        uint64_t time_start = __rdtsc();
        glfw__get_time_s();
        uint64_t time_end   = __rdtsc();
        total_time_glfw_get_time += time_end - time_start;

        time_start = __rdtsc();
        system__get_time();
        time_end   = __rdtsc();
        total_time_system_get_time += time_end - time_start;
    }

    printf("On average, time taken:\n");
    printf("glfw__get_time_s: %lfCy\n", (double) total_time_glfw_get_time   / number_of_iters);
    printf("system__get_time: %lfCy\n", (double) total_time_system_get_time / number_of_iters);
}

int main() {
    system__init();
    
    compare_api_test();

    system_api_test();

    return 0;
}
