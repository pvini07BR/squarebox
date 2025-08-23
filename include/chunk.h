#ifndef CHUNK_H
#define CHUNK_H

#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

#define CHUNK_WIDTH 16
#define CHUNK_AREA CHUNK_WIDTH*CHUNK_WIDTH

#define CHUNK_VERTEX_COUNT CHUNK_AREA * 6

typedef struct {
    unsigned int x;
    unsigned int y;
} Vector2u;

typedef struct {
	int x;
	int y;
} Vector2i;

typedef struct {
	void* up;
	void* right;
	void* down;
	void* left;

	void* upLeft;
	void* upRight;
	void* downLeft;
	void* downRight;
} ChunkNeighbors;

typedef struct {
	// ID Number in the Block Registry.
	uint8_t id;
	// Member variable used for block states.
	// Its usage depends on what flag is set
	// for a specific block.
	uint8_t state;
} BlockInstance;

typedef struct {
	unsigned int seed;
	Vector2i position;
	BlockInstance blocks[CHUNK_AREA];
	BlockInstance walls[CHUNK_AREA];
    uint8_t light[CHUNK_AREA];
	ChunkNeighbors neighbors;
	bool initializedMeshes;
	Mesh blockMesh;
	Mesh wallMesh;
} Chunk;

void chunk_init_meshes(Chunk* chunk);
void chunk_regenerate(Chunk* chunk);
void chunk_genmesh(Chunk* chunk);
void chunk_draw(Chunk* chunk, Material* material);
void chunk_free_meshes(Chunk* chunk);

void chunk_fill_light(Chunk* chunk, Vector2u startPoint, uint8_t newLightValue);

// Position is relative to the chunk origin.
void chunk_set_block(Chunk* chunk, Vector2u position, BlockInstance blockValue, bool isWall);
// Position is relative to the chunk origin.
BlockInstance chunk_get_block(Chunk* chunk, Vector2u position, bool isWall);
// Position is relative to the chunk origin.
void chunk_set_light(Chunk* chunk, Vector2u position, uint8_t value);
// Position is relative to the chunk origin.
uint8_t chunk_get_light(Chunk* chunk, Vector2u position);
// Position is relative to the chunk origin, but it accepts
// negative values so that it is used to get the block from the neighboring
// chunk.
BlockInstance chunk_get_block_extrapolating(Chunk* chunk, Vector2i position, bool isWall);
// Position is relative to the chunk origin, but it accepts
// negative values so that it is used to get the light from the neighboring
// chunk.
uint8_t chunk_get_light_extrapolating(Chunk* chunk, Vector2i position);

#endif