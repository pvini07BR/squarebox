#ifndef GAME_H
#define GAME_H

#include "entity/player.h"
#include <stdbool.h>

void game_init();
void game_tick();
void game_update(float deltaTime);
void game_draw();
void game_free();

Player* game_get_player();

#endif