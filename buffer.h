#ifndef BUFFER_H
# define BUFFER_H

# include <stdint.h>
# include <stdbool.h>
# include <stdarg.h>

typedef struct buffer {
    uint8_t* cur;
    uint8_t* start;
    uint8_t* end;
} buffer_t;

bool buffer__create(buffer_t* self, uint32_t size);
void buffer__destroy(buffer_t* self);

void buffer__write(buffer_t* self, const char* format, ...);
void buffer__vwrite(buffer_t* self, const char* format, va_list ap);

void buffer__clear(buffer_t* self);

#endif // BUFFER_H
