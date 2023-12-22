#ifndef BUILD_H
# define BUILD_H

/********************************************************************************
 * Compiler API
 ********************************************************************************/

# include <stdarg.h>

struct         compiler;
typedef struct compiler* compiler_t;

compiler_t compiler__create(const char* path);
void compiler__destroy(compiler_t self);

/********************************************************************************
 * Module API
 ********************************************************************************/

struct         module;
struct         module_file;
typedef struct module*      module_t;
typedef struct module_file* module_file_t;

module_t module__create(const char* dir);
void module__destroy(module_t self);
const char* module__dir(module_t self);
module_file_t module__add_file(module_t self, const char* src);
void module__add_dependency(module_t self, module_t dependency);
void module__compile(module_t self, compiler_t compiler);
void module__wait_for_compilation(module_t self);

void module_file__prepend_cflag(module_file_t self, const char* cflag_format, ...);
void module_file__append_cflag(module_file_t self, const char* cflag_format, ...);
void module_file__prepend_lflag(module_file_t self, const char* lflag_format, ...);
void module_file__append_lflag(module_file_t self, const char* lflag_format, ...);

/********************************************************************************
 * Program API
 ********************************************************************************/

struct         program;
typedef struct program* program_t;

program_t program__create(const char* binary_name);
void program__destroy(program_t self);
void program__add_module(program_t self, module_t module);
int program__link(program_t self);

#endif // BUILD_H
