#include "player.h"
#include "types.h"
#include "chunk_manager.h"

#include <stdlib.h>
#include <stdint.h>

#include <raymath.h>
#include <rlgl.h>

#define SPEED 10.0f * TILE_SIZE
#define JUMP_FORCE 16.0f * TILE_SIZE
#define ROTATION_AMOUNT 549.57f
#define MOVEMENT_ACCEL_RATE 20.0f

#define TO_BLOCK_COORDS(value) ((int)floorf((float)value / (float)TILE_SIZE))

void player_update(Entity* entity, float deltaTime);
void player_draw(Entity* entity);
void player_destroy(Entity* entity);

Player* player_create(Vector2 initialPosition) {
	Player* player = malloc(sizeof(Player));
	if (!player) return NULL;

	player->direction = 0;
	player->rotation = 0.0f;
	player->disable_input = false;

	player->entity.rect.x = initialPosition.x;
	player->entity.rect.y = initialPosition.y;
	player->entity.rect.width = 27.0f;
	player->entity.rect.height = 27.0f;

	player->entity.velocity = Vector2Zero();
	player->entity.collides = true;
	player->entity.gravity_affected = true;

	player->entity.parent = player;
	player->entity.update = player_update;
	player->entity.draw = player_draw;
	player->entity.destroy = player_destroy;

	return player;
}

void player_update(Entity* entity, float deltaTime) {
	if (!entity) return;
	Player* player = (Player*)entity->parent;

	float movement_accel_rate = MOVEMENT_ACCEL_RATE;
	if (player->entity.on_liquid) {
		movement_accel_rate /= 4.0f;
	}

	if (!player->disable_input) {
		float speed = SPEED;
		if (player->entity.on_liquid) {
			speed /= 2.0f;
		}

		if (IsKeyDown(KEY_LEFT_SHIFT)) {
			speed /= 4.0;
		}
		else if (IsKeyDown(KEY_LEFT_CONTROL)) {
			speed *= 2.0;
		}

		if (entity->gravity_affected) {
			if ((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))) {
				
				if (player->entity.on_liquid || player->entity.on_climbable) {
					float up_force = -JUMP_FORCE;
					if (!player->entity.on_liquid && player->entity.on_climbable) up_force *= 0.8f;
					player->entity.velocity.y = Lerp(player->entity.velocity.y, up_force, movement_accel_rate * deltaTime);
				}
				else if (player->entity.grounded) {
					player->entity.velocity.y -= JUMP_FORCE;
				}
			}
		}
		else {
			player->direction = 0;

			if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
				player->entity.velocity.y = Lerp(player->entity.velocity.y, -speed, movement_accel_rate * deltaTime);
			}
			else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
				player->entity.velocity.y = Lerp(player->entity.velocity.y, speed, movement_accel_rate * deltaTime);
			}
			else {
				player->entity.velocity.y = Lerp(player->entity.velocity.y, 0.0f, movement_accel_rate * deltaTime);
			}
		}

		if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
			player->entity.velocity.x = Lerp(player->entity.velocity.x, -speed, movement_accel_rate * deltaTime);
			if (entity->gravity_affected) player->direction = -1;
		}
		else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
			player->entity.velocity.x = Lerp(player->entity.velocity.x, speed, movement_accel_rate * deltaTime);
			if (entity->gravity_affected) player->direction = 1;
		}
		else {
			player->entity.velocity.x = Lerp(player->entity.velocity.x, 0.0f, movement_accel_rate * deltaTime);
			if (player->entity.grounded && entity->gravity_affected) player->direction = 0;
		}
	}
	else {
		player->entity.velocity.x = Lerp(player->entity.velocity.x, 0.0f, movement_accel_rate * deltaTime);
		if (player->entity.grounded) player->direction = 0;
		if (!entity->gravity_affected) {
			player->entity.velocity.y = Lerp(player->entity.velocity.y, 0.0f, movement_accel_rate * deltaTime);
		}
	}

	if (entity->gravity_affected) {
		float rotation_amount = ROTATION_AMOUNT;

		if (entity->on_liquid) {
			rotation_amount /= 2.0f;
		}

		if (!entity->grounded) {
			player->rotation += rotation_amount * deltaTime * player->direction;
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
}

void player_draw(Entity* entity) {
	if (!entity) return;
	Player* player = entity->parent;

	Vector2 playerCenter = entity_get_center(&player->entity);
	uint8_t light = chunk_manager_get_light((Vector2i) { TO_BLOCK_COORDS(playerCenter.x), TO_BLOCK_COORDS(playerCenter.y) });
	if (light < 2) light = 2;

	rlPushMatrix();

	rlTranslatef(
		player->entity.rect.width / 2.0f,
		player->entity.rect.height / 2.0f,
		0.0f
	);

	DrawRectanglePro(
		player->entity.rect,
		Vector2Scale((Vector2) { player->entity.rect.width, player->entity.rect.height }, 0.5f),
		player->rotation,
		(Color) { 255 * ((float)light / 15.0f), 0, 0, 255 }
	);
	
	rlPopMatrix();
}

void player_destroy(Entity* entity) {
	if (!entity) return;
	Player* player = entity->parent;
	free(player);
}

Vector2 player_get_position(Player* player) {
	if (!player) return Vector2Zero();
	return (Vector2) { player->entity.rect.x, player->entity.rect.y };
}

Vector2 player_get_size(Player* player) {
	if (!player) return Vector2Zero();
	return (Vector2) { player->entity.rect.width, player->entity.rect.height };
}
