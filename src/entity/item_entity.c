#include "entity/item_entity.h"
#include "entity/player.h"
#include "game.h"
#include "item_container.h"
#include "raylib.h"
#include "types.h"

#include <raymath.h>
#include <stdlib.h>

void item_entity_update(Entity* entity, float deltaTime);
void item_entity_draw(Entity* entity);
void item_entity_destroy(Entity* entity);

ItemEntity* item_entity_create(Vector2 position, Vector2 initial_velocity, ItemSlot item) {
	ItemEntity* ie = malloc(sizeof(ItemEntity));
	if (!ie) return NULL;

	ie->item = item;
	ie->timer = 0.0f;

	ie->entity.parent = ie;

	ie->entity.rect.x = position.x;
	ie->entity.rect.y = position.y;
	ie->entity.rect.width = TILE_SIZE * 0.5f;
	ie->entity.rect.height = TILE_SIZE * 0.5f;

	ie->entity.velocity = initial_velocity;

	ie->entity.gravity_affected = true;
	ie->entity.collides = true;

	ie->entity.update = item_entity_update;
	ie->entity.draw = item_entity_draw;
	ie->entity.destroy = item_entity_destroy;

	return ie;
}

void item_entity_update(Entity* entity, float deltaTime) {
	if (!entity) return;
	ItemEntity* ie = entity->parent;

	if (ie->timer < 1.0f) ie->timer += deltaTime;
	if (ie->timer > 1.0f) ie->timer = 1.0f;

	if (entity->grounded) {
		entity->velocity.x = Lerp(entity->velocity.x, 0.0f, 20.0f * deltaTime);
	}

	Player* player = game_get_player();
	if (player) {
		if (CheckCollisionRecs(entity->rect, player->entity.rect) && ie->timer >= 1.0f && !entity->to_remove) {
			distribute_item(&ie->item, get_inventory());
			entity->to_remove = true;
		}
	}
}

void item_entity_draw(Entity* entity) {
	if (!entity) return;
	ItemEntity* ie = entity->parent;

	draw_item(ie->item, entity->rect.x, entity->rect.y, 0, entity->rect.width / TILE_SIZE, false);
}

void item_entity_destroy(Entity* entity) {
	if (!entity) return;
	ItemEntity* ie = entity->parent;
	free(ie);
}