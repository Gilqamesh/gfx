#ifndef SYSTEM_H
# define SYSTEM_H

# include <stdint.h>

void system__init();

void system__sleep(uint32_t s);
void system__usleep(uint64_t us);

// @returns returns time in seconds
double system__get_time();

#endif // SYSTEM_H
