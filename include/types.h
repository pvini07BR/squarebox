#ifndef TYPES_H
#define TYPES_H

#define TILE_SIZE 32

#define CHUNK_WIDTH 16
#define CHUNK_AREA CHUNK_WIDTH*CHUNK_WIDTH

typedef struct {
	unsigned int x;
	unsigned int y;
} Vector2u;

typedef struct {
	int x;
	int y;
} Vector2i;

#endif