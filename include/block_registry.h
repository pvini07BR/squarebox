#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <raylib.h>

typedef enum {
	BLOCK_AIR,
	BLOCK_GRASS_BLOCK,
	BLOCK_DIRT,
	BLOCK_STONE,
	BLOCK_COBBLESTONE,
	BLOCK_WOODEN_PLANKS,
	BLOCK_WOOD_LOG,
	BLOCK_LEAVES,
	BLOCK_GLASS,
	BLOCK_LAMP,
	BLOCK_CHEST,
	BLOCK_DIRT_SLAB,
	BLOCK_STONE_SLAB,
	BLOCK_COBBLESTONE_SLAB,
	BLOCK_WOODEN_PLANKS_SLAB,
	BLOCK_STONE_STAIRS,
	BLOCK_COBBLESTONE_STAIRS,
	BLOCK_WOODEN_PLANKS_STAIRS,
	BLOCK_STONE_NUB,
	BLOCK_COBBLESTONE_NUB,
	BLOCK_WOODEN_PLANKS_NUB,
	BLOCK_COUNT
} BlockEnum;

typedef enum {
	// No trait is assigned to that block.
	BLOCK_TRAIT_NONE,

	// Will make the block be able to rotate.
	BLOCK_TRAIT_ROTATES,

	// This makes the block be interactable and store (a index to) a container.
	BLOCK_TRAIT_CONTAINER
} BlockTrait;

typedef enum {
	// Tells if the block is transparent.
	BLOCK_FLAG_TRANSPARENT = (1 << 0),
	
	// Tells if the block is solid. That means, if it will have collision or not.
	BLOCK_FLAG_SOLID = (1 << 1),

	// Tells if the block can flip horizontally randomly.
	BLOCK_FLAG_FLIP_H = (1 << 2),

	// Tells if the block can flip vertically randomly.
	BLOCK_FLAG_FLIP_V = (1 << 3)
} BlockFlag;

// Struct that tells the properties of a block.
// It is used through a index on the static registry array
// to obtain the necessary details.
typedef struct {
	// Index in the atlas texture to use.
	// See texture_atlas.c.
	size_t atlas_idx;
	// Index of the model in the block models array.
	// see block_models.c.
	size_t model_idx;
	// Enum value that tells what trait the block will have.
	// It tells if it will rotate, or have a container, etc.
	BlockTrait trait;
	// Bit flags that will tell if the block is transaprent, solid, etc.
	// see the BlockFlag enum.
	uint8_t flags;
	// If the value is greater than zero, then this block will emit light.
	// otherwise it will not.
	uint8_t lightLevel;
} BlockRegistry;

void block_registry_init();
BlockRegistry* br_get_block_registry(size_t idx);
void block_registry_free();

#endif