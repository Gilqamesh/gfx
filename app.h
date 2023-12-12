#ifndef APP_H
# define APP_H

# include <stdbool.h>
# include <stdint.h>

# include "glfw.h"

typedef struct image {
    uint8_t* data;
    uint32_t w;
    uint32_t h;
} image_t;

typedef struct app {
    image_t   window_icon;
    uint32_t  frame_count;
    window_t  window;
    monitor_t monitor;
} app_t;

bool app__create(app_t* self, int argc, char* argv[]);
void app__destroy(app_t* self);

void app__run(app_t* self);

#endif // APP_H
