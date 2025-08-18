#ifndef CHUNK_H
#define CHUNK_H

#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

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

typedef struct {
	unsigned int seed;
	Vector2i position;
	int blocks[CHUNK_AREA];
	int walls[CHUNK_AREA];
    uint8_t light[CHUNK_AREA];
} Chunk;

void chunk_regenerate(Chunk* chunk);
void chunk_draw(Chunk* chunk, Texture2D* blocksAtlas);
// Position is relative to the chunk origin
void chunk_set_block(Chunk* chunk, Vector2u position, int blockValue, bool isWall);
// Position is relative to the chunk origin
int chunk_get_block(Chunk* chunk, Vector2u position, bool isWall);
// Position is relative to the chunk origin
void chunk_set_light(Chunk* chunk, Vector2u position, uint8_t value);
// Position is relative to the chunk origin
uint8_t chunk_get_light(Chunk* chunk, Vector2u position);

#endif