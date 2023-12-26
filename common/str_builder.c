#include "str_builder.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static uint32_t str_builder__size_left(str_builder_t* self);

static uint32_t str_builder__size_left(str_builder_t* self) {
    assert(self->cur <= self->end);
    return self->end - self->cur;
}

void str_builder__create(str_builder_t* self, void* memory, uint32_t memory_size) {
    assert(memory_size > 0);
    self->start = (char*) memory;
    self->cur   = (char*) memory;
    self->end   = (char*) memory + memory_size;

    str_builder__clear(self);
}

uint32_t str_builder__prepend(str_builder_t* self, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    const uint32_t bytes_written = str_builder__vprepend(self, format, ap);
    va_end(ap);

    return bytes_written;
}

uint32_t str_builder__append(str_builder_t* self, const char* format, ...) {
    // 1. [n1,n2][o1,o2,o3] -> [o1,o2,o3,n1,n2]
    va_list ap;
    va_start(ap, format);
    const uint32_t bytes_written = str_builder__vappend(self, format, ap);
    va_end(ap);

    return bytes_written;
}

uint32_t str_builder__vprepend(str_builder_t* self, const char* format, va_list ap) {
    va_list ap_cpy;
    va_copy(ap_cpy, ap);
    int32_t bytes_written = vsnprintf(0, 0, format, ap_cpy);
    va_end(ap_cpy);
    assert(bytes_written >= 0);

    const uint32_t size_left = str_builder__size_left(self);
    if (size_left <= (uint32_t) bytes_written) {
        return 0;
    }

    const uint32_t old_str_len = str_builder__len(self);
    char tmp = *self->start;
    char* dst = self->start + bytes_written;
    char* src = self->start;
    size_t size = old_str_len + 1;
    assert(dst + size <= self->end);
    memmove(dst, src, size);

    if (vsnprintf(self->start, size_left, format, ap) != bytes_written) {
        assert(false);
    }
    *(self->start + bytes_written) = tmp;

    self->cur += bytes_written;

    return bytes_written;
}

uint32_t str_builder__vappend(str_builder_t* self, const char* format, va_list ap) {
    const uint32_t old_size_left = str_builder__size_left(self);
    int32_t bytes_written = vsnprintf(self->cur, old_size_left, format, ap);
    assert(bytes_written >= 0);
    self->cur += bytes_written;
    if (self->cur >= self->end) {
        self->cur = self->end - 1;
    }
    const uint32_t new_size_left = str_builder__size_left(self);

    return old_size_left - new_size_left;
}

const char* str_builder__str(str_builder_t* self) {
    assert(*self->cur == '\0');
    return self->start;
}

uint32_t str_builder__len(str_builder_t* self) {
    assert(self->start <= self->cur);
    return self->cur - self->start;
}

void str_builder__clear(str_builder_t* self) {
    self->cur = self->start;
    *self->cur = '\0';
}
