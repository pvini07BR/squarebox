#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <stddef.h>
#include <stdint.h>

#include <raylib.h>

#include "types.h"

typedef struct {
	const char* fileName;
	uint8_t variantCount;
	uint8_t offset;
} AtlasEntry;

void texture_atlas_init();

Texture2D texture_atlas_get();
Rectangle texture_atlas_get_rect(TextureAtlasEnum atlas, uint8_t variantIdx);
Vector2 texture_atlas_get_uv(TextureAtlasEnum atlas, uint8_t variantIdx, Vector2 relativePoint, bool flipH, bool flipV);
Material texture_atlas_get_material();
void texture_atlas_free();

#endif