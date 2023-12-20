#include "system.h"
#include "helper_macros.h"

#include <assert.h>

static double g_tick_resolution           = 0;
// the amount of clock cycles to query time on the platform
static double g_get_time_epsilon_overhead = 0;
static double g_time_at_start             = 0;

# if defined(WINDOWS)
#  include <windows.h>
# elif defined(LINUX) || defined(MAC)
#  include <unistd.h>
#  include <time.h>
# endif

static void system__platform_msleep(uint32_t ms);
static uint64_t system__platform_get_tick();

static void system__platform_msleep(uint32_t ms) {
# if defined(WINDOWS)
    Sleep(ms);
# elif defined(LINUX) || defined(MAC)
    usleep(ms * 1000);
# endif
}

static uint64_t system__platform_get_tick() {
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

void system__init() {
# if defined(WINDOWS)
    static LARGE_INTEGER res;
    QueryPerformanceFrequency(&res);
    g_tick_resolution = (double) res.QuadPart * 100;
# elif defined(LINUX) || defined(MAC)
    struct timespec res;
    clock_getres(CLOCK_REALTIME, &res);
    g_tick_resolution = (double) res.tv_sec + res.tv_nsec / 1000000000.0;
# endif

    // todo: What about context caching?
    double get_time_epsilon_overhead = 0.0;
    const uint32_t number_of_samples = 100;
    uint32_t us_to_sleep = 1;
    for (uint32_t sample_index = 0; sample_index < number_of_samples; ++sample_index, us_to_sleep += 10) {
        const double time_start = system__get_time();
        system__usleep(us_to_sleep);
        const double time_end = system__get_time();
        const double time_expected = us_to_sleep / 1000000.0;
        const double time_actual = time_end - time_start;
        // note: running average
        get_time_epsilon_overhead = (get_time_epsilon_overhead * sample_index + (time_actual - time_expected)) / (double) (sample_index + 1);
    }
    g_get_time_epsilon_overhead = get_time_epsilon_overhead;

    g_time_at_start = system__get_time();
}

void system__sleep(double s) {
    system__usleep(s * 1000000.0);
}

void system__usleep(double us) {
    const double time_end = system__get_time() + us / 1000000.0;

    uint64_t ms = (uint64_t) us / 1000;
    const uint64_t ms_granularity = 100;
    if (ms > ms_granularity) {
        system__platform_msleep(ms - ms_granularity);
    }

    while (system__get_time() + g_get_time_epsilon_overhead < time_end) { /* busy wait */ }
}

double system__get_time() {
    return system__platform_get_tick() * g_tick_resolution - g_time_at_start;
}
