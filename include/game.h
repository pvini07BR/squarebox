#ifndef GAME_H
#define GAME_H

#include "entity/player.h"
#include <stdbool.h>

void game_init();
void game_tick();
void game_update(float deltaTime);
void game_draw();
void game_free();

bool game_is_ui_open();

void game_set_demo_mode(bool demo);
bool game_is_demo_mode();

void game_set_draw_ui(bool draw);

Player* game_get_player();
Vector2 game_get_camera_pos();

#endif