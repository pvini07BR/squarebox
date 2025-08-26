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

    blockRegistry[0] = (BlockRegistry){
        .name = "Air",
        .atlas_idx = 0,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_TRANSPARENT,
        .lightLevel = 0,
    };

    blockRegistry[1] = (BlockRegistry){
        .name = "Grass Block",
        .atlas_idx = 0,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = 0,
    };

    blockRegistry[2] = (BlockRegistry){
        .name = "Dirt Block",
        .atlas_idx = 1,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V,
        .lightLevel = 0,
    };

    blockRegistry[3] = (BlockRegistry){
        .name = "Stone Block",
        .atlas_idx = 2,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_FLIP_H,
        .lightLevel = 0,
    };

    blockRegistry[4] = (BlockRegistry){
        .name = "Cobblestone",
        .atlas_idx = 3,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[5] = (BlockRegistry){
        .name = "Wooden Planks",
        .atlas_idx = 4,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[6] = (BlockRegistry){
        .name = "Wood Log",
        .atlas_idx = 5,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_ROTATES,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 0,
    };

    blockRegistry[7] = (BlockRegistry){
        .name = "Leaves",
        .atlas_idx = 6,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_TRANSPARENT | BLOCK_FLAG_FLIP_H | BLOCK_FLAG_FLIP_V,
        .lightLevel = 0,
    };

    blockRegistry[8] = (BlockRegistry){
        .name = "Glass Block",
        .atlas_idx = 7,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID | BLOCK_FLAG_TRANSPARENT,
        .lightLevel = 0,
    };

    blockRegistry[9] = (BlockRegistry){
        .name = "Lamp Block",
        .atlas_idx = 8,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_NONE,
        .flags = BLOCK_FLAG_SOLID,
        .lightLevel = 15,
    };

    blockRegistry[10] = (BlockRegistry){
        .name = "Chest",
        .atlas_idx = 9,
        .model_idx = 0,
        .trait = BLOCK_TRAIT_CONTAINER,
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
