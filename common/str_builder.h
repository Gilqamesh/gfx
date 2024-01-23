#ifndef STR_BUILDER_H
# define STR_BUILDER_H

# include <stdarg.h>
# include <stddef.h>

struct         str_builder;
typedef struct str_builder str_builder_t;

struct str_builder {
    char* start;
    char* cur;
    char* end;

    int is_static;
};

void str_builder__create_static(str_builder_t* self, void* memory, size_t memory_size);
void str_builder__create(str_builder_t* self);
void str_builder__destroy(str_builder_t* self);

size_t str_builder__prepend(str_builder_t* self, const void* in, size_t in_size);
size_t str_builder__fprepend(str_builder_t* self, const char* format, ...);
size_t str_builder__vfprepend(str_builder_t* self, const char* format, va_list ap);

size_t str_builder__append(str_builder_t* self, const void* in, size_t in_size);
size_t str_builder__fappend(str_builder_t* self, const char* format, ...);
size_t str_builder__vfappend(str_builder_t* self, const char* format, va_list ap);

void str_builder__patch(str_builder_t* self, size_t at, const void* in, size_t in_size);

//! @returns null-terminated string
char* str_builder__str(str_builder_t* self);
size_t str_builder__len(str_builder_t* self);
void str_builder__clear(str_builder_t* self);

#endif // STR_BUILDER_H
