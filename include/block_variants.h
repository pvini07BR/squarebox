#ifndef BLOCK_VARIANTS_H
#define BLOCK_VARIANTS_H

#include "block_states.h"
#include "raylib.h"
#include "registries/block_registry.h"
#include "types.h"
#include <raymath.h>

#define QUICK_VARIANT(name, atlas, atlas_var, model, collider, rot, uvlock) \
    static inline BlockVariant variant_##name(uint8_t state) { \
		(void)state; \
		return (BlockVariant) {\
			.atlas_idx=(atlas), \
			.atlas_variant=(atlas_var), \
			.model_idx=(model), \
			.collider_idx=(collider), \
			.rotation=(rot), \
			.uv_lock=(uvlock), \
			.tint=WHITE \
		}; \
	}

#define QUICK_BLOCK(name, atlas) QUICK_VARIANT(name, atlas, 0, BLOCK_MODEL_QUAD, BLOCK_COLLIDER_QUAD, 0, false)

QUICK_BLOCK(grass_block, ATLAS_GRASS_BLOCK)
QUICK_BLOCK(dirt, ATLAS_DIRT)
QUICK_BLOCK(sand, ATLAS_SAND)
QUICK_BLOCK(stone, ATLAS_STONE)
QUICK_BLOCK(cobblestone, ATLAS_COBBLESTONE)
QUICK_BLOCK(wooden_planks, ATLAS_WOODEN_PLANKS)

static inline BlockVariant variant_wood_log(uint8_t state) {
	return (BlockVariant) {
		.atlas_idx = ATLAS_WOOD_LOG,
		.atlas_variant = state == LOGLIKE_BLOCK_STATE_FORWARD,
		.model_idx = BLOCK_MODEL_QUAD,
		.collider_idx = BLOCK_COLLIDER_QUAD,
		.rotation = state == LOGLIKE_BLOCK_STATE_HORIZONTAL,
		.uv_lock = false,
		.tint = WHITE
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
	return (BlockVariant) {
		.atlas_idx = ATLAS_WOODEN_FENCE,
		.atlas_variant = state,
		.model_idx = BLOCK_MODEL_QUAD,
		.collider_idx = BLOCK_COLLIDER_QUAD,
		.rotation = 0,
		.uv_lock = false,
		.tint = WHITE
	};
}

QUICK_BLOCK(ladders, ATLAS_LADDERS)

static inline BlockVariant variant_trapdoor(uint8_t state) {
	TrapdoorState* trapState = (TrapdoorState*)&state;

	return (BlockVariant) {
		.atlas_idx = ATLAS_TRAPDOOR,
		.atlas_variant = 0,
		.model_idx = BLOCK_MODEL_QUAD,
		.collider_idx = BLOCK_COLLIDER_TRAPDOOR,
		.rotation = trapState->rotation + trapState->open,
		.uv_lock = false,
		.tint = WHITE,
	};
}

static inline BlockVariant variant_sign(uint8_t state) {
	return (BlockVariant) {
		.atlas_idx = ATLAS_SIGN,
		.atlas_variant = state % 2,
		.model_idx = BLOCK_MODEL_QUAD,
		.collider_idx = BLOCK_COLLIDER_QUAD,
		.rotation = 0,
		.uv_lock = false,
		.tint = WHITE
	};
}

static inline BlockVariant variant_torch(uint8_t state) {
	return (BlockVariant) {
		.atlas_idx = ATLAS_TORCH,
		.model_idx = BLOCK_MODEL_TORCH + (state % 3),
		.collider_idx = BLOCK_COLLIDER_QUAD,
		.rotation = 0,
		.uv_lock = true,
		.tint = WHITE
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
		.atlas_variant = 0,
		.model_idx = BLOCK_MODEL_SLAB,
		.collider_idx = BLOCK_COLLIDER_SLAB,
		.rotation = s->rotation,
		.uv_lock = s->blockIdx > 0,
		.tint = WHITE
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
		.atlas_variant = 0,
		.model_idx = BLOCK_MODEL_STAIRS,
		.collider_idx = BLOCK_COLLIDER_STAIRS,
		.rotation = s->rotation,
		.uv_lock = s->blockIdx > 0,
		.tint = WHITE
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
		.atlas_variant = 0,
		.model_idx = BLOCK_MODEL_NUB,
		.collider_idx = BLOCK_COLLIDER_NUB,
		.rotation = s->rotation,
		.uv_lock = s->blockIdx > 0,
		.tint = WHITE
	};
}

static inline BlockVariant variant_power_wire(uint8_t state) {
	PowerWireState* s = (PowerWireState*)&state;

	float fPower = (float)s->power / 15.0f;

	return (BlockVariant) {
		.atlas_idx = ATLAS_POWER_WIRE,
		.atlas_variant = s->right << 0 | s->up << 1 | s->left << 2 | s->down << 3,
		.model_idx = BLOCK_MODEL_QUAD,
		.collider_idx = BLOCK_COLLIDER_QUAD,
		.rotation = 0,
		.uv_lock = false,
		.tint = (Color) { Lerp(64, 255, fPower), 0, 0, 255}
	};
}

static inline BlockVariant variant_battery(uint8_t state) {
	return (BlockVariant) {
		.atlas_idx = ATLAS_BATTERY,
		.atlas_variant = state == LOGLIKE_BLOCK_STATE_FORWARD,
		.model_idx = BLOCK_MODEL_QUAD,
		.collider_idx = BLOCK_COLLIDER_QUAD,
		.rotation = state == LOGLIKE_BLOCK_STATE_HORIZONTAL,
		.uv_lock = false,
		.tint = WHITE
	};
}

static inline BlockVariant variant_power_repeater(uint8_t state) {
	PowerRepeaterState* s = (PowerRepeaterState*)&state;

	return (BlockVariant) {
		.atlas_idx = ATLAS_POWER_REPEATER,
		.atlas_variant = s->powered,
		.model_idx = BLOCK_MODEL_QUAD,
		.collider_idx = BLOCK_COLLIDER_QUAD,
		.rotation = s->rotation,
		.uv_lock = false,
		.tint = WHITE
	};
}

static inline BlockVariant variant_powered_lamp(uint8_t state) {
	return (BlockVariant) {
		.atlas_idx = ATLAS_POWERED_LAMP,
		.atlas_variant = state % 2,
		.model_idx = BLOCK_MODEL_QUAD,
		.collider_idx = BLOCK_COLLIDER_QUAD,
		.rotation = 0,
		.uv_lock = false,
		.tint = WHITE
	};
}

#endif