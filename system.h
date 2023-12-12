#ifndef SYSTEM_H
# define SYSTEM_H

# include <stdint.h>

void system__init();

void system__sleep(uint32_t s);
void system__usleep(uint64_t us);

// @returns number of ticks per second measured by the platform
uint64_t system__tick_resolution();
uint64_t system__get_tick();

#endif // SYSTEM_H
