#ifndef TYPES_H
#define TYPES_H

#define TILE_SIZE 32

#define CHUNK_WIDTH 16
#define CHUNK_AREA CHUNK_WIDTH*CHUNK_WIDTH

#include <stdint.h>
#include <stddef.h>

#include <raylib.h>

typedef enum {
	CHUNK_LAYER_BACKGROUND,
	CHUNK_LAYER_FOREGROUND,
	CHUNK_LAYER_COUNT
} ChunkLayerEnum;

typedef struct {
	unsigned int x;
	unsigned int y;
} Vector2u;

typedef struct {
	int x;
	int y;
} Vector2i;

typedef struct {
	// ID Number in the Block Registry.
	uint8_t id;
	// Member variable used for block states.
	// Its usage depends on what flag is set
	// for a specific block.
	uint8_t state;
	// Pointer to a external data. Set NULL if the block
	// does not have an associated external data.
	void* data;
} BlockInstance;

typedef struct {
	Font font;
	const char* str;
	Vector2 bounds;
	float fontSize;
	float spacing;
} Label;

Label create_label(const char* str, float fontSize, float spacing, Font font);

#endif