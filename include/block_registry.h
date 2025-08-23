#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#define BLOCK_COUNT 9
#define BLOCK_ATLAS_SIZE 9

#include <stdbool.h>
#include <stdint.h>

#include <raylib.h>

typedef enum {
	BLOCK_FLAG_NONE,
	BLOCK_FLAG_LOG_LIKE
} BlockFlag;

typedef struct {
	const char* name;
	uint8_t atlas_idx;
	uint8_t lightLevel;
	BlockFlag flag;
	bool transparent;
	bool solid;
	bool flipH;
	bool flipV;
} BlockRegistry;

void block_registry_init();
BlockRegistry* br_get_block_registry(size_t idx);
Rectangle br_get_block_texture_rect(size_t idx, bool flipH, bool flipV);
Rectangle br_get_block_uvs(size_t idx, bool flipH, bool flipV);
void block_registry_free();

#endif