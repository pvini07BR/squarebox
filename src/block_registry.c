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
        .lightLevel = 0,
        .transparent = true,
        .solid = false,
        .flipH = false,
        .flipV = false
    };

    registry[1] = (BlockRegistry){
        .name = "Grass Block",
        .lightLevel = 0,
        .transparent = false,
        .solid = true,
        .flipH = true,
        .flipV = false
    };

    registry[2] = (BlockRegistry){
        .name = "Dirt Block",
        .lightLevel = 0,
        .transparent = false,
        .solid = true,
        .flipH = true,
        .flipV = true
    };

    registry[3] = (BlockRegistry){
        .name = "Stone Block",
        .lightLevel = 0,
        .transparent = false,
        .solid = true,
        .flipH = true,
        .flipV = false
    };

    registry[4] = (BlockRegistry){
        .name = "Cobblestone",
        .lightLevel = 0,
        .transparent = false,
        .solid = true,
        .flipH = false,
        .flipV = false
    };

    registry[5] = (BlockRegistry){
        .name = "Wooden Planks",
        .lightLevel = 0,
        .transparent = false,
        .solid = true,
        .flipH = false,
        .flipV = false
    };

    registry[6] = (BlockRegistry){
        .name = "Wood Log",
        .lightLevel = 0,
        .transparent = false,
        .solid = true,
        .flipH = false,
        .flipV = false
    };

    registry[7] = (BlockRegistry){
        .name = "Leaves",
        .lightLevel = 0,
        .transparent = true,
        .solid = true,
        .flipH = true,
        .flipV = true
    };

    registry[8] = (BlockRegistry){
        .name = "Glass Block",
        .lightLevel = 0,
        .transparent = true,
        .solid = true,
        .flipH = false,
        .flipV = false
    };
}

BlockRegistry* br_get_block_registry(uint8_t idx) {
    if (idx > BLOCK_COUNT - 1) return NULL;
    return &registry[idx];
}

Texture2D* br_get_block_atlas()
{
    return &blockAtlas;
}

Rectangle br_get_block_texture_rect(uint8_t idx, bool flipH, bool flipV)
{
    if (idx > BLOCK_COUNT - 1) return (Rectangle) { 0, 0, 0, 0 };
    if (idx == 0) return (Rectangle) { 0, 0, 0, 0 };
    return (Rectangle){
        .x = (float)(idx - 1) * (float)TILE_SIZE,
        .y = 0.0f,
        .width = (float)TILE_SIZE * (flipH ? -1.0f : 1.0f),
        .height = (float)TILE_SIZE * (flipV ? -1.0f : 1.0f)
    };
}

Rectangle br_get_block_uvs(uint8_t idx, bool flipH, bool flipV)
{
    if (idx == 0 || idx > BLOCK_COUNT - 1) return (Rectangle) { 0, 0, 0, 0 };

    float uv_unit = 1.0f / (BLOCK_COUNT - 1);

    float u0 = uv_unit * (idx - 1);
    float u1 = u0 + uv_unit;
    float v0 = 0.0f;
    float v1 = 1.0f;

    if (flipH) {
        float tmp = u0;
        u0 = u1;
        u1 = tmp;
    }
    if (flipV) {
        float tmp = v0;
        v0 = v1;
        v1 = tmp;
    }

    return (Rectangle) {
        .x = u0,
        .y = v0,
        .width = u1 - u0,
        .height = v1 - v0
    };
}

void block_registry_free() {
    UnloadTexture(blockAtlas);
	if (registry) free(registry);
}
