#include "buffer.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

bool buffer__create(buffer_t* self, uint32_t size) {
    assert(size > 0);

    self->start = malloc(size);
    if (!self->start) {
        return false;
    }

    self->cur = self->start;
    self->end = self->start + size;

    return true;
}

void buffer__destroy(buffer_t* self) {
    if (self->start) {
        free(self->start);
    }
}

void buffer__write(buffer_t* self, const char* format, ...) {
    va_list ap;
    va_start(ap, format);

    buffer__vwrite(self, format, ap);

    va_end(ap);
}

void buffer__vwrite(buffer_t* self, const char* format, va_list ap) {
    const uint32_t bytes_left = self->end - self->cur;
    const uint32_t bytes_written = vsnprintf((char*) self->cur, bytes_left, format, ap);
    if (bytes_written >= bytes_left) {
        fprintf(
            stderr,
            "Output to buffer was truncated, bytes left: %u, bytes written: %u\n",
            bytes_left,
            bytes_written
        );
    }
    self->cur += bytes_written;
}

void buffer__clear(buffer_t* self) {
    self->cur = self->start;
    *self->cur = '\0';
}
