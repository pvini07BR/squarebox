#ifndef ITEM_REGISTRY_H
#define ITEM_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

#define ITEM_COUNT 14

typedef struct {
	const char* name;
	size_t atlas_idx;
	size_t model_idx;
	Mesh mesh;
	
	// Set 0 if this item isn't going to place any blocks.
	uint8_t blockId;
} ItemRegistry;

void item_registry_init();
ItemRegistry* ir_get_item_registry(size_t idx);
void item_registry_free();

#endif