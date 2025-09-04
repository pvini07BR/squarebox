#ifndef PLAYER_H
#define PLAYER_H

#include <raylib.h>

#include "entity.h"

typedef struct {
	Entity entity;
	int direction;
	float rotation;
	bool flying;
} Player;

void player_init(Player* player, Vector2 initialPosition);
void player_update(Player* player, float deltaTime, bool disableInput);
void player_draw(Player* player);

Vector2 player_get_position(Player* player);
Vector2 player_get_size(Player* player);

#endif