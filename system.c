#include "system.h"
#include "helper_macros.h"

#include <assert.h>

int64_t       g_tick_resolution       = 0;
// the amount of clock cycles to query time on the platform
static double g_epsilon_overhead_tick = 0;

# if defined(WINDOWS)
#  include <windows.h>
# elif defined(LINUX) || defined(MAC)
#  include <unistd.h>
#  include <time.h>
# endif

static void system__platform_msleep(uint32_t ms);

static void system__platform_msleep(uint32_t ms) {
# if defined(WINDOWS)
    Sleep(ms);
# elif defined(LINUX) || defined(MAC)
    usleep(ms * 1000);
# endif
}

void system__init() {
# if defined(WINDOWS)
    static LARGE_INTEGER res;
    QueryPerformanceFrequency(&res);
    g_tick_resolution = (uint64_t) res.QuadPart;
# elif defined(LINUX) || defined(MAC)
    struct timespec res;
    clock_getres(CLOCK_REALTIME, &res);
    g_tick_resolution = (res.tv_sec * 1000000000 + res.tv_nsec) * 1000000000;
# endif

    // todo: What about context caching?
    const uint32_t number_of_samples = 100;
    for (uint32_t sample_index = 0; sample_index < number_of_samples; ++sample_index) {
        const uint32_t us_to_sleep = 15;
        const uint64_t start_tick = system__get_tick();
        system__usleep(us_to_sleep);
        const uint64_t end_tick = system__get_tick();
        const uint64_t expected_tick_delta = us_to_sleep * system__tick_resolution() / 1000000;
        const uint64_t actual_tick_delta = end_tick - start_tick;
        // note: running average
        g_epsilon_overhead_tick = (g_epsilon_overhead_tick * sample_index + (double) ((int64_t) actual_tick_delta - (int64_t) expected_tick_delta)) / (double) (sample_index + 1);
    }
}

void system__sleep(uint32_t s) {
    system__usleep(s * 1000 * 1000);
}

void system__usleep(uint64_t us) {
    const uint64_t end_tick = system__get_tick() + us * system__tick_resolution() / 1000000;

    uint64_t ms = us / 1000;
    const uint64_t ms_granularity = 100;
    if (ms > ms_granularity) {
        system__platform_msleep(ms - ms_granularity);
    }

    while (system__get_tick() + g_epsilon_overhead_tick < end_tick) { /* busy wait */ }
}

uint64_t system__tick_resolution() {
    return g_tick_resolution;
}

uint64_t system__get_tick() {
# if defined(WINDOWS)
    LARGE_INTEGER current_tick;
    QueryPerformanceCounter(&current_tick);
    return (uint64_t) current_tick.QuadPart;
# elif defined(LINUX) || defined(MAC)
    struct timespec start_time;
    clock_gettime(CLOCK_REALTIME, &start_time);
    return start_time.tv_sec * 1000000000 + start_time.tv_nsec;
# endif
}
