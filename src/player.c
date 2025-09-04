#include "player.h"
#include "defines.h"

#include <stdio.h>

#include <raymath.h>
#include <rlgl.h>

#define SPEED 10.0f * TILE_SIZE
#define GRAVITY_ACCEL 98.07f * TILE_SIZE
#define TERMINAL_GRAVITY 32.0f * TILE_SIZE
#define JUMP_FORCE 16.0f * TILE_SIZE
#define ROTATION_AMOUNT 549.57f
#define MOVEMENT_ACCEL_RATE 20.0f

void player_init(Player* player, Vector2 initialPosition) {
	if (!player) return;

	player->direction = 0;
	player->rotation = 0.0f;
	player->flying = false;

	player->entity.rect.x = initialPosition.x;
	player->entity.rect.y = initialPosition.y;
	player->entity.rect.width = 28.0f;
	player->entity.rect.height = 28.0f;

	player->entity.velocity = Vector2Zero();
	player->entity.collides = true;
}

void player_update(Player* player, float deltaTime, bool disableInput) {
	if (!player) return;

	if (!disableInput) {
		float speed = SPEED;
		if (IsKeyDown(KEY_LEFT_SHIFT)) {
			speed /= 4.0;
		}
		else if (IsKeyDown(KEY_LEFT_CONTROL)) {
			speed *= 2.0;
		}

		if (!player->flying) {
			if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP)) && player->entity.grounded) {
				player->entity.velocity.y -= JUMP_FORCE;
			}
		}
		else {
			player->direction = 0;

			if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
				player->entity.velocity.y = Lerp(player->entity.velocity.y, -speed, MOVEMENT_ACCEL_RATE * deltaTime);
			}
			else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
				player->entity.velocity.y = Lerp(player->entity.velocity.y, speed, MOVEMENT_ACCEL_RATE * deltaTime);
			}
			else {
				player->entity.velocity.y = Lerp(player->entity.velocity.y, 0.0f, MOVEMENT_ACCEL_RATE * deltaTime);
			}
		}

		if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
			player->entity.velocity.x = Lerp(player->entity.velocity.x, -speed, MOVEMENT_ACCEL_RATE * deltaTime);
			if (!player->flying) player->direction = -1;
		}
		else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
			player->entity.velocity.x = Lerp(player->entity.velocity.x, speed, MOVEMENT_ACCEL_RATE * deltaTime);
			if (!player->flying) player->direction = 1;
		}
		else {
			player->entity.velocity.x = Lerp(player->entity.velocity.x, 0.0f, MOVEMENT_ACCEL_RATE * deltaTime);
			if (player->entity.grounded && !player->flying) player->direction = 0;
		}
	}
	else {
		player->entity.velocity.x = Lerp(player->entity.velocity.x, 0.0f, MOVEMENT_ACCEL_RATE * deltaTime);
		if (player->entity.grounded) player->direction = 0;
	}

	if (!player->flying) {
		if (player->entity.velocity.y < TERMINAL_GRAVITY) {
			player->entity.velocity.y += GRAVITY_ACCEL * deltaTime;
		}
		else if (player->entity.velocity.y > TERMINAL_GRAVITY) {
			player->entity.velocity.y = TERMINAL_GRAVITY;
		}

		if (!player->entity.grounded) {
			player->rotation += ROTATION_AMOUNT * deltaTime * player->direction;
		}
		else {
			float nineties = roundf((player->rotation / 90.0f)) * 90.0f;
			player->rotation = Lerp(player->rotation, nineties, 50.0f * deltaTime);
		}
	}
	else {
		float nineties = roundf((player->rotation / 90.0f)) * 90.0f;
		player->rotation = Lerp(player->rotation, nineties, 50.0f * deltaTime);
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
