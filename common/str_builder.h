#ifndef STR_BUILDER_H
# define STR_BUILDER_H

# include <stdarg.h>
# include <stdint.h>
# include <stdbool.h>

struct         str_builder;
typedef struct str_builder str_builder_t;

struct str_builder {
    char* start;
    char* cur;
    char* end;
};

void str_builder__create(str_builder_t* self, void* memory, uint32_t memory_size);

/**
 * Does not do anything if there isn't enough size for the prepended string
*/
uint32_t str_builder__prepend(str_builder_t* self, const char* format, ...);
uint32_t str_builder__append(str_builder_t* self, const char* format, ...);
uint32_t str_builder__vprepend(str_builder_t* self, const char* format, va_list ap);
uint32_t str_builder__vappend(str_builder_t* self, const char* format, va_list ap);

/**
 * @returns Null-terminated string
*/
const char* str_builder__str(str_builder_t* self);
uint32_t str_builder__len(str_builder_t* self);
void str_builder__clear(str_builder_t* self);

#endif // STR_BUILDER_H
