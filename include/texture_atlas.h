#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <stddef.h>

#include <raylib.h>

void texture_atlas_load(const char* path, size_t r, size_t c);
Texture2D texture_atlas_get();
Rectangle texture_atlas_get_rect(size_t idx, bool flipH, bool flipV);
Rectangle texture_atlas_get_uv(size_t idx, bool flipH, bool flipV);
Material texture_atlas_get_material();
void texture_atlas_free();

#endif