#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#include "chunk.h"
#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

#define MAX_BLOCK_VARIANTS 4

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
	BLOCK_TORCH,
	BLOCK_WATER_SOURCE,
	BLOCK_WATER_FLOWING,
	BLOCK_COUNT
} BlockEnum;

typedef enum {
	// Tells if the block is transparent.
	BLOCK_FLAG_TRANSPARENT = (1 << 0),
	
	// Tells if the block is solid. That means, if it will have collision or not.
	BLOCK_FLAG_SOLID = (1 << 1),

	// Tells if the block is a full quad.
	BLOCK_FLAG_FULL_BLOCK = (1 << 2),

	// Tells if the block can flip horizontally randomly.
	BLOCK_FLAG_FLIP_H = (1 << 3),

	// Tells if the block can flip vertically randomly.
	BLOCK_FLAG_FLIP_V = (1 << 4),

	// Tells if the state of this block can be determined
	// instead of calculated.
	BLOCK_FLAG_STATE_MUTABLE = (1 << 5),

	// Tells if the block is replaceable, like water, grass, flowers, etc.
	BLOCK_FLAG_REPLACEABLE = (1 << 6),

	// Tells if the block is a liquid source.
	// this will make the block render differently from
	// other blocks, and will render only if it is in the
	// block layer.
	BLOCK_FLAG_LIQUID_SOURCE = (1 << 7),
	// Tells if the block is a flowing liquid.
	// this will make the block render differently from
	// other blocks, and will render only if it is in the
	// block layer.
	BLOCK_FLAG_LIQUID_FLOWING = (1 << 8)
} BlockFlag;

typedef struct {
	// Index in the atlas texture to use.
	// See texture_atlas.c.
	size_t atlas_idx;
	// Index of the model in the block models array.
	// see block_models.c.
	size_t model_idx;
	// Determines if the model should flip horizontally in this block state.
	bool flipH;
	// Determines if the model should flip vertically in this block state.
	bool flipV;
	// Determines what rotation the model should have in this block state.
	uint8_t rotation;
} BlockVariant;

// Struct that tells the properties of a block.
// It is used through a index on the static registry array
// to obtain the necessary details.
typedef struct {
	// Number of variants this block has.
	size_t variant_count;
	// An array of variants for that block.
	// it will be indexed with the state member variable
	// in the BlockInstance struct.
	BlockVariant variants[MAX_BLOCK_VARIANTS];
	// Bit flags that will tell if the block is transaprent, solid, etc.
	// see the BlockFlag enum.
	int flags;
	// If the value is greater than zero, then this block will emit light.
	// otherwise it will not.
	uint8_t lightLevel;
	// Function pointer to a function that will be called when the block is interacted with.
	// returns a bool value to indicate if the interaction was successful or not.
	bool (*interact_callback)(BlockInstance* inst, Chunk* chunk);
	// Function pointer to a function that will resolve the state of the block.
	// returns a bool value to indicate if the block can be placed or not.
	bool (*state_resolver)(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
	// Function pointer to a function that will be called when the block is destroyed.
	void (*destroy_callback)(BlockInstance* inst, Chunk* chunk, uint8_t idx);
} BlockRegistry;

void block_registry_init();
BlockRegistry* br_get_block_registry(size_t idx);
BlockVariant br_get_block_variant(size_t reg_idx, size_t variant_idx);
void block_registry_free();

#endif