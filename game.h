#ifndef GAME_H
# define GAME_H

# include "glfw.h"

# include <stdbool.h>

struct         game;
typedef struct game* game_t;

game_t game__create(window_t window);
void game__destroy(game_t self);

void game__update(game_t self, double s);

#endif // GAME_H
