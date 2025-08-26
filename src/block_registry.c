#include "block_registry.h"
#include "block_models.h"
#include "defines.h"

#include <stdlib.h>
#include <stdio.h>

static BlockRegistry* blockRegistry = NULL;

void block_registry_init() {
    blockRegistry = calloc(BLOCK_COUNT, sizeof(BlockRegistry));
    if (blockRegistry == NULL) {
        fprintf(stderr, "[ERROR] Could not allocate memory for the block registry.\n");
        return;
    };

    blockRegistry[BLOCK_AIR] = (BlockRegistry){
        .atlas_idx = 0,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_TRANSPARENT,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_GRASS_BLOCK] = (BlockRegistry){
        .atlas_idx = 0,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_DIRT] = (BlockRegistry){
        .atlas_idx = 1,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_STONE] = (BlockRegistry){
        .atlas_idx = 2,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_COBBLESTONE] = (BlockRegistry){
        .atlas_idx = 3,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_WOODEN_PLANKS] = (BlockRegistry){
        .atlas_idx = 4,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_WOOD_LOG] = (BlockRegistry){
        .atlas_idx = 5,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_LEAVES] = (BlockRegistry){
        .atlas_idx = 6,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_TRANSPARENT | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_GLASS] = (BlockRegistry){
        .atlas_idx = 7,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_TRANSPARENT,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_LAMP] = (BlockRegistry){
        .atlas_idx = 8,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 15,
    };

    blockRegistry[BLOCK_CHEST] = (BlockRegistry){
        .atlas_idx = 9,
        .model_idx = BLOCK_MODEL_QUAD,
        .trait = BLOCK_TRAIT_CONTAINER,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_DIRT_SLAB] = (BlockRegistry){
        .atlas_idx = 1,
        .model_idx = BLOCK_MODEL_SLAB,
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_STONE_SLAB] = (BlockRegistry){
        .atlas_idx = 2,
        .model_idx = BLOCK_MODEL_SLAB,
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_COBBLESTONE_SLAB] = (BlockRegistry){
        .atlas_idx = 3,
        .model_idx = BLOCK_MODEL_SLAB,
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_SLAB] = (BlockRegistry){
        .atlas_idx = 4,
        .model_idx = BLOCK_MODEL_SLAB,
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_STONE_STAIRS] = (BlockRegistry){
        .atlas_idx = 2,
        .model_idx = BLOCK_MODEL_STAIRS,
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_COBBLESTONE_STAIRS] = (BlockRegistry){
        .atlas_idx = 3,
        .model_idx = BLOCK_MODEL_STAIRS,
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[BLOCK_WOODEN_PLANKS_STAIRS] = (BlockRegistry){
        .atlas_idx = 4,
        .model_idx = BLOCK_MODEL_STAIRS,
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };
}

BlockRegistry* br_get_block_registry(size_t idx) {
    if (idx > BLOCK_COUNT - 1) return NULL;
    return &blockRegistry[idx];
}

void block_registry_free() {
	if (blockRegistry) free(blockRegistry);
}
