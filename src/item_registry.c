#include "item_registry.h"
#include "block_models.h"

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
		.model_idx = 0,
		.blockId = 0
	};

	itemRegistry[1] = (ItemRegistry){
		.name = "Grass Block",
		.atlas_idx = 0,
		.model_idx = 0,
		.blockId = 1
	};

	itemRegistry[2] = (ItemRegistry){
		.name = "Dirt Block",
		.atlas_idx = 1,
		.model_idx = 0,
		.blockId = 2
	};

	itemRegistry[3] = (ItemRegistry){
		.name = "Stone Block",
		.atlas_idx = 2,
		.model_idx = 0,
		.blockId = 3
	};

	itemRegistry[4] = (ItemRegistry){
		.name = "Cobblestone",
		.atlas_idx = 3,
		.model_idx = 0,
		.blockId = 4
	};

	itemRegistry[5] = (ItemRegistry){
		.name = "Wooden Planks",
		.atlas_idx = 4,
		.model_idx = 0,
		.blockId = 5
	};

	itemRegistry[6] = (ItemRegistry){
		.name = "Wood Log",
		.atlas_idx = 5,
		.model_idx = 0,
		.blockId = 6
	};

	itemRegistry[7] = (ItemRegistry){
		.name = "Leaves",
		.atlas_idx = 6,
		.model_idx = 0,
		.blockId = 7
	};

	itemRegistry[8] = (ItemRegistry){
		.name = "Glass Block",
		.atlas_idx = 7,
		.model_idx = 0,
		.blockId = 8
	};

	itemRegistry[9] = (ItemRegistry){
		.name = "Lamp Block",
		.atlas_idx = 8,
		.model_idx = 0,
		.blockId = 9
	};

	itemRegistry[10] = (ItemRegistry){
		.name = "Chest",
		.atlas_idx = 9,
		.model_idx = 0,
		.blockId = 10
	};

	itemRegistry[11] = (ItemRegistry){
		.name = "Cobblestone Slab",
		.atlas_idx = 3,
		.model_idx = 1,
		.blockId = 11
	};

	itemRegistry[12] = (ItemRegistry){
		.name = "Cobblestone Stairs",
		.atlas_idx = 3,
		.model_idx = 2,
		.blockId = 12
	};

	itemRegistry[13] = (ItemRegistry){
		.name = "Test Item",
		.atlas_idx = 10,
		.model_idx = 0,
		.blockId = 0
	};

	for (int i = 0; i < ITEM_COUNT; i++) {
		itemRegistry[i].mesh = (Mesh){ 0 };
		printf("Model index: %d\nAtlas index: %d\n", itemRegistry[i].model_idx, itemRegistry[i].atlas_idx);
		block_models_build_mesh(&itemRegistry[i].mesh, itemRegistry[i].model_idx, itemRegistry[i].atlas_idx, false, false);
	}
}

ItemRegistry* ir_get_item_registry(size_t idx)
{
	if (idx < 0 || idx > ITEM_COUNT - 1) return NULL;
	return &itemRegistry[idx];
}

void item_registry_free() {
	for (int i = 0; i < ITEM_COUNT; i++) { UnloadMesh(itemRegistry[i].mesh); }
	if (itemRegistry) free(itemRegistry);
}