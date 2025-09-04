#include "player.h"
#include "defines.h"

#include <stdio.h>

#include <raymath.h>
#include <rlgl.h>

#define SPEED 10.0f * TILE_SIZE
#define GRAVITY_ACCEL 98.07f * TILE_SIZE
#define TERMINAL_GRAVITY 32.0f * TILE_SIZE
#define JUMP_FORCE 16.0f * TILE_SIZE

void player_init(Player* player, Vector2 initialPosition) {
	if (!player) return;

	player->direction = 0;
	player->rotation = 0.0f;

	player->entity.rect.x = initialPosition.x;
	player->entity.rect.y = initialPosition.y;
	player->entity.rect.width = 28.0f;
	player->entity.rect.height = 28.0f;

	player->entity.velocity = Vector2Zero();
	player->entity.collides = true;

}

void player_update(Player* player, float deltaTime, bool disableInput) {
	if (!player) return;

	/*
	Vector2 input = { 0 };

	if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x = -1.0f;
	else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x = 1.0f;

	if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y = -1.0f;
	else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y = 1.0f;

	input = Vector2Normalize(input);
	*/

	if (!disableInput) {
		if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
			player->entity.velocity.x = -1.0f;
			player->direction = -1;
		}
		else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
			player->entity.velocity.x = 1.0f;
			player->direction = 1;
		}
		else {
			player->entity.velocity.x = 0.0f;
			if (player->entity.grounded) player->direction = 0;
		}

		float speed = SPEED;
		if (IsKeyDown(KEY_LEFT_SHIFT)) {
			speed *= 4.0;
		}
		else if (IsKeyDown(KEY_LEFT_CONTROL)) {
			speed /= 4.0;
		}

		player->entity.velocity.x *= speed;

		if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && player->entity.grounded) {
			player->entity.velocity.y -= JUMP_FORCE;
		}
	}
	else {
		player->entity.velocity.x = 0.0f;
	}

	if (player->entity.velocity.y < TERMINAL_GRAVITY) {
		player->entity.velocity.y += GRAVITY_ACCEL * deltaTime;
	}
	else if (player->entity.velocity.y > TERMINAL_GRAVITY) {
		player->entity.velocity.y = TERMINAL_GRAVITY;
	}

	if (!player->entity.grounded) {
		player->rotation += 549.57f * deltaTime * player->direction;
	}
	else {
		player->rotation = 0.0f;
	}

	entity_update(&player->entity, deltaTime);
}

void player_draw(Player* player) {
	if (!player) return;

	rlPushMatrix();

	rlTranslatef(
		player->entity.rect.width / 2.0f,
		player->entity.rect.height / 2.0f,
		0.0f
	);

	DrawRectanglePro(player->entity.rect, Vector2Scale((Vector2) { player->entity.rect.width, player->entity.rect.height }, 0.5f), player->rotation, RED);
	
	rlPopMatrix();

	//entity_draw(&player->entity);
}

Vector2 player_get_position(Player* player) {
	if (!player) return Vector2Zero();
	return (Vector2) { player->entity.rect.x, player->entity.rect.y };
}

Vector2 player_get_size(Player* player) {
	if (!player) return Vector2Zero();
	return (Vector2) { player->entity.rect.width, player->entity.rect.height };
}
