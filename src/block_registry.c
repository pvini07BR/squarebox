#include "block_registry.h"
#include "block_models.h"
#include "liquid_spread.h"
#include "chunk.h"
#include "defines.h"

#include <stdlib.h>
#include <stdio.h>

bool grounded_block_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
    BlockRegistry* bottom_brg = br_get_block_registry(neighbors[NEIGHBOR_BOTTOM].id);
    if (bottom_brg) {
        return bottom_brg->flags & BLOCK_FLAG_SOLID && bottom_brg->flags & BLOCK_FLAG_FULL_BLOCK;
    } else {
		return false;
	}
}

bool plant_block_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
    BlockRegistry* bottom_brg = br_get_block_registry(neighbors[NEIGHBOR_BOTTOM].id);
    if (bottom_brg) {
        return bottom_brg->flags & BLOCK_FLAG_SOLID && bottom_brg->flags & BLOCK_FLAG_FULL_BLOCK && bottom_brg->flags & BLOCK_FLAG_PLANTABLE;
    }
    else {
        return false;
    }
}

bool torch_state_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
    BlockRegistry* bottom_brg = br_get_block_registry(neighbors[NEIGHBOR_BOTTOM].id);
    if (bottom_brg->flags & BLOCK_FLAG_SOLID && bottom_brg->flags & BLOCK_FLAG_FULL_BLOCK) {
        inst->state = 0;
        return true;
    }
    
    BlockRegistry* right_brg = br_get_block_registry(neighbors[NEIGHBOR_RIGHT].id);
    if (right_brg->flags & BLOCK_FLAG_SOLID && right_brg->flags & BLOCK_FLAG_FULL_BLOCK) {
        inst->state = 1;
        return true;
    }

    BlockRegistry* left_brg = br_get_block_registry(neighbors[NEIGHBOR_LEFT].id);
    if (left_brg->flags & BLOCK_FLAG_SOLID && left_brg->flags & BLOCK_FLAG_FULL_BLOCK) {
        inst->state = 2;
        return true;
    }

    return false;
}

bool fence_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
    bool right = neighbors[NEIGHBOR_RIGHT].id == inst->id;
	bool left = neighbors[NEIGHBOR_LEFT].id == inst->id;

    if (right && left) inst->state = 3;
    else if (right) inst->state = 1;
    else if (left) inst->state = 2;
	else inst->state = 0;

    return true;
}

bool on_chest_interact(BlockInstance* inst, Chunk* chunk) {
    if (inst->state >= 0) {
        item_container_open(container_vector_get(&chunk->containerVec, inst->state));
        return true;
    }
    return false;
}

bool chest_solver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
    inst->state = container_vector_add(&chunk->containerVec, "Chest", 3, 10, false);
    if (inst->state >= 0) return true;
    return false;
}

void on_chest_destroy(BlockInstance* inst, Chunk* chunk, uint8_t idx) {
    if (inst->state >= 0) {
        container_vector_remove(&chunk->containerVec, inst->state);
        inst->state = -1;
    }
}

bool liquid_solver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
	liquid_spread_list_add(&chunk->liquidSpreadList, idx);
    return true;
}

void on_liquid_destroy(BlockInstance* inst, Chunk* chunk, uint8_t idx) {
    liquid_spread_list_remove(&chunk->liquidSpreadList, idx);
}

static BlockRegistry* blockRegistry = NULL;

void block_registry_init() {
    blockRegistry = calloc(BLOCK_COUNT, sizeof(BlockRegistry));
    if (blockRegistry == NULL) {
        fprintf(stderr, "[ERROR] Could not allocate memory for the block registry.\n");
        return;
    };

    blockRegistry[BLOCK_AIR] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 0, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_GRASS_BLOCK] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 0, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_PLANTABLE,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_DIRT] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 1, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_PLANTABLE,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_SAND] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = 2, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK | BLOCK_FLAG_GRAVITY_AFFECTED,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_STONE] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 3, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_COBBLESTONE] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 4, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOODEN_PLANKS] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 5, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOOD_LOG] = (BlockRegistry){
        .variant_count = 2,
        .variants = { 
            { .atlas_idx = 6, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 6, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 1 },
        },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_LEAVES] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 7, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_GLASS] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 8, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_LAMP] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 9, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_15,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_CHEST] = (BlockRegistry){
        .variant_count = 1,
        .variants = { { .atlas_idx = 10, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FULL_BLOCK,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .interact_callback = on_chest_interact,
        .state_resolver = chest_solver,
        .destroy_callback = on_chest_destroy
    };

    blockRegistry[BLOCK_GRASS] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = 11, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_FLIP_H,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
		.state_resolver = plant_block_resolver
    };

    blockRegistry[BLOCK_FLOWER] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = 12, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = plant_block_resolver
    };

    blockRegistry[BLOCK_PEBBLES] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = 13, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = grounded_block_resolver
    };

    blockRegistry[BLOCK_WOODEN_FENCE] = (BlockRegistry){
        .variant_count = 4,
        .variants = {
            {.atlas_idx = 14, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = 15, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = 16, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = 17, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 },
        },
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = fence_resolver
    };

    blockRegistry[BLOCK_TORCH] = (BlockRegistry){
        .variant_count = 3,
        .variants = {
            {.atlas_idx = 18, .model_idx = BLOCK_MODEL_TORCH, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = 18, .model_idx = BLOCK_MODEL_TORCH_WALL, .flipH = false, .flipV = false, .rotation = 0 },
            {.atlas_idx = 18, .model_idx = BLOCK_MODEL_TORCH_WALL, .flipH = true, .flipV = false, .rotation = 0 },
        },
        .flags = 0,
        .lightLevel = BLOCK_LIGHT_15,
        .state_resolver = torch_state_resolver
    };

    blockRegistry[BLOCK_WATER_SOURCE] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = 0, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_LIQUID_SOURCE,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = liquid_solver,
        .destroy_callback = on_liquid_destroy
    };

    blockRegistry[BLOCK_WATER_FLOWING] = (BlockRegistry){
        .variant_count = 1,
        .variants = { {.atlas_idx = 0, .model_idx = BLOCK_MODEL_QUAD, .flipH = false, .flipV = false, .rotation = 0 } },
        .flags = BLOCK_FLAG_REPLACEABLE | BLOCK_FLAG_LIQUID_FLOWING,
        .lightLevel = BLOCK_LIGHT_TRANSPARENT,
        .state_resolver = liquid_solver,
        .destroy_callback = on_liquid_destroy
    };

    blockRegistry[BLOCK_DIRT_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 1, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 1, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 1, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 1, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_STONE_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_COBBLESTONE_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_SLAB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_SLAB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_STONE_STAIRS] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_COBBLESTONE_STAIRS] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_STAIRS] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_STAIRS, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_STONE_NUB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 3, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_COBBLESTONE_NUB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 4, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_SOLID,
        .lightLevel = BLOCK_LIGHT_NO_LIGHT,
        .state_resolver = NULL
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_NUB] = (BlockRegistry){
        .variant_count = 4,
        .variants = { 
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 0 },
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 1 },
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 2 },
            { .atlas_idx = 5, .model_idx = BLOCK_MODEL_NUB, .flipH = false, .flipV = false, .rotation = 3 },
        },
        .flags = BLOCK_FLAG_STATE_MUTABLE | BLOCK_FLAG_SOLID,
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
    if (br == NULL) return (BlockVariant){ 0, 0, 0, 0, 0 };
    if (variant_idx >= br->variant_count) return br->variants[0];
    return br->variants[variant_idx];
}

void block_registry_free() {
	if (blockRegistry) free(blockRegistry);
}
