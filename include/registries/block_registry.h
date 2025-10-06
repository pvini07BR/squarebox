#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

#include "block_functions.h"
#include "block_state_bitfields.h"
#include "types.h"

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
	BLOCK_BOUNCY_BLOCK,
	BLOCK_ICE,
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
	BLOCK_SLAB_FRAME,
	BLOCK_STAIRS_FRAME,
	BLOCK_NUB_FRAME,
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

	// Tells if the block is replaceable, like water, grass, flowers, etc.
	BLOCK_FLAG_REPLACEABLE = (1 << 4),

	// Tells if entities will be able to climb on this block, like ladders or vines.
	BLOCK_FLAG_CLIMBABLE = (1 << 5),
	
	// Tells if you can place plants like grass or flowers on top of the block.
	BLOCK_FLAG_PLANTABLE = (1 << 6),

	// Tells if the block is affected by gravity, like sand or gravel.
	BLOCK_FLAG_GRAVITY_AFFECTED = (1 << 7),

	// Tells if the block is a liquid.
	// this will make the block render differently from
	// other blocks, and will render only if it is in the
	// block layer.
	BLOCK_FLAG_LIQUID = (1 << 8),

	// Tells if the block is bouncy.
	// whenever a entity lands on the block, it will bounce
	// away the vertical velocity with some loss.
	BLOCK_FLAG_BOUNCY = (1 << 9),

	// Tells if the block is slippery.
	// it will make entities have less friction.
	BLOCK_FLAG_SLIPPERY = (1 << 10)
} BlockFlag;

typedef enum {
	BLOCK_LIGHT_TRANSPARENT = -1,
	BLOCK_LIGHT_NONE = 0,
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

// Struct that tells the properties of a block.
// It is used through a index on the static registry array
// to obtain the necessary details.
typedef struct {
	// You NEED to define the variant generator, otherwise the game will crash!!!
	// it is used to determine what model to be rendered for that block.
	BlockVariantGenerator variant_generator;
	BlockInteractionCallback interact_callback;
	BlockStateResolver state_resolver;
	BlockDestroyCallback destroy_callback;
	// If the tick callback is defined, the block will be added to the ticking list.
	BlockTickCallback tick_callback;
	// If the overlay draw function is defined, it will draw whatever the function tells to draw on the top
	// of the block.
	BlockOverlayRender overlay_draw;

	// Bit flags that will tell if the block is transaprent, solid, etc.
	// see the BlockFlag enum.
	int flags;
	// This value tells if the block is transparent, opaque or emit light.
	BlockLight lightLevel;

	// This member variable will only be used if tick_callback is defined.
	// 
	// In a value between 1-20, it determines the nth tick this block will tick, in a 20-tick cycle.
	// for example, when setting the value to 5, the block will tick every 5th tick.
	uint8_t tick_speed;

	// Determines if this block has states that can be selected manually on the game.
	// the array is an array of states. Depending on how a state is interpreted for a block,
	// it can be an array of numbers or an array of bitwise modified numbers.
	// See block_state_bitfields to see how uint8_t numbers becomes states with multiple members.
	uint8_t selectable_state_count;
	uint8_t selectable_states[4];
} BlockRegistry;

void block_registry_init();
BlockRegistry* br_get_block_registry(size_t idx);
void block_registry_free();

#define QUICK_VARIANT(name, atlas, model, collider, rot, uvlock) \
    static inline BlockVariant variant_##name(uint8_t state) { \
		(void)state; \
		return (BlockVariant) {\
			.atlas_idx=(atlas), \
			.model_idx=(model), \
			.collider_idx=(collider), \
			.rotation=(rot), \
			.uv_lock=(uvlock) \
		}; \
	}

#define QUICK_BLOCK(name, atlas) QUICK_VARIANT(name, atlas, BLOCK_MODEL_QUAD, BLOCK_COLLIDER_QUAD, 0, false)
#define QUICK_SLAB(name, atlas) QUICK_ROTATABLE_VARIANT(name, atlas, BLOCK_MODEL_SLAB, BLOCK_COLLIDER_SLAB, true)
#define QUICK_STAIRS(name, atlas) QUICK_ROTATABLE_VARIANT(name, atlas, BLOCK_MODEL_STAIRS, BLOCK_COLLIDER_STAIRS, true)
#define QUICK_NUB(name, atlas) QUICK_ROTATABLE_VARIANT(name, atlas, BLOCK_MODEL_NUB, BLOCK_COLLIDER_NUB, true)

QUICK_BLOCK(grass_block, ATLAS_GRASS_BLOCK)
QUICK_BLOCK(dirt, ATLAS_DIRT_BLOCK)
QUICK_BLOCK(sand, ATLAS_SAND)
QUICK_BLOCK(stone, ATLAS_STONE)
QUICK_BLOCK(cobblestone, ATLAS_COBBLESTONE)
QUICK_BLOCK(wooden_planks, ATLAS_WOODEN_PLANKS)

static inline BlockVariant variant_wood_log(uint8_t state) {
	return (BlockVariant) {
		.atlas_idx=ATLAS_WOOD_LOG,
		.model_idx=BLOCK_MODEL_QUAD,
		.collider_idx=BLOCK_COLLIDER_QUAD,
		.rotation= (state % 2),
		.uv_lock=false
	};
}

QUICK_BLOCK(leaves, ATLAS_LEAVES)
QUICK_BLOCK(glass, ATLAS_GLASS)
QUICK_BLOCK(lamp, ATLAS_LAMP)
QUICK_BLOCK(chest, ATLAS_CHEST)
QUICK_BLOCK(bouncy_block, ATLAS_BOUNCY_BLOCK)
QUICK_BLOCK(ice, ATLAS_ICE)
QUICK_BLOCK(wool, ATLAS_WOOL)
QUICK_BLOCK(orange_wool, ATLAS_ORANGE_WOOL)
QUICK_BLOCK(magenta_wool, ATLAS_MAGENTA_WOOL)
QUICK_BLOCK(light_blue_wool, ATLAS_LIGHT_BLUE_WOOL)
QUICK_BLOCK(yellow_wool, ATLAS_YELLOW_WOOL)
QUICK_BLOCK(lime_wool, ATLAS_LIME_WOOL)
QUICK_BLOCK(pink_wool, ATLAS_PINK_WOOL)
QUICK_BLOCK(gray_wool, ATLAS_GRAY_WOOL)
QUICK_BLOCK(light_gray_wool, ATLAS_LIGHT_GRAY_WOOL)
QUICK_BLOCK(cyan_wool, ATLAS_CYAN_WOOL)
QUICK_BLOCK(purple_wool, ATLAS_PURPLE_WOOL)
QUICK_BLOCK(blue_wool, ATLAS_BLUE_WOOL)
QUICK_BLOCK(brown_wool, ATLAS_BROWN_WOOL)
QUICK_BLOCK(green_wool, ATLAS_GREEN_WOOL)
QUICK_BLOCK(red_wool, ATLAS_RED_WOOL)
QUICK_BLOCK(black_wool, ATLAS_BLACK_WOOL)
QUICK_BLOCK(grass, ATLAS_GRASS)
QUICK_BLOCK(flower, ATLAS_FLOWER)
QUICK_BLOCK(pebbles, ATLAS_PEBBLES)

static inline BlockVariant variant_wooden_fence(uint8_t state) {
	TextureAtlasEnum tex = ATLAS_FENCE;
	tex += state;

	return (BlockVariant) {
		.atlas_idx=tex,
		.model_idx=BLOCK_MODEL_QUAD,
		.collider_idx=BLOCK_COLLIDER_QUAD,
		.rotation=0,
		.uv_lock=false
	};
}

QUICK_BLOCK(ladders, ATLAS_LADDERS)

static inline BlockVariant variant_trapdoor(uint8_t state) {
	TrapdoorState* trapState = (TrapdoorState*)&state;

	return (BlockVariant) {
		.atlas_idx = ATLAS_TRAPDOOR,
		.model_idx = BLOCK_MODEL_QUAD,
		.collider_idx = BLOCK_COLLIDER_TRAPDOOR,
		.rotation = trapState->rotation + trapState->open,
		.uv_lock = false
	};
}

static inline BlockVariant variant_sign(uint8_t state) {
	return (BlockVariant) {
		.atlas_idx=ATLAS_SIGN + (state % 2),
		.model_idx=BLOCK_MODEL_QUAD,
		.collider_idx=BLOCK_COLLIDER_QUAD,
		.rotation=0,
		.uv_lock=false
	};
}

static inline BlockVariant variant_torch(uint8_t state) {
	return (BlockVariant) {
		.atlas_idx=ATLAS_TORCH,
		.model_idx = BLOCK_MODEL_TORCH + (state % 3),
		.collider_idx=BLOCK_COLLIDER_QUAD,
		.rotation = 0,
		.uv_lock=true
	};
}

static inline BlockVariant variant_slab_frame(uint8_t state) {
	FrameBlockState* s = (FrameBlockState*)&state;

	TextureAtlasEnum atlas = ATLAS_SLAB_FRAME;
	if (s->blockIdx > 0) {
		atlas = br_get_block_registry(s->blockIdx)->variant_generator(0).atlas_idx;
	}

	return (BlockVariant) {
		.atlas_idx = atlas,
		.model_idx = BLOCK_MODEL_SLAB,
		.collider_idx = BLOCK_COLLIDER_SLAB,
		.rotation = s->rotation,
		.uv_lock = s->blockIdx > 0
	};
}

static inline BlockVariant variant_stairs_frame(uint8_t state) {
	FrameBlockState* s = (FrameBlockState*)&state;

	TextureAtlasEnum atlas = ATLAS_STAIRS_FRAME;
	if (s->blockIdx > 0) {
		atlas = br_get_block_registry(s->blockIdx)->variant_generator(0).atlas_idx;
	}

	return (BlockVariant) {
		.atlas_idx = atlas,
		.model_idx = BLOCK_MODEL_STAIRS,
		.collider_idx = BLOCK_COLLIDER_STAIRS,
		.rotation = s->rotation,
		.uv_lock = s->blockIdx > 0
	};
}

static inline BlockVariant variant_nub_frame(uint8_t state) {
	FrameBlockState* s = (FrameBlockState*)&state;

	TextureAtlasEnum atlas = ATLAS_NUB_FRAME;
	if (s->blockIdx > 0) {
		atlas = br_get_block_registry(s->blockIdx)->variant_generator(0).atlas_idx;
	}

	return (BlockVariant) {
		.atlas_idx = atlas,
		.model_idx = BLOCK_MODEL_NUB,
		.collider_idx = BLOCK_COLLIDER_NUB,
		.rotation = s->rotation,
		.uv_lock = s->blockIdx > 0
	};
}

#endif