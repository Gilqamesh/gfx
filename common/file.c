#include "file.h"

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdint.h>
#include <assert.h>
#include <sys/sendfile.h>

static inline uint32_t file_access_mode(enum file_access_mode access_mode) {
    uint32_t result = 0;

    if (access_mode == FILE_ACCESS_MODE_READ) {
        result = O_RDONLY;
    } else if (access_mode == FILE_ACCESS_MODE_WRITE) {
        result = O_WRONLY;
    } else if (access_mode == FILE_ACCESS_MODE_RDWR) {
        result = O_RDWR;
    } else {
        assert(false);
    }

    return result;
}

static inline uint32_t file_creation_mode(enum file_creation_mode creation_mode) {
    uint32_t result = 0;

    if (creation_mode == FILE_CREATION_MODE_OPEN) {
    } else if (creation_mode == FILE_CREATION_MODE_CREATE) {
        result = O_CREAT | O_TRUNC;
    } else {
        assert(false);
    }

    return result;
}

bool file__open(
    file_t* self,
    const char* file_path,
    file_access_mode_t access_mode,
    file_creation_mode_t creation_mode
) {
    self->fd = open(
        file_path,
        file_access_mode(access_mode) | file_creation_mode(creation_mode),
        S_IRUSR | S_IWUSR
    );
    if (self->fd == -1) {
        // todo: diagnostic, check errno
        return false;
    }

    return true;
}

void file__close(file_t* self) {
    close(self->fd);
}

bool file__create(const char* path) {
    file_t file;
    if (!file__open(&file, path, FILE_ACCESS_MODE_WRITE, FILE_CREATION_MODE_CREATE)) {
        return false;
    }
    file__close(&file);
    return true;
}

bool file__exists(const char* path) {
    return access(path, F_OK) == 0;
}

bool file__delete(const char* path) {
    if (unlink(path) == -1) {
        // todo: diagnostic, check errno
        return false;
    }

    return true;
}

bool file__move(const char* src_path, const char* dest_path) {
    if (rename(src_path, dest_path) == -1) {
        // todo: diagnostic, check errno
        return false;
    }

    return true;
}

bool file__last_modified(const char* path, time_t* last_modified) {
    struct stat file_info;
    if (stat(path, &file_info) == -1) {
        // todo: diagnostics, check errno
        return false;
    }

    *last_modified = file_info.st_mtime;

    return true;
}

bool file__stat(const char* path, file_type_t* file_type) {
    struct stat file_info;
    if (stat(path, &file_info) == -1) {
        // todo: diagnostics, check errno
        return false;
    }

    *file_type = 0;
    *file_type = S_ISDIR(file_info.st_mode) ? FILE_TYPE_DIRECTORY : FILE_TYPE_FILE;

    return true;
}

bool file__size(const char* path, size_t* file_size) {
    struct stat file_info;

    if (stat(path, &file_info) == -1) {
        // todo: diagnostics, check errno
        return false;
    }

    *file_size = file_info.st_size;

    return true;
}

bool file__copy(const char* dest_path, const char* src_path) {
    size_t in_file_size = 0;
    if (!file__size(src_path, &in_file_size)) {
        return false;
    }

    file_t in_file;
    file_t out_file;
    if (!file__open(&in_file, src_path, FILE_ACCESS_MODE_READ, FILE_CREATION_MODE_OPEN)) {
        return false;
    }
    if (!file__open(&out_file, dest_path, FILE_ACCESS_MODE_WRITE, FILE_CREATION_MODE_CREATE)) {
        file__close(&in_file);
        return false;
    }

    bool result = true;
    if (sendfile(out_file.fd, in_file.fd, 0, in_file_size) == -1) {
        // todo: diagnostic, check errno
        result = false;
    }

    file__close(&in_file);
    file__close(&out_file);

    return result;
}

bool file__read(file_t* self, void* out, size_t size, size_t* opt_read_bytes) {
    ssize_t read_bytes = read(self->fd, out, size);
    if (read_bytes == -1) {
        // todo: diagnostic, check errno
        return false;
    }
    
    if (opt_read_bytes) {
        *opt_read_bytes = (size_t) read_bytes;
    }

    return true;
}

bool file__write(file_t* self, const void* in, size_t size, size_t* opt_written_bytes) {
    ssize_t written_bytes = write(self->fd, in, size);

    if (written_bytes == -1) {
        // todo: diagnostic, check errno
        return false;
    }

    if (opt_written_bytes) {
        *opt_written_bytes = (size_t) written_bytes;
    }

    return true;
}

bool file__fwrite(file_t* self, size_t* opt_written_bytes, const char* format, ...) {
    va_list ap;
    va_start(ap, format);
    bool result = file__vfwrite(self, opt_written_bytes, format, ap);
    va_end(ap);

    return result;
}

bool file__vfwrite(file_t* self, size_t* opt_written_bytes, const char* format, va_list ap) {
    int bytes_written = vdprintf(self->fd, format, ap);
    if (bytes_written < 0) {
        return false;
    }

    if (opt_written_bytes) {
        *opt_written_bytes = bytes_written;
    }
    return true;
}

bool file__seek(file_t* self, size_t offset, file_seek_type_t seek_type, size_t* opt_file_pointer_position) {
    (void) seek_type;

    off_t file_pointer_position = lseek(self->fd, offset, SEEK_SET);
    if (file_pointer_position == -1) {
        // todo: diagnostic, check errno
        return false;
    }

    if (opt_file_pointer_position) {
        *opt_file_pointer_position = file_pointer_position;
    }

    return true;
}