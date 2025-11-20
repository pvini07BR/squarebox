#include "registries/block_registry.h"
#include "block_functions.h"
#include "block_states.h"
#include "block_variants.h"

#include <stdlib.h>
#include <stdio.h>

static BlockRegistry* reg = NULL;

void block_registry_init() {
    reg = calloc(BLOCK_COUNT, sizeof(BlockRegistry));
    if (reg == NULL) {
        TraceLog(LOG_ERROR, "Could not allocate memory for the block registry.");
        return;
    };

    reg[BLOCK_AIR] = (BlockRegistry){
        .variant_generator = variant_grass_block,
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    reg[BLOCK_GRASS_BLOCK] = (BlockRegistry){
        .variant_generator = variant_grass_block,        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_PLANTABLE,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    reg[BLOCK_DIRT] = (BlockRegistry){
        .variant_generator = variant_dirt,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_PLANTABLE,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    reg[BLOCK_SAND] = (BlockRegistry){
        .variant_generator = variant_sand,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_GRAVITY_AFFECTED,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL,
        .tick_callback = falling_block_tick,
        .tick_speed = 1
    };

    reg[BLOCK_STONE] = (BlockRegistry){
        .variant_generator = variant_stone,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    reg[BLOCK_COBBLESTONE] = (BlockRegistry){
        .variant_generator = variant_cobblestone,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_WOODEN_PLANKS] = (BlockRegistry){
        .variant_generator = variant_wooden_planks,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    reg[BLOCK_WOOD_LOG] = (BlockRegistry){
        .variant_generator = variant_wood_log,
        .selectable_state_count = 3,
        .selectable_states = {
            LOGLIKE_BLOCK_STATE_VERTICAL,
            LOGLIKE_BLOCK_STATE_HORIZONTAL,
            LOGLIKE_BLOCK_STATE_FORWARD
        },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = NULL
    };

    reg[BLOCK_LEAVES] = (BlockRegistry){
        .variant_generator = variant_leaves,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    reg[BLOCK_GLASS] = (BlockRegistry){
        .variant_generator = variant_glass,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    reg[BLOCK_LAMP] = (BlockRegistry){
        .variant_generator = variant_lamp,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_15,
        .state_resolver = NULL
    };

    reg[BLOCK_CHEST] = (BlockRegistry){
        .variant_generator = variant_chest,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE,
        .interact_callback = on_chest_interact,
        .state_resolver = chest_solver,
        .free_data = chest_free_data
    };

    reg[BLOCK_BOUNCY_BLOCK] = (BlockRegistry){
        .variant_generator = variant_bouncy_block,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_BOUNCY,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_ICE] = (BlockRegistry){
        .variant_generator = variant_ice,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_SLIPPERY,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT
    };

    reg[BLOCK_WOOL] = (BlockRegistry){
        .variant_generator = variant_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_ORANGE_WOOL] = (BlockRegistry){
        .variant_generator = variant_orange_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_MAGENTA_WOOL] = (BlockRegistry){
        .variant_generator = variant_magenta_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_LIGHT_BLUE_WOOL] = (BlockRegistry){
        .variant_generator = variant_light_blue_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_YELLOW_WOOL] = (BlockRegistry){
        .variant_generator = variant_yellow_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_LIME_WOOL] = (BlockRegistry){
        .variant_generator = variant_lime_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_PINK_WOOL] = (BlockRegistry){
        .variant_generator = variant_pink_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_GRAY_WOOL] = (BlockRegistry){
        .variant_generator = variant_gray_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_LIGHT_GRAY_WOOL] = (BlockRegistry){
        .variant_generator = variant_light_gray_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_CYAN_WOOL] = (BlockRegistry){
        .variant_generator = variant_cyan_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_PURPLE_WOOL] = (BlockRegistry){
        .variant_generator = variant_purple_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_BLUE_WOOL] = (BlockRegistry){
        .variant_generator = variant_blue_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_BROWN_WOOL] = (BlockRegistry){
        .variant_generator = variant_brown_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_GREEN_WOOL] = (BlockRegistry){
        .variant_generator = variant_green_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_RED_WOOL] = (BlockRegistry){
        .variant_generator = variant_red_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_BLACK_WOOL] = (BlockRegistry){
        .variant_generator = variant_black_wool,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_GRASS] = (BlockRegistry){
        .variant_generator = variant_grass,
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_FLIP_H,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
		.state_resolver = plant_block_resolver
    };

    reg[BLOCK_FLOWER] = (BlockRegistry){
        .variant_generator = variant_flower,
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = plant_block_resolver
    };

    reg[BLOCK_PEBBLES] = (BlockRegistry){
        .variant_generator = variant_pebbles,
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = grounded_block_resolver
    };

    reg[BLOCK_WOODEN_FENCE] = (BlockRegistry){
        .variant_generator = variant_wooden_fence,
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = fence_resolver
    };

    reg[BLOCK_LADDERS] = (BlockRegistry){
        .variant_generator = variant_ladders,
        .flags = BLOCK_FLAG_CLIMBABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
    };

    reg[BLOCK_TRAPDOOR] = (BlockRegistry){
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

    reg[BLOCK_SIGN] = (BlockRegistry){
        .variant_generator = variant_sign,
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = sign_solver,
        .free_data = sign_free_data,
        .overlay_draw = sign_text_draw,
        .interact_callback = sign_interact
    };

    reg[BLOCK_TORCH] = (BlockRegistry){
        .variant_generator = variant_torch,
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_15,
        .state_resolver = torch_state_resolver
    };

    reg[BLOCK_WATER_SOURCE] = (BlockRegistry){
        .variant_generator = variant_grass_block,
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_LIQUID,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .tick_callback = water_source_tick,
        .tick_speed = 3
    };

    reg[BLOCK_WATER_FLOWING] = (BlockRegistry){
        .variant_generator = variant_grass_block,
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_LIQUID,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .tick_callback = water_flowing_tick,
        .tick_speed = 3
    };

    reg[BLOCK_SLAB_FRAME] = (BlockRegistry){
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

    reg[BLOCK_STAIRS_FRAME] = (BlockRegistry){
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

    reg[BLOCK_NUB_FRAME] = (BlockRegistry){
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

    reg[BLOCK_POWER_WIRE] = (BlockRegistry){
        .flags = BLOCK_FLAG_POWER_TRIGGERED,
        .variant_generator = variant_power_wire,
        .state_resolver = power_wire_solver,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_BATTERY] = (BlockRegistry) {
        .variant_generator = variant_battery,
        .selectable_state_count = 3,
        .selectable_states = {
            LOGLIKE_BLOCK_STATE_VERTICAL,
            LOGLIKE_BLOCK_STATE_HORIZONTAL,
            LOGLIKE_BLOCK_STATE_FORWARD
        },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .state_resolver = NULL,
        .lightLevel = BLOCK_LIGHT_NONE
    };

    reg[BLOCK_POWER_REPEATER] = (BlockRegistry) {
        .variant_generator = variant_power_repeater,
        .state_resolver = power_repeater_solver,
        .flags = BLOCK_FLAG_POWER_TRIGGERED,
        .selectable_state_count = 4,
        .selectable_states = {
            get_power_repeater_state(0, false),
            get_power_repeater_state(1, false),
            get_power_repeater_state(2, false),
            get_power_repeater_state(3, false),
        }
    };

    reg[BLOCK_POWERED_LAMP] = (BlockRegistry){
        .variant_generator = variant_powered_lamp,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_POWER_TRIGGERED,
        .lightLevel = BLOCK_LIGHT_NONE,
        .state_resolver = powered_lamp_solver
    };
}

BlockRegistry* br_get_block_registry(size_t idx) {
    if (idx > BLOCK_COUNT - 1) return NULL;
    return &reg[idx];
}

void block_registry_free() {
	if (reg) free(reg);
}
