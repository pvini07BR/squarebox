#include "item_registry.h"
#include "block_registry.h"
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
	
	itemRegistry[ITEM_NONE] = (ItemRegistry){
		.name = "No Item",
		.atlas_idx = 0,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_AIR
	};

	itemRegistry[ITEM_GRASS_BLOCK] = (ItemRegistry){
		.name = "Grass Block",
		.atlas_idx = 0,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_GRASS_BLOCK
	};

	itemRegistry[ITEM_DIRT_BLOCK] = (ItemRegistry){
		.name = "Dirt Block",
		.atlas_idx = 1,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_DIRT
	};

	itemRegistry[ITEM_STONE_BLOCK] = (ItemRegistry){
		.name = "Stone Block",
		.atlas_idx = 2,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_STONE
	};

	itemRegistry[ITEM_COBBLESTONE] = (ItemRegistry){
		.name = "Cobblestone",
		.atlas_idx = 3,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_COBBLESTONE
	};

	itemRegistry[ITEM_WOODEN_PLANKS] = (ItemRegistry){
		.name = "Wooden Planks",
		.atlas_idx = 4,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_WOODEN_PLANKS
	};

	itemRegistry[ITEM_WOOD_LOG] = (ItemRegistry){
		.name = "Wood Log",
		.atlas_idx = 5,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_WOOD_LOG
	};

	itemRegistry[ITEM_LEAVES] = (ItemRegistry){
		.name = "Leaves",
		.atlas_idx = 6,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_LEAVES
	};

	itemRegistry[ITEM_GLASS] = (ItemRegistry){
		.name = "Glass Block",
		.atlas_idx = 7,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_GLASS
	};

	itemRegistry[ITEM_LAMP] = (ItemRegistry){
		.name = "Lamp Block",
		.atlas_idx = 8,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_LAMP
	};

	itemRegistry[ITEM_CHEST] = (ItemRegistry){
		.name = "Chest",
		.atlas_idx = 9,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_CHEST
	};

	itemRegistry[ITEM_DIRT_SLAB] = (ItemRegistry){
		.name = "Dirt Slab",
		.atlas_idx = 1,
		.model_idx = BLOCK_MODEL_SLAB,
		.blockId = BLOCK_DIRT_SLAB
	};

	itemRegistry[ITEM_STONE_SLAB] = (ItemRegistry){
		.name = "Stone Slab",
		.atlas_idx = 2,
		.model_idx = BLOCK_MODEL_SLAB,
		.blockId = BLOCK_STONE_SLAB
	};

	itemRegistry[ITEM_COBBLESTONE_SLAB] = (ItemRegistry){
		.name = "Cobblestone Slab",
		.atlas_idx = 3,
		.model_idx = BLOCK_MODEL_SLAB,
		.blockId = BLOCK_COBBLESTONE_SLAB
	};

	itemRegistry[ITEM_WOODEN_PLANKS_SLAB] = (ItemRegistry){
		.name = "Wooden Planks Slab",
		.atlas_idx = 4,
		.model_idx = BLOCK_MODEL_SLAB,
		.blockId = BLOCK_WOODEN_PLANKS_SLAB
	};

	itemRegistry[ITEM_STONE_STAIRS] = (ItemRegistry){
		.name = "Stone Stairs",
		.atlas_idx = 2,
		.model_idx = BLOCK_MODEL_STAIRS,
		.blockId = BLOCK_STONE_STAIRS
	};

	itemRegistry[ITEM_COBBLESTONE_STAIRS] = (ItemRegistry){
		.name = "Cobblestone Stairs",
		.atlas_idx = 3,
		.model_idx = BLOCK_MODEL_STAIRS,
		.blockId = BLOCK_COBBLESTONE_STAIRS
	};

	itemRegistry[ITEM_WOODEN_PLANKS_STAIRS] = (ItemRegistry){
		.name = "Wooden Planks Stairs",
		.atlas_idx = 4,
		.model_idx = BLOCK_MODEL_STAIRS,
		.blockId = BLOCK_WOODEN_PLANKS_STAIRS
	};

	for (int i = 0; i < ITEM_COUNT; i++) {
		itemRegistry[i].mesh = (Mesh){ 0 };
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