#ifndef GAME_H
# define GAME_H

# include <stdbool.h>

# include "gfx.h"
# include "helper_macros.h"

typedef struct game* game_t;

/**
 * @brief Creates and initializes the game state
 * @brief Set window options for the game, ex. cursor, icon, size, position
*/
PUBLIC_API game_t game__create(window_t window);
PUBLIC_API void game__destroy(game_t self);

PUBLIC_API void game__frame_start(game_t self);

/**
 * @returns The real time (in seconds) it takes at most to do one game__update
 * @note Necessary for fixed time step across multiple clients
 * @note If too low (lower than the average time to do a single game__update), the game loop won't catch up to real-time
 * @note Or it could catch by the game lowering its own time to take a single game__update, for example by lowering the game's LOD
*/
PUBLIC_API double game__update_upper_bound(game_t self);

/**
 * @note Should be fairly deterministic in terms of how much a call takes to work best with the fixed time step
*/
PUBLIC_API void game__update(game_t self, double s);

/**
 * @param render_interpolation_factor [0, 1] value that can be used for interpolation during rendering, it is a result of the fixed time step update
*/
PUBLIC_API void game__render(game_t self, double render_interpolation_factor);

#endif // GAME_H
