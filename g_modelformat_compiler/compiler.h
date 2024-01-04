#ifndef COMPILER_H
# define COMPILER_H

# include "str_builder.h"

str_builder_t compile(const char* ascii_source, size_t ascii_source_len, int* result);
str_builder_t decompile(const char* binary_source, size_t binary_source_len, int* result);

#endif // COMPILER_H
