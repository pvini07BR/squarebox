#ifndef ITEM_REGISTRY_H
#define ITEM_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

#include "texture_atlas.h"
#include "registries/block_models.h"
#include "registries/block_registry.h"

typedef enum {
	ITEM_NONE,
	ITEM_GRASS_BLOCK,
	ITEM_DIRT_BLOCK,
	ITEM_SAND_BLOCK,
	ITEM_STONE_BLOCK,
	ITEM_COBBLESTONE,
	ITEM_WOODEN_PLANKS,
	ITEM_WOOD_LOG,
	ITEM_LEAVES,
	ITEM_GLASS,
	ITEM_LAMP,
	ITEM_CHEST,
	ITEM_BOUNCY_BLOCK,
	ITEM_ICE,
	ITEM_WOOL,
	ITEM_ORANGE_WOOL,
	ITEM_MAGENTA_WOOL,
	ITEM_LIGHT_BLUE_WOOL,
	ITEM_YELLOW_WOOL,
	ITEM_LIME_WOOL,
	ITEM_PINK_WOOL,
	ITEM_GRAY_WOOL,
	ITEM_LIGHT_GRAY_WOOL,
	ITEM_CYAN_WOOL,
	ITEM_PURPLE_WOOL,
	ITEM_BLUE_WOOL,
	ITEM_BROWN_WOOL,
	ITEM_GREEN_WOOL,
	ITEM_RED_WOOL,
	ITEM_BLACK_WOOL,
	ITEM_GRASS,
	ITEM_FLOWER,
	ITEM_PEBBLES,
	ITEM_WOODEN_FENCE,
	ITEM_LADDERS,
	ITEM_TRAPDOOR,
	ITEM_SIGN,
	ITEM_TORCH,
	ITEM_WATER_BUCKET,
	ITEM_SLAB_FRAME,
	ITEM_STAIRS_FRAME,
	ITEM_NUB_FRAME,
	ITEM_POWER_WIRE,
	ITEM_COUNT
} ItemEnum;

typedef enum {
	ITEM_PLACE_FLAG_NOT_BLOCK = (1 << 0),
	ITEM_PLACE_FLAG_NOT_WALL = (1 << 1)
} ItemPlacingFlags;

typedef struct {
	const char* name;
	TextureAtlasEnum atlas_idx;
	BlockModelEnum model_idx;
	Mesh mesh;
	
	// Set to BLOCK_AIR if this item isn't going to place any blocks.
	BlockEnum blockId;
	ItemPlacingFlags placingFlags;
} ItemRegistry;

void item_registry_init();
ItemRegistry* ir_get_item_registry(size_t idx);
void item_registry_free();

#endif