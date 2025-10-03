#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include <stddef.h>

#include <raylib.h>

typedef enum {
	ATLAS_GRASS_BLOCK,
	ATLAS_DIRT_BLOCK,
	ATLAS_SAND,
	ATLAS_STONE,
	ATLAS_COBBLESTONE,
	ATLAS_WOODEN_PLANKS,
	ATLAS_WOOD_LOG,
	ATLAS_LEAVES,
	ATLAS_GLASS,
	ATLAS_LAMP,
	ATLAS_CHEST,
	ATLAS_WOOL,
	ATLAS_ORANGE_WOOL,
	ATLAS_MAGENTA_WOOL,
	ATLAS_LIGHT_BLUE_WOOL,
	ATLAS_YELLOW_WOOL,
	ATLAS_LIME_WOOL,
	ATLAS_PINK_WOOL,
	ATLAS_GRAY_WOOL,
	ATLAS_LIGHT_GRAY_WOOL,
	ATLAS_CYAN_WOOL,
	ATLAS_PURPLE_WOOL,
	ATLAS_BLUE_WOOL,
	ATLAS_BROWN_WOOL,
	ATLAS_GREEN_WOOL,
	ATLAS_RED_WOOL,
	ATLAS_BLACK_WOOL,
	ATLAS_GRASS,
	ATLAS_FLOWER,
	ATLAS_PEBBLES,
	ATLAS_FENCE,
	ATLAS_FENCE_RIGHT,
	ATLAS_FENCE_LEFT,
	ATLAS_FENCE_BOTH,
	ATLAS_FENCE_UP,
	ATLAS_FENCE_UP_RIGHT,
	ATLAS_FENCE_UP_LEFT,
	ATLAS_FENCE_UP_BOTH,
	ATLAS_LADDERS,
	ATLAS_TRAPDOOR,
	ATLAS_SIGN,
	ATLAS_SIGN_WALL,
	ATLAS_TORCH,
	ATLAS_WATER_BUCKET,
	ATLAS_COUNT
} TextureAtlasEnum;

void texture_atlas_load(const char* path, size_t r, size_t c);
Texture2D texture_atlas_get();
Rectangle texture_atlas_get_rect(size_t idx, bool flipH, bool flipV);
Rectangle texture_atlas_get_uv(size_t idx, bool flipH, bool flipV);
Material texture_atlas_get_material();
void texture_atlas_free();

#endif