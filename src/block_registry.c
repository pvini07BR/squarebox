#include "block_registry.h"

#include <stdlib.h>
#include <chunk.h>

static Texture2D blockAtlas;
static BlockRegistry* registry = NULL;

void block_registry_init() {
    blockAtlas = LoadTexture(ASSETS_PATH "blocks.png");
    SetTextureWrap(blockAtlas, TEXTURE_WRAP_CLAMP);

	registry = calloc(BLOCK_COUNT, sizeof(BlockRegistry));

    registry[0] = (BlockRegistry){
        .name = "Air",
        .atlasX = 0,
        .atlasY = 0,
        .transparent = true,
        .solid = false,
        .flipH = false,
        .flipV = false
    };

    registry[1] = (BlockRegistry){
        .name = "Grass Block",
        .atlasX = 0,
        .atlasY = 0,
        .transparent = false,
        .solid = true,
        .flipH = true,
        .flipV = false
    };

    registry[2] = (BlockRegistry){
        .name = "Dirt Block",
        .atlasX = 32,
        .atlasY = 0,
        .transparent = false,
        .solid = true,
        .flipH = true,
        .flipV = true
    };

    registry[3] = (BlockRegistry){
        .name = "Stone Block",
        .atlasX = 64,
        .atlasY = 0,
        .transparent = false,
        .solid = true,
        .flipH = true,
        .flipV = false
    };

    registry[4] = (BlockRegistry){
        .name = "Cobblestone",
        .atlasX = 96,
        .atlasY = 0,
        .transparent = false,
        .solid = true,
        .flipH = false,
        .flipV = false
    };

    registry[5] = (BlockRegistry){
        .name = "Wooden Planks",
        .atlasX = 128,
        .atlasY = 0,
        .transparent = false,
        .solid = true,
        .flipH = false,
        .flipV = false
    };

    registry[6] = (BlockRegistry){
        .name = "Wood Log",
        .atlasX = 160,
        .atlasY = 0,
        .transparent = false,
        .solid = true,
        .flipH = false,
        .flipV = false
    };

    registry[7] = (BlockRegistry){
        .name = "Leaves",
        .atlasX = 192,
        .atlasY = 0,
        .transparent = true,
        .solid = true,
        .flipH = true,
        .flipV = true
    };

    registry[8] = (BlockRegistry){
        .name = "Glass Block",
        .atlasX = 224,
        .atlasY = 0,
        .transparent = true,
        .solid = true,
        .flipH = false,
        .flipV = false
    };
}

BlockRegistry* block_registry_get_block_registry(uint8_t idx) {
    if (idx > BLOCK_COUNT - 1) return NULL;
    return &registry[idx];
}

Texture2D* block_registry_get_block_atlas()
{
    return &blockAtlas;
}

Rectangle block_registry_get_block_texture_rect(uint8_t idx, bool flipH, bool flipV)
{
    if (idx > BLOCK_COUNT - 1) return (Rectangle) { 0, 0, 0, 0 };
    if (idx == 0) (Rectangle) { 0, 0, 0, 0 };
    return (Rectangle){
        .x = (float)registry[idx].atlasX,
        .y = 0.0f,
        .width = (float)TILE_SIZE * (flipH ? -1.0f : 1.0f),
        .height = (float)TILE_SIZE * (flipV ? -1.0f : 1.0f)
    };
}

void block_registry_free() {
    UnloadTexture(blockAtlas);
	if (registry) free(registry);
}
