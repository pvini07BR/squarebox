#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#define BLOCK_COUNT 9

#include <stdbool.h>
#include <stdint.h>

#include <raylib.h>

typedef struct {
	const char* name;
	unsigned int atlasX;
	unsigned int atlasY;
	uint8_t lightLevel;
	bool transparent;
	bool solid;
	bool flipH;
	bool flipV;
} BlockRegistry;

void block_registry_init();
BlockRegistry* block_registry_get_block_registry(uint8_t idx);
Texture2D* block_registry_get_block_atlas();
Rectangle block_registry_get_block_texture_rect(uint8_t idx, bool flipH, bool flipV);
void block_registry_free();

#endif