#include "registries/block_registry.h"
#include "registries/block_models.h"
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
        .variant_count = 1,
        .variants = { { .atlas_idx = ATLAS_GRASS_BLOCK, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_GRASS_BLOCK] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = ATLAS_GRASS_BLOCK, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_PLANTABLE,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_DIRT] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = ATLAS_DIRT_BLOCK, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_PLANTABLE,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_SAND] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_SAND, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_GRAVITY_AFFECTED,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL,
        .tick_callback = falling_block_tick,
        .tick_speed = 1
    };

    blockRegistry[BLOCK_STONE] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_COBBLESTONE] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOODEN_PLANKS] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOOD_LOG] = (BlockRegistry){
        .variant_count = 2,
        .variants = { 
            { .atlas_idx = ATLAS_WOOD_LOG, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_WOOD_LOG, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 1 },
        },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_LEAVES] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = ATLAS_LEAVES, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_GLASS] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = ATLAS_GLASS, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_LAMP] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = ATLAS_LAMP, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_15,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_CHEST] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = ATLAS_CHEST, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .interact_callback = on_chest_interact,
        .state_resolver = chest_solver,
        .destroy_callback = on_chest_destroy
    };

    blockRegistry[BLOCK_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_ORANGE_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_ORANGE_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_MAGENTA_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_MAGENTA_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_LIGHT_BLUE_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_LIGHT_BLUE_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_YELLOW_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_YELLOW_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_LIME_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_LIME_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_PINK_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_PINK_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_GRAY_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_GRAY_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_LIGHT_GRAY_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_LIGHT_GRAY_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_CYAN_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_CYAN_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_PURPLE_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_PURPLE_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_BLUE_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_BLUE_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_BROWN_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_BROWN_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_GREEN_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_GREEN_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_RED_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_RED_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_BLACK_WOOL] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_BLACK_WOOL, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT
    };

    blockRegistry[BLOCK_GRASS] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_GRASS, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_FLIP_H,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
		.state_resolver = plant_block_resolver
    };

    blockRegistry[BLOCK_FLOWER] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_FLOWER, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = plant_block_resolver
    };

    blockRegistry[BLOCK_PEBBLES] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_PEBBLES, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = grounded_block_resolver
    };

    blockRegistry[BLOCK_WOODEN_FENCE] = (BlockRegistry){
        .variant_count = 8,
        .variants = {
            {.atlas_idx = ATLAS_FENCE, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_FENCE_RIGHT, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_FENCE_LEFT, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_FENCE_BOTH, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_FENCE_UP, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_FENCE_UP_RIGHT, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_FENCE_UP_LEFT, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_FENCE_UP_BOTH, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
        },
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = fence_resolver
    };

    blockRegistry[BLOCK_LADDERS] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = ATLAS_LADDERS, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_CLIMBABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
    };

    blockRegistry[BLOCK_TRAPDOOR] = (BlockRegistry){
        .variant_count = 4,
        .variants = {
            {.atlas_idx = ATLAS_TRAPDOOR, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_TRAPDOOR, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_TRAPDOOR, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_TRAPDOOR, .flipH = false, .flipV = false, .rotation = 1 },
            {.atlas_idx = ATLAS_TRAPDOOR, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_TRAPDOOR, .flipH = false, .flipV = false, .rotation = 2 },
            {.atlas_idx = ATLAS_TRAPDOOR, .model_idx = BLOCK_MODEL_QUAD, .collider_idx = BLOCK_COLLIDER_TRAPDOOR, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_STATE_SELECTABLE,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_count = 4,
        .state_selector = trapdoor_state_selector,
        .variant_selector = trapdoor_variant_selector,
        .interact_callback = trapdoor_interact
    };

    blockRegistry[BLOCK_SIGN] = (BlockRegistry){
        .variant_count = 2,
        .variants = {
            {.atlas_idx = ATLAS_SIGN, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_SIGN_WALL, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
        },
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = sign_solver,
        .destroy_callback = on_sign_destroy,
        .overlay_draw = sign_text_draw,
        .interact_callback = sign_interact
    };

    blockRegistry[BLOCK_TORCH] = (BlockRegistry){
        .variant_count = 3,
        .variants = {
            {.atlas_idx = ATLAS_TORCH, .model_idx = BLOCK_MODEL_TORCH, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_TORCH, .model_idx = BLOCK_MODEL_TORCH_WALL, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = ATLAS_TORCH, .model_idx = BLOCK_MODEL_TORCH_WALL, .flipH = true, .flipV = false, .rotation = 0 },
        },
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_15,
        .state_resolver = torch_state_resolver
    };

    blockRegistry[BLOCK_WATER_SOURCE] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = 0, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_LIQUID,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .tick_callback = water_source_tick,
        .tick_speed = 3
    };

    blockRegistry[BLOCK_WATER_FLOWING] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = 0, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_LIQUID,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .tick_callback = water_flowing_tick,
        .tick_speed = 3
    };

    blockRegistry[BLOCK_DIRT_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = ATLAS_DIRT_BLOCK, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_DIRT_BLOCK, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = ATLAS_DIRT_BLOCK, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = ATLAS_DIRT_BLOCK, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_STONE_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_COBBLESTONE_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_SLAB, .collider_idx = BLOCK_COLLIDER_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_STONE_STAIRS] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_COBBLESTONE_STAIRS] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_STAIRS] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_STAIRS, .collider_idx = BLOCK_COLLIDER_STAIRS, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_STONE_NUB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = ATLAS_STONE, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_COBBLESTONE_NUB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = ATLAS_COBBLESTONE, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_NUB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = ATLAS_WOODEN_PLANKS, .model_idx = BLOCK_MODEL_NUB, .collider_idx = BLOCK_COLLIDER_NUB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_SELECTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };
}

BlockRegistry* br_get_block_registry(size_t idx) {
    if (idx > BLOCK_COUNT - 1) return NULL;
    return &blockRegistry[idx];
}

BlockVariant br_get_block_variant(size_t reg_idx, size_t variant_idx) {
    BlockRegistry* br = br_get_block_registry(reg_idx);
    if (br == NULL) return (BlockVariant){ 0, 0, 0, 0, 0, 0 };
    if (variant_idx >= br->variant_count) return br->variants[0];
    return br->variants[variant_idx];
}

void block_registry_free() {
	if (blockRegistry) free(blockRegistry);
}
