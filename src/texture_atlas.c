#include "texture_atlas.h"

static Texture2D textureAtlas;
static Material material;
size_t rows;
size_t columns;

void texture_atlas_load(const char* path, size_t r, size_t c) {
    rows = r;
    columns = c;

	textureAtlas = LoadTexture(path);
	SetTextureWrap(textureAtlas, TEXTURE_WRAP_CLAMP);

    material = LoadMaterialDefault();
    SetMaterialTexture(&material, MATERIAL_MAP_ALBEDO, textureAtlas);
}

Texture2D texture_atlas_get() { return textureAtlas; }

Rectangle texture_atlas_get_rect(size_t idx, bool flipH, bool flipV)
{
    size_t col = idx % columns;
    size_t row = idx / columns;

    size_t tileWidth = (float)textureAtlas.width / columns;
    size_t tileHeight = (float)textureAtlas.height / rows;

    float x = (float)(col * tileWidth);
    float y = (float)(row * tileHeight);

    float width = (float)tileWidth * (flipH ? -1.0f : 1.0f);
    float height = (float)tileHeight * (flipV ? -1.0f : 1.0f);

    return (Rectangle) {
        .x = x,
        .y = y,
        .width = width,
        .height = height
    };
}

Rectangle texture_atlas_get_uv(size_t idx, bool flipH, bool flipV)
{
    size_t col = idx % columns;
    size_t row = idx / columns;

    float uv_unit_x = 1.0f / columns;
    float uv_unit_y = 1.0f / rows;

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

Material texture_atlas_get_material()
{
    return material;
}

void texture_atlas_free()
{
    UnloadMaterial(material);
	UnloadTexture(textureAtlas);
}
