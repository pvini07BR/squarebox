#ifndef ITEM_REGISTRY_H
#define ITEM_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

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
	ITEM_GRASS,
	ITEM_FLOWER,
	ITEM_PEBBLES,
	ITEM_WOODEN_FENCE,
	ITEM_LADDERS,
	ITEM_TRAPDOOR,
	ITEM_TORCH,
	ITEM_WATER_BUCKET,
	ITEM_DIRT_SLAB,
	ITEM_STONE_SLAB,
	ITEM_COBBLESTONE_SLAB,
	ITEM_WOODEN_PLANKS_SLAB,
	ITEM_STONE_STAIRS,
	ITEM_COBBLESTONE_STAIRS,
	ITEM_WOODEN_PLANKS_STAIRS,
	ITEM_STONE_NUB,
	ITEM_COBBLESTONE_NUB,
	ITEM_WOODEN_PLANKS_NUB,
	ITEM_COUNT
} ItemEnum;

typedef enum {
	ITEM_PLACE_FLAG_NOT_BLOCK = (1 << 0),
	ITEM_PLACE_FLAG_NOT_WALL = (1 << 1)
} ItemPlacingFlags;

typedef struct {
	const char* name;
	size_t atlas_idx;
	size_t model_idx;
	Mesh mesh;
	
	// Set 0 if this item isn't going to place any blocks.
	uint8_t blockId;
	ItemPlacingFlags placingFlags;
} ItemRegistry;

void item_registry_init();
ItemRegistry* ir_get_item_registry(size_t idx);
void item_registry_free();

#endif