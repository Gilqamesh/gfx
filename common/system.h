#ifndef SYSTEM_H
# define SYSTEM_H

# include <stdint.h>

# include "helper_macros.h"

PUBLIC_API void system__init();

PUBLIC_API void system__sleep(double s);
PUBLIC_API void system__usleep(double us);

// @returns returns time in seconds since system__init was called
PUBLIC_API double system__get_time();

#endif // SYSTEM_H
