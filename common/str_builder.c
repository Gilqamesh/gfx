#include "str_builder.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static size_t str_builder__size_left(str_builder_t* self);
static size_t str_builder__vfensure_size_left(str_builder_t* self, const char* format, va_list ap);
static void   str_builder__ensure_size_left(str_builder_t* self, size_t bytes_to_write);

static size_t str_builder__size_left(str_builder_t* self) {
    assert(self->cur <= self->end);
    return self->end - self->cur;
}

static size_t str_builder__vfensure_size_left(str_builder_t* self, const char* format, va_list ap) {
    va_list ap_cpy;
    va_copy(ap_cpy, ap);
    int32_t bytes_written = vsnprintf(0, 0, format, ap_cpy);
    va_end(ap_cpy);
    assert(bytes_written >= 0);

    str_builder__ensure_size_left(self, (size_t) bytes_written);
    return (size_t) bytes_written;
}

static void str_builder__ensure_size_left(str_builder_t* self, size_t bytes_to_write) {
    const size_t necessary_size = self->cur + bytes_to_write - self->start;
    const size_t old_size       = self->end - self->start;
    size_t new_size             = old_size;
    while (new_size <= necessary_size) {
        new_size <<= 1;
    }
    
    if (new_size != old_size) {
        const size_t old_cur = self->cur - self->start;
        self->start = realloc(self->start, new_size);
        self->end   = self->start + new_size;
        self->cur   = self->start + old_cur;
    }
}

void str_builder__create(str_builder_t* self) {
    const size_t memory_size = 1;
    self->start = malloc(memory_size);
    self->cur   = self->start;
    self->end   = self->start + memory_size;

    str_builder__clear(self);
}

void str_builder__destroy(str_builder_t* self) {
    free(self->start);
}

size_t str_builder__prepend(str_builder_t* self, const void* in, size_t in_size) {
    str_builder__ensure_size_left(self, in_size);

    const size_t old_str_len = str_builder__len(self);
    char tmp = *self->start;
    char* dst = self->start + in_size;
    char* src = self->start;
    size_t size = old_str_len + 1;
    assert(dst + size <= self->end);
    memmove(dst, src, size);

    memcpy(self->start, in, in_size);

    self->cur += in_size;
    
    *(self->start + in_size) = tmp;

    return in_size;
}

size_t str_builder__fprepend(str_builder_t* self, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    const size_t bytes_written = str_builder__vfprepend(self, format, ap);
    va_end(ap);

    return bytes_written;
}

size_t str_builder__vfprepend(str_builder_t* self, const char* format, va_list ap) {
    const size_t bytes_to_write = str_builder__vfensure_size_left(self, format, ap);

    const size_t old_str_len = str_builder__len(self);
    char tmp = *self->start;
    char* dst = self->start + bytes_to_write;
    char* src = self->start;
    size_t size = old_str_len + 1;
    assert(dst + size <= self->end);
    memmove(dst, src, size);

    if ((size_t) vsnprintf(self->start, str_builder__size_left(self), format, ap) != bytes_to_write) {
        assert(0);
    }
    self->cur += bytes_to_write;
    
    *(self->start + bytes_to_write) = tmp;

    return bytes_to_write;
}

size_t str_builder__append(str_builder_t* self, const void* in, size_t in_size) {
    str_builder__ensure_size_left(self, in_size);

    memcpy(self->cur, in, in_size);
    self->cur += in_size;
    *self->cur = '\0';

    return in_size;
}

size_t str_builder__fappend(str_builder_t* self, const char* format, ...) {
    // 1. [n1,n2][o1,o2,o3] -> [o1,o2,o3,n1,n2]
    va_list ap;
    va_start(ap, format);
    const size_t bytes_written = str_builder__vfappend(self, format, ap);
    va_end(ap);

    return bytes_written;
}

size_t str_builder__vfappend(str_builder_t* self, const char* format, va_list ap) {
    const size_t bytes_to_write = str_builder__vfensure_size_left(self, format, ap);

    if ((size_t) vsnprintf(self->cur, str_builder__size_left(self), format, ap) != bytes_to_write) {
        assert(0);
    }
    self->cur += bytes_to_write;

    return bytes_to_write;
}

void str_builder__patch(str_builder_t* self, size_t at, const void* in, size_t in_size) {
    assert(self->start + at + in_size < self->cur);
    memcpy(self->start + at, in, in_size);
}

const char* str_builder__str(str_builder_t* self) {
    assert(*self->cur == '\0');
    return self->start;
}

size_t str_builder__len(str_builder_t* self) {
    assert(self->start <= self->cur);
    return self->cur - self->start;
}

void str_builder__clear(str_builder_t* self) {
    self->cur = self->start;
    *self->cur = '\0';
}
