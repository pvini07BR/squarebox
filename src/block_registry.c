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

Rectangle br_get_block_texture_rect(size_t idx, bool flipH, bool flipV)
{
    if (idx <= 0 || idx > BLOCK_COUNT - 1) return (Rectangle) { 0, 0, 0, 0 };

    size_t atlas_idx = blockRegistry[idx].atlas_idx;

    size_t col = atlas_idx % ATLAS_COLUMNS;
    size_t row = atlas_idx / ATLAS_COLUMNS;

    float x = (float)(col * TILE_SIZE);
    float y = (float)(row * TILE_SIZE);

    float width = (float)TILE_SIZE * (flipH ? -1.0f : 1.0f);
    float height = (float)TILE_SIZE * (flipV ? -1.0f : 1.0f);

    return (Rectangle) {
        .x = x,
        .y = y,
        .width = width,
        .height = height
    };
}

Rectangle br_get_block_uvs(size_t idx, bool flipH, bool flipV)
{
    if (idx <= 0 || idx > BLOCK_COUNT - 1) return (Rectangle) { 0, 0, 0, 0 };

    size_t atlas_idx = blockRegistry[idx].atlas_idx;

    size_t col = atlas_idx % ATLAS_COLUMNS;
    size_t row = atlas_idx / ATLAS_COLUMNS;

    float uv_unit_x = 1.0f / ATLAS_COLUMNS;
    float uv_unit_y = 1.0f / ATLAS_ROWS;

    float u0 = uv_unit_x * col;
    float u1 = u0 + uv_unit_x;
    float v0 = uv_unit_y * row;
    float v1 = v0 + uv_unit_y;

    if (flipH) {
        float tmp = u0; u0 = u1; u1 = tmp;
    }
    if (flipV) {
        float tmp = v0; v0 = v1; v1 = tmp;
    }

    return (Rectangle) {
        .x = u0,
            .y = v0,
            .width = u1 - u0,
            .height = v1 - v0
    };
}

void block_registry_free() {
	if (blockRegistry) free(blockRegistry);
}
