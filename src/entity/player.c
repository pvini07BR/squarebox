#include "entity/player.h"
#include "chunk_manager.h"
#include "types.h"

#include <stdlib.h>
#include <stdint.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define SPEED 10.0f * TILE_SIZE
#define JUMP_FORCE 16.0f * TILE_SIZE
#define ROTATION_AMOUNT 549.57f

#define TO_BLOCK_COORDS(value) ((int)floorf((float)value / (float)TILE_SIZE))

void player_update(Entity* entity, float deltaTime);
void player_draw(Entity* entity);
void player_destroy(Entity* entity);

Player* player_create(Vector2 initialPosition, Color color) {
	Player* player = malloc(sizeof(Player));
	if (!player) return NULL;

	player->direction = 0;
	player->rotation = 0.0f;
	player->disable_input = false;
	player->color = color;

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

	if (player->disable_input) {
		if (!entity->gravity_affected)
			entity->velocity = Vector2Lerp(entity->velocity, Vector2Zero(), 5.0f * deltaTime);
		else {
			entity->velocity.x = Lerp(player->entity.velocity.x, 0.0f, 20.0f * deltaTime);
			if (player->entity.grounded) player->direction = 0;
		}

		return;
	}

	float speed = SPEED;

	if (entity->on_liquid) speed /= 2.0f;

	if (IsKeyDown(KEY_LEFT_SHIFT)) speed /= 4.0f;
	else if (IsKeyDown(KEY_LEFT_CONTROL)) speed *= 2.5f;

	// If not gravity affected, then start floating
	if (!entity->gravity_affected) {
		player->direction = 0;
		float nineties = roundf((player->rotation / 90.0f)) * 90.0f;
		player->rotation = Lerp(player->rotation, nineties, 50.0f * deltaTime);

		Vector2 dir = {
			((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) - (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))),
			((IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) - (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)))
		};
		
		if (dir.x != 0.0f || dir.y != 0.0f)
			dir = Vector2Normalize(dir);

		entity->velocity = Vector2Lerp(entity->velocity, Vector2Scale(dir, speed), 5.0f * deltaTime);
	}
	// Otherwise behave like a platformer player controller
	else {
		// Horizontal movement
		if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
			player->entity.velocity.x = Lerp(player->entity.velocity.x, -speed, 20.0f * deltaTime);
			player->direction = -1;
		}
		else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
			player->entity.velocity.x = Lerp(player->entity.velocity.x, speed, 20.0f * deltaTime);
			player->direction = 1;
		}
		else {
			player->entity.velocity.x = Lerp(player->entity.velocity.x, 0.0f, 20.0f * deltaTime);
			if (player->entity.grounded) player->direction = 0;
		}

		// Jumping or swimming
		if (IsKeyDown(KEY_W) || IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP)) {
			if (entity->on_liquid || entity->on_climbable) {
				float up_speed = -JUMP_FORCE;
				if (entity->on_liquid) up_speed *= 0.5f;
				else if (entity->on_climbable) up_speed *= 0.75f;

				player->entity.velocity.y = Lerp(player->entity.velocity.y, up_speed, 20.0f * deltaTime);
			}
			else if (entity->grounded) {
				player->entity.velocity.y -= JUMP_FORCE;
			}
		}

		// Rotating the player quad based on speed (purely visual)
		float rotation_amount = ROTATION_AMOUNT;

		if (entity->on_liquid) {
			rotation_amount /= 2.0f;
		}

		if (!entity->grounded && !entity->on_climbable) {
			player->rotation += rotation_amount * deltaTime * player->direction;
		}
		else {
			float nineties = roundf((player->rotation / 90.0f)) * 90.0f;
			player->rotation = Lerp(player->rotation, nineties, 50.0f * deltaTime);
		}
	}
}

void player_draw(Entity* entity) {
	if (!entity) return;
	Player* player = entity->parent;

	Vector2 playerCenter = entity_get_center(&player->entity);
	uint8_t light = chunk_manager_get_light((Vector2i) { TO_BLOCK_COORDS(playerCenter.x), TO_BLOCK_COORDS(playerCenter.y) });
	if (light < 2) light = 2;

	Color playerColor = player->color;
	playerColor.r *= ((float)light / 15.0f);
	playerColor.g *= ((float)light / 15.0f);
	playerColor.b *= ((float)light / 15.0f);

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
		playerColor
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
