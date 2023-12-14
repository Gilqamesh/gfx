#ifndef HELPER_MACROS_H
# define HELPER_MACROS_H

# define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))

# if defined (_WIN32) || defined (__CYGWIN__)
#  define WINDOWS
# elif defined(__linux__)
#  define LINUX
# elif defined(__APPLE__)
#  define MAC
# else
#  error "platform unsupported"
# endif

# define MAX(x, y) ((x) < (y) ? (y) : (x))
# define MIN(x, y) ((x) < (y) ? (x) : (y))

#endif // HELPER_MACROS_H
