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
		.name = "Stone Block",
		.atlas_idx = 2,
		.blockId = 3
	};

	itemRegistry[4] = (ItemRegistry){
		.name = "Cobblestone",
		.atlas_idx = 3,
		.blockId = 4
	};

	itemRegistry[5] = (ItemRegistry){
		.name = "Wooden Planks",
		.atlas_idx = 4,
		.blockId = 5
	};

	itemRegistry[6] = (ItemRegistry){
		.name = "Wood Log",
		.atlas_idx = 5,
		.blockId = 6
	};

	itemRegistry[7] = (ItemRegistry){
		.name = "Leaves",
		.atlas_idx = 6,
		.blockId = 7
	};

	itemRegistry[8] = (ItemRegistry){
		.name = "Glass Block",
		.atlas_idx = 7,
		.blockId = 8
	};

	itemRegistry[9] = (ItemRegistry){
		.name = "Lamp Block",
		.atlas_idx = 8,
		.blockId = 9
	};

	itemRegistry[10] = (ItemRegistry){
		.name = "Chest",
		.atlas_idx = 9,
		.blockId = 10
	};

	itemRegistry[11] = (ItemRegistry){
		.name = "Test Item",
		.atlas_idx = 10,
		.blockId = 0
	};
}

ItemRegistry* ir_get_item_registry(size_t idx)
{
	if (idx < 0 || idx > ITEM_COUNT - 1) return NULL;
	return &itemRegistry[idx];
}

void item_registry_free() { if (itemRegistry) free(itemRegistry); }