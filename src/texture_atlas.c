#include "texture_atlas.h"

#include "defines.h"

static Texture2D textureAtlas;

void texture_atlas_load(const char* path) {
	textureAtlas = LoadTexture(path);
	SetTextureWrap(textureAtlas, TEXTURE_WRAP_CLAMP);
}

Texture2D texture_atlas_get() { return textureAtlas; }

Rectangle texture_atlas_get_rect(size_t idx, bool flipH, bool flipV)
{
    size_t col = idx % ATLAS_COLUMNS;
    size_t row = idx / ATLAS_COLUMNS;

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

Rectangle texture_atlas_get_uv(size_t idx, bool flipH, bool flipV)
{
    size_t col = idx % ATLAS_COLUMNS;
    size_t row = idx / ATLAS_COLUMNS;

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

void texture_atlas_free()
{
	UnloadTexture(textureAtlas);
}
