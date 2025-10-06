#include "registries/block_registry.h"
#include "block_functions.h"

#include <stdlib.h>
#include <stdio.h>

static BlockRegistry* blockRegistry = NULL;

void block_registry_init() {
    blockRegistry = calloc(BLOCK_COUNT, sizeof(BlockRegistry));
    if (blockRegistry == NULL) {
        TraceLog(LOG_ERROR, "Could not allocate memory for the block registry.");
        return;
    };

    blockRegistry[BLOCK_AIR] = (BlockRegistry){
        .variant_generator = variant_grass_block,
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_GRASS_BLOCK] = (BlockRegistry){
        .variant_generator = variant_grass_block,        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_PLANTABLE,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_DIRT] = (BlockRegistry){
        .variant_generator = variant_dirt,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_PLANTABLE,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_SAND] = (BlockRegistry){
        .variant_generator = variant_sand,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_GRAVITY_AFFECTED,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL,
        .tick_callback = falling_block_tick,
        .tick_speed = 1
    };

    blockRegistry[BLOCK_STONE] = (BlockRegistry){
        .variant_generator = variant_stone,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_COBBLESTONE] = (BlockRegistry){
        .variant_generator = variant_cobblestone,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOODEN_PLANKS] = (BlockRegistry){
        .variant_generator = variant_wooden_planks,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOOD_LOG] = (BlockRegistry){
        .variant_generator = variant_wood_log,
        .selectable_state_count = 2,
        .selectable_states = { 0, 1 },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_LEAVES] = (BlockRegistry){
        .variant_generator = variant_leaves,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_GLASS] = (BlockRegistry){
        .variant_generator = variant_glass,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_LAMP] = (BlockRegistry){
        .variant_generator = variant_lamp,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_15,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_CHEST] = (BlockRegistry){
        .variant_generator = variant_chest,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE,
        .interact_callback = on_chest_interact,
        .state_resolver = chest_solver,
        .destroy_callback = on_chest_destroy
    };

    blockRegistry[BLOCK_BOUNCY_BLOCK] = (BlockRegistry){
        .variant_generator = variant_bouncy_block,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_BOUNCY,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_ICE] = (BlockRegistry){
        .variant_generator = variant_ice,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_SLIPPERY,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT
    };

    blockRegistry[BLOCK_WOOL] = (BlockRegistry){
        .variant_generator = variant_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_ORANGE_WOOL] = (BlockRegistry){
        .variant_generator = variant_orange_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_MAGENTA_WOOL] = (BlockRegistry){
        .variant_generator = variant_magenta_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_LIGHT_BLUE_WOOL] = (BlockRegistry){
        .variant_generator = variant_light_blue_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_YELLOW_WOOL] = (BlockRegistry){
        .variant_generator = variant_yellow_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_LIME_WOOL] = (BlockRegistry){
        .variant_generator = variant_lime_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_PINK_WOOL] = (BlockRegistry){
        .variant_generator = variant_pink_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_GRAY_WOOL] = (BlockRegistry){
        .variant_generator = variant_gray_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_LIGHT_GRAY_WOOL] = (BlockRegistry){
        .variant_generator = variant_light_gray_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_CYAN_WOOL] = (BlockRegistry){
        .variant_generator = variant_cyan_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_PURPLE_WOOL] = (BlockRegistry){
        .variant_generator = variant_purple_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_BLUE_WOOL] = (BlockRegistry){
        .variant_generator = variant_blue_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_BROWN_WOOL] = (BlockRegistry){
        .variant_generator = variant_brown_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_GREEN_WOOL] = (BlockRegistry){
        .variant_generator = variant_green_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_RED_WOOL] = (BlockRegistry){
        .variant_generator = variant_red_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_BLACK_WOOL] = (BlockRegistry){
        .variant_generator = variant_black_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    blockRegistry[BLOCK_GRASS] = (BlockRegistry){
        .variant_generator = variant_grass,
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_FLIP_H,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
		.state_resolver = plant_block_resolver
    };

    blockRegistry[BLOCK_FLOWER] = (BlockRegistry){
        .variant_generator = variant_flower,
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = plant_block_resolver
    };

    blockRegistry[BLOCK_PEBBLES] = (BlockRegistry){
        .variant_generator = variant_pebbles,
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = grounded_block_resolver
    };

    blockRegistry[BLOCK_WOODEN_FENCE] = (BlockRegistry){
        .variant_generator = variant_wooden_fence,
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = fence_resolver
    };

    blockRegistry[BLOCK_LADDERS] = (BlockRegistry){
        .variant_generator = variant_ladders,
        .flags = BLOCK_FLAG_CLIMBABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
    };

    blockRegistry[BLOCK_TRAPDOOR] = (BlockRegistry){
        .variant_generator = variant_trapdoor,
        .selectable_state_count = 4,
        .selectable_states = {
            get_trapdoor_state(0, false),
            get_trapdoor_state(1, false),
            get_trapdoor_state(2, false),
            get_trapdoor_state(3, false)
        },
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NONE,
        .interact_callback = trapdoor_interact
    };

    blockRegistry[BLOCK_SIGN] = (BlockRegistry){
        .variant_generator = variant_sign,
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = sign_solver,
        .destroy_callback = on_sign_destroy,
        .overlay_draw = sign_text_draw,
        .interact_callback = sign_interact
    };

    blockRegistry[BLOCK_TORCH] = (BlockRegistry){
        .variant_generator = variant_torch,
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_15,
        .state_resolver = torch_state_resolver
    };

    blockRegistry[BLOCK_WATER_SOURCE] = (BlockRegistry){
        .variant_generator = variant_grass_block,
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_LIQUID,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .tick_callback = water_source_tick,
        .tick_speed = 3
    };

    blockRegistry[BLOCK_WATER_FLOWING] = (BlockRegistry){
        .variant_generator = variant_grass_block,
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_LIQUID,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .tick_callback = water_flowing_tick,
        .tick_speed = 3
    };

    blockRegistry[BLOCK_SLAB_FRAME] = (BlockRegistry){
        .variant_generator = variant_slab_frame,
        .selectable_state_count = 4,
        .selectable_states = {
            get_frame_block_state(0, 0),
            get_frame_block_state(1, 0),
            get_frame_block_state(2, 0),
            get_frame_block_state(3, 0)
        },
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NONE,
        .interact_callback = frame_block_interact
    };

    blockRegistry[BLOCK_STAIRS_FRAME] = (BlockRegistry){
        .variant_generator = variant_stairs_frame,
        .selectable_state_count = 4,
        .selectable_states = {
            get_frame_block_state(0, 0),
            get_frame_block_state(1, 0),
            get_frame_block_state(2, 0),
            get_frame_block_state(3, 0)
        },
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NONE,
        .interact_callback = frame_block_interact
    };

    blockRegistry[BLOCK_NUB_FRAME] = (BlockRegistry){
        .variant_generator = variant_nub_frame,
        .selectable_state_count = 4,
        .selectable_states = {
            get_frame_block_state(0, 0),
            get_frame_block_state(1, 0),
            get_frame_block_state(2, 0),
            get_frame_block_state(3, 0)
        },
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NONE,
        .interact_callback = frame_block_interact
    };
}

BlockRegistry* br_get_block_registry(size_t idx) {
    if (idx > BLOCK_COUNT - 1) return NULL;
    return &blockRegistry[idx];
}

void block_registry_free() {
	if (blockRegistry) free(blockRegistry);
}
