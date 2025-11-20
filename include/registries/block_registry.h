#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

#include "block_functions.h"

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
	BLOCK_POWER_WIRE,
	BLOCK_BATTERY,
	BLOCK_POWER_REPEATER,
	BLOCK_POWERED_LAMP,
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
	BLOCK_FLAG_SLIPPERY = (1 << 10),

	// Tells if the block will be triggered a block update
	// when there is a power update nearby.
	BLOCK_FLAG_POWER_TRIGGERED = (1 << 1),
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
	BlockFreeData free_data;

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
	uint8_t selectable_states[5];
} BlockRegistry;

void block_registry_init();
BlockRegistry* br_get_block_registry(size_t idx);
void block_registry_free();

#endif