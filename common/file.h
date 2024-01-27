#ifndef FILE_H
# define FILE_H

# include <stdbool.h>
# include <time.h>
# include <stdarg.h>
# include <dirent.h>

# include "helper_macros.h"

typedef struct file {
    int fd;
} file_t;

typedef enum file_access_mode {
    FILE_ACCESS_MODE_READ,  // open file in read only mode
    FILE_ACCESS_MODE_WRITE, // open file in write only mode
    FILE_ACCESS_MODE_RDWR   // open file in write and read mode
} file_access_mode_t;

typedef enum file_creation_mode {
    FILE_CREATION_MODE_OPEN,  // open existing file
    FILE_CREATION_MODE_CREATE // create new file or truncate if exists
} file_creation_mode_t;

// todo: differentiate from pipe/socket and other types of files
typedef enum file_type {
    FILE_TYPE_DIRECTORY = 1 << 0,
    FILE_TYPE_FILE      = 1 << 1
} file_type_t;

typedef enum file_seek_type {
    FILE_SEEK_TYPE_BEGIN,
    FILE_SEEK_TYPE_CUR,
    FILE_SEEK_TYPE_END
} file_seek_type_t;

PUBLIC_API bool file__open(
    file_t* self,
    const char* file_path,
    file_access_mode_t access_mode,
    file_creation_mode_t creation_mode
);
PUBLIC_API void file__close(file_t* self);

// @brief creates an empty file on the specified path
// @note if the file existed on the path, it gets replaced
PUBLIC_API bool file__create(const char* path);
PUBLIC_API bool file__exists(const char* path);
PUBLIC_API bool file__delete(const char* path);
// @brief renames or moves a directory (including its children) if exists
PUBLIC_API bool file__move(const char* src_path, const char* dest_path);
// @returns last time that the file was modified
PUBLIC_API bool file__last_modified(const char* path, time_t* last_modified);
PUBLIC_API bool file__stat(const char* path, file_type_t* file_type);
// @returns whether the operations was successful or not as well as the file_size if it was
PUBLIC_API bool file__size(const char* path, size_t* file_size);
// @brief copies the source file to the destination path
// @note if dest exists, its contents will be overwritten
PUBLIC_API bool file__copy(const char* dest_path, const char* src_path);

// @brief reads from opened file, returns bytes read
PUBLIC_API bool file__read(file_t* self, void* out, size_t size, size_t* opt_read_bytes);
//! @returns bytes written
PUBLIC_API bool file__write(file_t* self, const void* in, size_t size, size_t* opt_written_bytes);
//! @returns bytes written excluding null-terminating character
PUBLIC_API bool file__fwrite(file_t* self, size_t* opt_written_bytes, const char* format, ...);
//! @returns bytes written excluding null-terminating character
PUBLIC_API bool file__vfwrite(file_t* self, size_t* opt_written_bytes, const char* format, va_list ap);
PUBLIC_API bool file__seek(file_t* self, size_t offset, file_seek_type_t seek_type, size_t* opt_file_pointer_position);

/**
 * Directory API
*/

typedef struct directory {
    DIR* handle;
} directory_t;

// todo: move file dependency out from this file
// note: enum file_flag def

// @brief opens the first file in the specified directory
PUBLIC_API bool directory__open(directory_t* self, const char* path);
// @brief closes the directory handle
PUBLIC_API void directory__close(directory_t* self);
// @brief returns the name of the current file of the opened directory, and moves to the next file
// @param bytes_written optional parameter to retrieve the number of bytes written (or would have been written in the case of truncation) into the buffer
// @returns true on success, false if no more files are in the directory
PUBLIC_API bool directory__read(directory_t* self, char* buffer, size_t buffer_size, size_t* bytes_written);

// @brief if it doesn't already exist, create a directory
PUBLIC_API bool directory__create(const char* path);
// @brief deletes a directory if exists and empty
PUBLIC_API bool directory__delete(const char* path);

// TODO: implement for-each API
// // @brief apply fn on each file_type on path one level deep
// void directory__foreach_shallow(const char* path, bool (*fn)(const char* path, void* user_data), void* user_data, enum file_type file_type_flags);
// // @brief apply fn on each file_type on path recursively
// // @param fn function to apply on each of the matching files, should return true if the algorithm should keep recursing on the matched file if it's a directory
// void directory__foreach_deep(
//     const char* path,
//     bool (*fn)(const char* path, void* user_data), void* user_data,
//     enum file_type file_type_flags
// );
// // @brief apply fn on each file_type on path depth level deep
// // @param depth 0 depth is equal to a for each shallow
// // @param fn function to apply on each of the matching files, should return true if the algorithm should keep recursing on the matched file if it's a directory
// void directory__foreach(const char* path, bool (*fn)(const char* path, void* user_data), void* user_data, enum file_type file_type_flags, size_t depth);

#endif
