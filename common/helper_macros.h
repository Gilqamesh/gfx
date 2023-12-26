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

# define MAX(x, y) ((int64_t)(x) < (int64_t)(y) ? (int64_t)(y) : (int64_t)(x))
# define MIN(x, y) ((int64_t)(x) < (int64_t)(y) ? (int64_t)(x) : (int64_t)(y))

# define BYTES(bytes)     (bytes)
# define KILOBYTES(bytes) (BYTES(bytes) * 1024LL)
# define MEGABYTES(bytes) (KILOBYTES(bytes) * 1024LL)
# define GIGABYTES(bytes) (MEGABYTES(bytes) * 1024LL)
# define TERABYTES(bytes) (GIGABYTES(bytes) * 1024LL)

// todo: create dynamic array module
#define ARRAY_ENSURE_TOP(array, array_top, array_size) do { \
    if ((array_top) >= (array_size)) { \
        if ((array_size) == 0) { \
            (array_size) = 8; \
            (array) = malloc((array_size) * sizeof((array)[0])); \
        } else { \
            (array_size) <<= 1; \
            (array) = realloc((array), (array_size) * sizeof((array)[0])); \
        } \
    } \
} while (0)

#endif // HELPER_MACROS_H
