#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <raylib.h>

void texture_atlas_load(const char* path);
Texture2D texture_atlas_get();
Rectangle texture_atlas_get_rect(size_t idx, bool flipH, bool flipV);
Rectangle texture_atlas_get_uv(size_t idx, bool flipH, bool flipV);
void texture_atlas_free();

#endif