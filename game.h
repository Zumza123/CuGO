#ifndef CUGO_GAME_H
#define CUGO_GAME_H

#include <SDL2/SDL.h>

void game_init();
void game_deinit();

bool game_inited();
bool game_started();
bool game_ended();

void game_loop(SDL_Event);

#endif // CUGO_GAME_H
