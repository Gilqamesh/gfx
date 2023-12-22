typedef struct image {
    uint8_t* data;
    uint32_t w;
    uint32_t h;
} image_t;

struct game {
    cursor_t  cursor;
    image_t   window_icon_image;
    image_t   cursor_image;
};

static bool game__load_images(game_t self);

static bool game__load_images(game_t self) {
    int number_of_channels_per_pixel;
    self->window_icon_image.data = stbi_load("assets/icon.png", (int32_t*) &self->window_icon_image.w, (int32_t*) &self->window_icon_image.h, &number_of_channels_per_pixel, 0);
    if (!self->window_icon_image.data) {
        return false;
    }
    self->cursor_image.data = stbi_load("assets/cursor.png", (int32_t*) &self->cursor_image.w, (int32_t*) &self->cursor_image.h, &number_of_channels_per_pixel, 0);
    if (!self->cursor_image.data) {
        return false;
    }

    return true;
}
