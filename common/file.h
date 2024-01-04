#ifndef FILE_H
# define FILE_H

# include <stdbool.h>
# include <time.h>
# include <stdarg.h>

struct         file;
enum           file_access_mode;
enum           file_creation_mode;
enum           file_type;
enum           file_seek_type;
typedef struct file               file_t;
typedef enum   file_access_mode   file_access_mode_t;
typedef enum   file_creation_mode file_creation_mode_t;
typedef enum   file_type          file_type_t;
typedef enum   file_seek_type     file_seek_type_t;

struct file {
    int fd;
};

enum file_access_mode {
    FILE_ACCESS_MODE_READ,  // open file in read only mode
    FILE_ACCESS_MODE_WRITE, // open file in write only mode
    FILE_ACCESS_MODE_RDWR   // open file in write and read mode
};

enum file_creation_mode {
    FILE_CREATION_MODE_OPEN,  // open existing file
    FILE_CREATION_MODE_CREATE // create new file or truncate if exists
};

// todo: differentiate from pipe/socket and other types of files
enum file_type {
    FILE_TYPE_DIRECTORY = 1 << 0,
    FILE_TYPE_FILE      = 1 << 1
};

enum file_seek_type {
    FILE_SEEK_TYPE_BEGIN,
    FILE_SEEK_TYPE_CUR,
    FILE_SEEK_TYPE_END
};

bool file__open(
    file_t* self,
    const char* file_path,
    file_access_mode_t access_mode,
    file_creation_mode_t creation_mode
);
void file__close(file_t* self);

// @brief creates an empty file on the specified path
// @note if the file existed on the path, it gets replaced
bool file__create(const char* path);
bool file__exists(const char* path);
bool file__delete(const char* path);
// @brief renames or moves a directory (including its children) if exists
bool file__move(const char* src_path, const char* dest_path);
// @returns last time that the file was modified
bool file__last_modified(const char* path, time_t* last_modified);
bool file__stat(const char* path, file_type_t* file_type);
// @returns whether the operations was successful or not as well as the file_size if it was
bool file__size(const char* path, size_t* file_size);
// @brief copies the source file to the destination path
// @note if dest exists, its contents will be overwritten
bool file__copy(const char* dest_path, const char* src_path);

// @brief reads from opened file, returns bytes read
bool file__read(file_t* self, void* out, size_t size, size_t* opt_read_bytes);
//! @returns bytes written
bool file__write(file_t* self, const void* in, size_t size, size_t* opt_written_bytes);
//! @returns bytes written excluding null-terminating character
bool file__fwrite(file_t* self, size_t* opt_written_bytes, const char* format, ...);
//! @returns bytes written excluding null-terminating character
bool file__vfwrite(file_t* self, size_t* opt_written_bytes, const char* format, va_list ap);
bool file__seek(file_t* self, size_t offset, file_seek_type_t seek_type, size_t* opt_file_pointer_position);

#endif
