#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>

#include "file.h"

int main() {
    file_t file;
    if (!file__open(&file, "idk", FILE_ACCESS_MODE_WRITE, FILE_CREATION_MODE_CREATE)) {
        return 1;
    }
    size_t bytes_written = 0;
    if (!file__fwrite(&file, &bytes_written, "%s", "hi")) {
        return 2;
    }
    printf("Bytes written: %u\n", bytes_written);

    file__close(&file);

    return 0;
}
