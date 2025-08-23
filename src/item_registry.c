#include "item_registry.h"

#include <stdio.h>
#include <stdlib.h>

static ItemRegistry* itemRegistry = NULL;

void item_registry_init() {
	itemRegistry = calloc(ITEM_COUNT, sizeof(ItemRegistry));
	if (itemRegistry == NULL) {
		fprintf(stderr, "[ERROR] Could not allocate memory for the item registry.\n");
		return;
	};
	
	itemRegistry[0] = (ItemRegistry){
		.name = "No Item",
		.atlas_idx = 0,
		.blockId = 0
	};

	itemRegistry[1] = (ItemRegistry){
		.name = "Grass Block",
		.atlas_idx = 0,
		.blockId = 1
	};

	itemRegistry[2] = (ItemRegistry){
		.name = "Dirt Block",
		.atlas_idx = 1,
		.blockId = 2
	};

	itemRegistry[3] = (ItemRegistry){
		.name = "Test Item",
		.atlas_idx = 8,
		.blockId = 0
	};
}

ItemRegistry* ir_get_item_registry(size_t idx)
{
	if (idx < 0 || idx > ITEM_COUNT - 1) return NULL;
	return &itemRegistry[idx];
}

void item_registry_free() { if (itemRegistry) free(itemRegistry); }