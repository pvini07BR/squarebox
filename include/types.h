#ifndef TYPES_H
#define TYPES_H

#define TILE_SIZE 32

#define CHUNK_WIDTH 16
#define CHUNK_AREA CHUNK_WIDTH*CHUNK_WIDTH

#include <stdint.h>
#include <stddef.h>

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
	size_t state;
} BlockInstance;

#endif