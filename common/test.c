#include <stdio.h>
#include "str_builder.h"

int main() {
    char buffer[17];
    str_builder_t str_builder;
    str_builder__create(&str_builder, buffer, sizeof(buffer));

    uint32_t bytes_written = 0;
    bytes_written = str_builder__append(&str_builder, "%f", 2.3f);
    printf("written: %u, str: %s, len: %u\n", bytes_written, str_builder__str(&str_builder), str_builder__len(&str_builder));
    bytes_written = str_builder__append(&str_builder, "%d", -2);
    printf("written: %u, str: %s, len: %u\n", bytes_written, str_builder__str(&str_builder), str_builder__len(&str_builder));
    bytes_written = str_builder__prepend(&str_builder, "%d", -2);
    printf("written: %u, str: %s, len: %u\n", bytes_written, str_builder__str(&str_builder), str_builder__len(&str_builder));
    bytes_written = str_builder__prepend(&str_builder, "%d", -2);
    printf("written: %u, str: %s, len: %u\n", bytes_written, str_builder__str(&str_builder), str_builder__len(&str_builder));
    bytes_written = str_builder__prepend(&str_builder, "%d", -2);
    printf("written: %u, str: %s, len: %u\n", bytes_written, str_builder__str(&str_builder), str_builder__len(&str_builder));

    return 0;
}