#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>

#include "entity.h"

typedef struct {
	Entity entity;
	int direction;
	float rotation;
	bool disable_input;
} Player;

Player* player_create(Vector2 initialPosition);

Vector2 player_get_position(Player* player);
Vector2 player_get_size(Player* player);

#endif