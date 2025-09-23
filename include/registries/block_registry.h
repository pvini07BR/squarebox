#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

#include "block_models.h"
#include "block_functions.h"
#include "block_colliders.h"

#define MAX_BLOCK_VARIANTS 8

typedef enum {
	BLOCK_AIR,
	BLOCK_GRASS_BLOCK,
	BLOCK_DIRT,
	BLOCK_SAND,
	BLOCK_STONE,
	BLOCK_COBBLESTONE,
	BLOCK_WOODEN_PLANKS,
	BLOCK_WOOD_LOG,
	BLOCK_LEAVES,
	BLOCK_GLASS,
	BLOCK_LAMP,
	BLOCK_CHEST,
	BLOCK_WOOL,
	BLOCK_ORANGE_WOOL,
	BLOCK_MAGENTA_WOOL,
	BLOCK_LIGHT_BLUE_WOOL,
	BLOCK_YELLOW_WOOL,
	BLOCK_LIME_WOOL,
	BLOCK_PINK_WOOL,
	BLOCK_GRAY_WOOL,
	BLOCK_LIGHT_GRAY_WOOL,
	BLOCK_CYAN_WOOL,
	BLOCK_PURPLE_WOOL,
	BLOCK_BLUE_WOOL,
	BLOCK_BROWN_WOOL,
	BLOCK_GREEN_WOOL,
	BLOCK_RED_WOOL,
	BLOCK_BLACK_WOOL,
	BLOCK_GRASS,
	BLOCK_FLOWER,
	BLOCK_PEBBLES,
	BLOCK_WOODEN_FENCE,
	BLOCK_LADDERS,
	BLOCK_TRAPDOOR,
	BLOCK_SIGN,
	BLOCK_TORCH,
	BLOCK_WATER_SOURCE,
	BLOCK_WATER_FLOWING,
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
	// Tells if the block is solid. That means, if it will have collision or not.
	BLOCK_FLAG_SOLID = (1 << 0),

	// Tells if the block is a full quad.
	BLOCK_FLAG_FULL_BLOCK = (1 << 1),

	// Tells if the block can flip horizontally randomly.
	BLOCK_FLAG_FLIP_H = (1 << 2),

	// Tells if the block can flip vertically randomly.
	BLOCK_FLAG_FLIP_V = (1 << 3),

	// Tells if the state of this block can be manually selected.
	// see BlockStateSelector in block_functions.h.
	BLOCK_FLAG_STATE_SELECTABLE = (1 << 4),

	// Tells if the block is replaceable, like water, grass, flowers, etc.
	BLOCK_FLAG_REPLACEABLE = (1 << 5),

	// Tells if entities will be able to climb on this block, like ladders or vines.
	BLOCK_FLAG_CLIMBABLE = (1 << 6),
	
	// Tells if you can place plants like grass or flowers on top of the block.
	BLOCK_FLAG_PLANTABLE = (1 << 7),

	// Tells if the block is affected by gravity, like sand or gravel.
	BLOCK_FLAG_GRAVITY_AFFECTED = (1 << 8),

	// Tells if the block is a liquid.
	// this will make the block render differently from
	// other blocks, and will render only if it is in the
	// block layer.
	BLOCK_FLAG_LIQUID = (1 << 9),
} BlockFlag;

typedef enum {
	BLOCK_LIGHT_TRANSPARENT = -1,
	BLOCK_LIGHT_NO_LIGHT = 0,
	BLOCK_LIGHT_1 = 1,
	BLOCK_LIGHT_2 = 2,
	BLOCK_LIGHT_3 = 3,
	BLOCK_LIGHT_4 = 4,
	BLOCK_LIGHT_5 = 5,
	BLOCK_LIGHT_6 = 6,
	BLOCK_LIGHT_7 = 7,
	BLOCK_LIGHT_8 = 8,
	BLOCK_LIGHT_9 = 9,
	BLOCK_LIGHT_10 = 10,
	BLOCK_LIGHT_11 = 11,
	BLOCK_LIGHT_12 = 12,
	BLOCK_LIGHT_13 = 13,
	BLOCK_LIGHT_14 = 14,
	BLOCK_LIGHT_15 = 15
} BlockLight;

typedef struct {
	// Index in the atlas texture to use.
	// See texture_atlas.c.
	size_t atlas_idx;
	// Index of the model in the block models array.
	// see block_models.c.
	BlockModelEnum model_idx;
	// Index of the collider in the block colliders array.
	// see block_colliders.c.
	BlockColliderEnum collider_idx;
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
	// This value tells if the block is transparent, opaque or emit light.
	BlockLight lightLevel;

	// These two fields will be used only when selecting the state before placing a block.
	size_t state_count;
	// If the state selector is defined, it will use that to determine what states will be used.
	// otherwise, it will use the state value to directly index the variants.
	BlockStateSelector state_selector;

	BlockVariantSelector variant_selector;
	BlockInteractionCallback interact_callback;
	BlockStateResolver state_resolver;
	BlockDestroyCallback destroy_callback;
	// If the tick callback is defined, the block will be added to the ticking list.
	BlockTickCallback tick_callback;
	// If the overlay draw function is defined, it will draw whatever the function tells to draw on the top
	// of the block.
	BlockOverlayRender overlay_draw;

	// This member variable will only be used if tick_callback is defined.
	// 
	// In a value between 1-20, it determines the nth tick this block will tick, in a 20-tick cycle.
	// for example, when setting the value to 5, the block will tick every 5th tick.
	uint8_t tick_speed;
} BlockRegistry;

void block_registry_init();
BlockRegistry* br_get_block_registry(size_t idx);
BlockVariant br_get_block_variant(size_t reg_idx, size_t variant_idx);
void block_registry_free();

#endif