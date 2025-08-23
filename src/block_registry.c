#include "block_registry.h"

#include <stdlib.h>
#include <stdio.h>

#include "defines.h"

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
        .lightLevel = 0,
        .flag = BLOCK_FLAG_NONE,
        .transparent = true,
        .solid = false,
        .flipH = false,
        .flipV = false
    };

    blockRegistry[1] = (BlockRegistry){
        .name = "Grass Block",
        .atlas_idx = 0,
        .lightLevel = 0,
        .flag = BLOCK_FLAG_NONE,
        .transparent = false,
        .solid = true,
        .flipH = true,
        .flipV = false
    };

    blockRegistry[2] = (BlockRegistry){
        .name = "Dirt Block",
        .atlas_idx = 1,
        .lightLevel = 0,
        .flag = BLOCK_FLAG_NONE,
        .transparent = false,
        .solid = true,
        .flipH = true,
        .flipV = true
    };

    blockRegistry[3] = (BlockRegistry){
        .name = "Stone Block",
        .atlas_idx = 2,
        .lightLevel = 0,
        .flag = BLOCK_FLAG_NONE,
        .transparent = false,
        .solid = true,
        .flipH = true,
        .flipV = false
    };

    blockRegistry[4] = (BlockRegistry){
        .name = "Cobblestone",
        .atlas_idx = 3,
        .lightLevel = 0,
        .transparent = false,
        .solid = true,
        .flipH = false,
        .flipV = false
    };

    blockRegistry[5] = (BlockRegistry){
        .name = "Wooden Planks",
        .atlas_idx = 4,
        .lightLevel = 0,
        .flag = BLOCK_FLAG_NONE,
        .transparent = false,
        .solid = true,
        .flipH = false,
        .flipV = false
    };

    blockRegistry[6] = (BlockRegistry){
        .name = "Wood Log",
        .atlas_idx = 5,
        .lightLevel = 0,
        .flag = BLOCK_FLAG_LOG_LIKE,
        .transparent = false,
        .solid = true,
        .flipH = false,
        .flipV = false
    };

    blockRegistry[7] = (BlockRegistry){
        .name = "Leaves",
        .atlas_idx = 6,
        .lightLevel = 0,
        .flag = BLOCK_FLAG_NONE,
        .transparent = true,
        .solid = true,
        .flipH = true,
        .flipV = true
    };

    blockRegistry[8] = (BlockRegistry){
        .name = "Glass Block",
        .atlas_idx = 7,
        .lightLevel = 0,
        .flag = BLOCK_FLAG_NONE,
        .transparent = true,
        .solid = true,
        .flipH = false,
        .flipV = false
    };
}

BlockRegistry* br_get_block_registry(size_t idx) {
    if (idx > BLOCK_ATLAS_SIZE - 1) return NULL;
    return &blockRegistry[idx];
}

void block_registry_free() {
	if (blockRegistry) free(blockRegistry);
}
