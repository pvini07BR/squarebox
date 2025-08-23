#ifndef ITEM_REGISTRY_H
#define ITEM_REGISTRY_H

#include <stdint.h>

#define ITEM_COUNT 10

typedef struct {
	const char* name;
	uint8_t atlas_idx;
	
	// Set 0 if this item isn't going to place any blocks.
	uint8_t blockId;
} ItemRegistry;

void item_registry_init();
ItemRegistry* ir_get_item_registry(size_t idx);
void item_registry_free();

#endif