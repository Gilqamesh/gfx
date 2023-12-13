#ifndef SYSTEM_H
# define SYSTEM_H

# include <stdint.h>

void system__init();

void system__sleep(double s);
void system__usleep(double us);

// @returns returns time in seconds
double system__get_time();

#endif // SYSTEM_H
