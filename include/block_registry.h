#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#define BLOCK_COUNT 11

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <raylib.h>

typedef enum {
	BLOCK_FLAG_NONE,
	BLOCK_FLAG_LOG_LIKE,
	BLOCK_FLAG_CONTAINER
} BlockFlag;

typedef struct {
	const char* name;
	size_t atlas_idx;
	size_t model_idx;
	uint8_t lightLevel;
	BlockFlag flag;
	bool transparent;
	bool solid;
	bool flipH;
	bool flipV;
} BlockRegistry;

void block_registry_init();
BlockRegistry* br_get_block_registry(size_t idx);
void block_registry_free();

#endif