#include "registries/item_registry.h"
#include "registries/block_registry.h"
#include "registries/block_models.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>

static ItemRegistry* itemRegistry = NULL;

void item_registry_init() {
	itemRegistry = calloc(ITEM_COUNT, sizeof(ItemRegistry));
	if (itemRegistry == NULL) {
		TraceLog(LOG_ERROR, "Could not allocate memory for the item registry.");
		return;
	};
	
	itemRegistry[ITEM_NONE] = (ItemRegistry){
		.name = "No Item",
		.atlas_idx = ATLAS_GRASS_BLOCK,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_AIR
	};

	itemRegistry[ITEM_GRASS_BLOCK] = (ItemRegistry){
		.name = "Grass Block",
		.atlas_idx = ATLAS_GRASS_BLOCK,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_GRASS_BLOCK
	};

	itemRegistry[ITEM_DIRT_BLOCK] = (ItemRegistry){
		.name = "Dirt Block",
		.atlas_idx = ATLAS_DIRT,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_DIRT
	};

	itemRegistry[ITEM_SAND_BLOCK] = (ItemRegistry){
		.name = "Sand Block",
		.atlas_idx = ATLAS_SAND,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_SAND
	};

	itemRegistry[ITEM_STONE_BLOCK] = (ItemRegistry){
		.name = "Stone Block",
		.atlas_idx = ATLAS_STONE,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_STONE
	};

	itemRegistry[ITEM_COBBLESTONE] = (ItemRegistry){
		.name = "Cobblestone",
		.atlas_idx = ATLAS_COBBLESTONE,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_COBBLESTONE
	};

	itemRegistry[ITEM_WOODEN_PLANKS] = (ItemRegistry){
		.name = "Wooden Planks",
		.atlas_idx = ATLAS_WOODEN_PLANKS,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_WOODEN_PLANKS
	};

	itemRegistry[ITEM_WOOD_LOG] = (ItemRegistry){
		.name = "Wood Log",
		.atlas_idx = ATLAS_WOOD_LOG,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_WOOD_LOG
	};

	itemRegistry[ITEM_LEAVES] = (ItemRegistry){
		.name = "Leaves",
		.atlas_idx = ATLAS_LEAVES,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_LEAVES
	};

	itemRegistry[ITEM_GLASS] = (ItemRegistry){
		.name = "Glass Block",
		.atlas_idx = ATLAS_GLASS,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_GLASS
	};

	itemRegistry[ITEM_LAMP] = (ItemRegistry){
		.name = "Lamp Block",
		.atlas_idx = ATLAS_LAMP,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_LAMP
	};

	itemRegistry[ITEM_CHEST] = (ItemRegistry){
		.name = "Chest",
		.atlas_idx = ATLAS_CHEST,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_CHEST
	};

	itemRegistry[ITEM_BOUNCY_BLOCK] = (ItemRegistry){
		.name = "Bouncy Block",
		.atlas_idx = ATLAS_BOUNCY_BLOCK,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_BOUNCY_BLOCK
	};

	itemRegistry[ITEM_ICE] = (ItemRegistry){
		.name = "Ice",
		.atlas_idx = ATLAS_ICE,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_ICE
	};

	itemRegistry[ITEM_WOOL] = (ItemRegistry){
		.name = "Wool",
		.atlas_idx = ATLAS_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_WOOL
	};

	itemRegistry[ITEM_ORANGE_WOOL] = (ItemRegistry){
		.name = "Orange Wool",
		.atlas_idx = ATLAS_ORANGE_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_ORANGE_WOOL
	};

	itemRegistry[ITEM_MAGENTA_WOOL] = (ItemRegistry){
		.name = "Magenta Wool",
		.atlas_idx = ATLAS_MAGENTA_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_MAGENTA_WOOL
	};

	itemRegistry[ITEM_LIGHT_BLUE_WOOL] = (ItemRegistry){
		.name = "Light Blue Wool",
		.atlas_idx = ATLAS_LIGHT_BLUE_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_LIGHT_BLUE_WOOL
	};

	itemRegistry[ITEM_YELLOW_WOOL] = (ItemRegistry){
		.name = "Yellow Wool",
		.atlas_idx = ATLAS_YELLOW_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_YELLOW_WOOL
	};

	itemRegistry[ITEM_LIME_WOOL] = (ItemRegistry){
		.name = "Lime Wool",
		.atlas_idx = ATLAS_LIME_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_LIME_WOOL
	};

	itemRegistry[ITEM_PINK_WOOL] = (ItemRegistry){
		.name = "Pink Wool",
		.atlas_idx = ATLAS_PINK_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_PINK_WOOL
	};

	itemRegistry[ITEM_GRAY_WOOL] = (ItemRegistry){
		.name = "Gray Wool",
		.atlas_idx = ATLAS_GRAY_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_GRAY_WOOL
	};

	itemRegistry[ITEM_LIGHT_GRAY_WOOL] = (ItemRegistry){
		.name = "Light Gray Wool",
		.atlas_idx = ATLAS_LIGHT_GRAY_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_LIGHT_GRAY_WOOL
	};

	itemRegistry[ITEM_CYAN_WOOL] = (ItemRegistry){
		.name = "Cyan Wool",
		.atlas_idx = ATLAS_CYAN_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_CYAN_WOOL
	};

	itemRegistry[ITEM_PURPLE_WOOL] = (ItemRegistry){
		.name = "Purple Wool",
		.atlas_idx = ATLAS_PURPLE_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_PURPLE_WOOL
	};

	itemRegistry[ITEM_BLUE_WOOL] = (ItemRegistry){
		.name = "Blue Wool",
		.atlas_idx = ATLAS_BLUE_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_BLUE_WOOL
	};

	itemRegistry[ITEM_BROWN_WOOL] = (ItemRegistry){
		.name = "Brown Wool",
		.atlas_idx = ATLAS_BROWN_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_BROWN_WOOL
	};

	itemRegistry[ITEM_GREEN_WOOL] = (ItemRegistry){
		.name = "Green Wool",
		.atlas_idx = ATLAS_GREEN_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_GREEN_WOOL
	};

	itemRegistry[ITEM_RED_WOOL] = (ItemRegistry){
		.name = "Red Wool",
		.atlas_idx = ATLAS_RED_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_RED_WOOL
	};

	itemRegistry[ITEM_BLACK_WOOL] = (ItemRegistry){
		.name = "Black Wool",
		.atlas_idx = ATLAS_BLACK_WOOL,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_BLACK_WOOL
	};

	itemRegistry[ITEM_GRASS] = (ItemRegistry){
		.name = "Grass",
		.atlas_idx = ATLAS_GRASS,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_GRASS
	};

	itemRegistry[ITEM_FLOWER] = (ItemRegistry){
		.name = "Flower",
		.atlas_idx = ATLAS_FLOWER,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_FLOWER
	};

	itemRegistry[ITEM_PEBBLES] = (ItemRegistry){
		.name = "Pebbles",
		.atlas_idx = ATLAS_PEBBLES,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_PEBBLES
	};

	itemRegistry[ITEM_WOODEN_FENCE] = (ItemRegistry){
		.name = "Wooden Fence",
		.atlas_idx = ATLAS_WOODEN_FENCE,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_WOODEN_FENCE
	};

	itemRegistry[ITEM_LADDERS] = (ItemRegistry){
		.name = "Ladders",
		.atlas_idx = ATLAS_LADDERS,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_LADDERS
	};

	itemRegistry[ITEM_TRAPDOOR] = (ItemRegistry){
		.name = "Trapdoor",
		.atlas_idx = ATLAS_TRAPDOOR,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_TRAPDOOR
	};

	itemRegistry[ITEM_SIGN] = (ItemRegistry){
		.name = "Sign",
		.atlas_idx = ATLAS_SIGN,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_SIGN
	};

	itemRegistry[ITEM_TORCH] = (ItemRegistry){
		.name = "Torch",
		.atlas_idx = ATLAS_TORCH,
		.model_idx = BLOCK_MODEL_TORCH,
		.blockId = BLOCK_TORCH
	};

	itemRegistry[ITEM_WATER_BUCKET] = (ItemRegistry){
		.name = "Water Bucket",
		.atlas_idx = ATLAS_WATER_BUCKET,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_WATER_SOURCE,
		.placingFlags = ITEM_PLACE_FLAG_NOT_WALL
	};

	itemRegistry[ITEM_SLAB_FRAME] = (ItemRegistry){
		.name = "Slab Frame",
		.atlas_idx = ATLAS_SLAB_FRAME,
		.model_idx = BLOCK_MODEL_SLAB,
		.blockId = BLOCK_SLAB_FRAME
	};

	itemRegistry[ITEM_STAIRS_FRAME] = (ItemRegistry){
		.name = "Stairs Frame",
		.atlas_idx = ATLAS_STAIRS_FRAME,
		.model_idx = BLOCK_MODEL_STAIRS,
		.blockId = BLOCK_STAIRS_FRAME
	};

	itemRegistry[ITEM_NUB_FRAME] = (ItemRegistry){
		.name = "Nub Frame",
		.atlas_idx = ATLAS_NUB_FRAME,
		.model_idx = BLOCK_MODEL_NUB,
		.blockId = BLOCK_NUB_FRAME
	};

	itemRegistry[ITEM_POWER_WIRE] = (ItemRegistry){
		.name = "Power Wire",
		.atlas_idx = ATLAS_POWER_WIRE,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_POWER_WIRE
	};

	itemRegistry[ITEM_BATTERY] = (ItemRegistry) {
		.name = "Battery",
		.atlas_idx = ATLAS_BATTERY,
		.model_idx = BLOCK_MODEL_QUAD,
		.blockId = BLOCK_BATTERY
	};

	for (int i = 0; i < ITEM_COUNT; i++) {
		ItemRegistry* irg = &itemRegistry[i];
		irg->mesh = (Mesh){ 0 };
		block_models_build_mesh(
			&irg->mesh,
			(BlockVariant) {
				.atlas_idx = irg->atlas_idx,
				.atlas_variant = 0,
				.model_idx = irg->model_idx 
			}
		);
	}
}

ItemRegistry* ir_get_item_registry(size_t idx)
{
	if (idx > ITEM_COUNT - 1) return NULL;
	return &itemRegistry[idx];
}

void item_registry_free() {
	for (int i = 0; i < ITEM_COUNT; i++) { UnloadMesh(itemRegistry[i].mesh); }
	if (itemRegistry) free(itemRegistry);
}