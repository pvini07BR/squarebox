#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>

#include "entity.h"

typedef struct {
	Entity entity;
	Color color;
	float rotation;
	bool disable_input;
	bool last_on_slippery;
} Player;

Player* player_create(Vector2 initialPosition, Color color);

Vector2 player_get_position(Player* player);
Vector2 player_get_size(Player* player);

#endif