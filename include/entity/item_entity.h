#ifndef ITEM_ENTITY_H
#define ITEM_ENTITY_H

#include <raylib.h>

#include "entity.h"
#include "item_container.h"

typedef struct {
	Entity entity;
	ItemSlot item;
} ItemEntity;

ItemEntity* item_entity_create(Vector2 position, Vector2 initial_velocity, ItemSlot item);

#endif