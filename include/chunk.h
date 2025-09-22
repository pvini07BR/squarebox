#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>
#include <stdint.h>

#include <raylib.h>

#include "types.h"
#include "lists/container_vector.h"
#include "lists/block_tick_list.h"

#define CHUNK_VERTEX_COUNT CHUNK_AREA * 6

typedef enum {
	NEIGHBOR_TOP = 0,
	NEIGHBOR_RIGHT = 1,
	NEIGHBOR_BOTTOM = 2,
	NEIGHBOR_LEFT = 3,

	NEIGHBOR_TOP_LEFT = 4,
	NEIGHBOR_TOP_RIGHT = 5,
	NEIGHBOR_BOTTOM_RIGHT = 6,
	NEIGHBOR_BOTTOM_LEFT = 7
} NeighborDirection;

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
	unsigned int seed;
	Vector2i position;
	BlockInstance blocks[CHUNK_AREA];
	BlockInstance walls[CHUNK_AREA];
    uint8_t light[CHUNK_AREA];
	size_t blockOffsets[CHUNK_AREA];
	size_t wallOffsets[CHUNK_AREA];
	ContainerVector containerVec;
	BlockTickList blockTickList;
	ChunkNeighbors neighbors;
	bool initializedMeshes;
	Mesh blockMesh;
	Mesh wallMesh;
	Mesh liquidMesh;
} Chunk;

typedef struct {
	BlockInstance* block;
	// Pointer to the block registry.
	// couldn't include the actual type because of stupid
	// circular dependency.
	void* reg;
	Chunk* chunk;
	Vector2u position;
	uint8_t idx;
} BlockExtraResult;

typedef struct {
	uint8_t* light;
	Chunk* chunk;
	Vector2u position;
} LightExtraResult;

typedef struct {
	BlockExtraResult replaced;
	BlockExtraResult down;
} DownProjectionResult;

void chunk_init(Chunk* chunk);
void chunk_regenerate(Chunk* chunk);
void chunk_genmesh(Chunk* chunk);
void chunk_draw(Chunk* chunk);
void chunk_draw_liquids(Chunk* chunk);
void chunk_tick(Chunk* chunk, uint8_t tick_value);
void chunk_free(Chunk* chunk);

void chunk_fill_light(Chunk* chunk, Vector2u startPoint, uint8_t newLightValue);

// This function will project downards from a starting point until it finds a replaceable block (like grass or air).
// The returned value is a struct that contains pointers to the replaceable block, and the block below it.
DownProjectionResult chunk_get_block_projected_downwards(Chunk* chunk, Vector2u startPoint, bool isWall, bool goToNeighbor);

// The extrapolating functions are functions that checks if the requested position is inside the chunk.
// if it is, it will return the block relative to that chunk.
// Otherwise, it will get the block from a neighboring chunk.

// Pointer getter functions
BlockInstance* chunk_get_block_ptr(Chunk* chunk, Vector2u position, bool isWall);
BlockExtraResult chunk_get_block_extrapolating_ptr(Chunk* chunk, Vector2i position, bool isWall);
uint8_t* chunk_get_light_ptr(Chunk* chunk, Vector2u position);
LightExtraResult chunk_get_light_extrapolating_ptr(Chunk* chunk, Vector2i position);

// Block setter functions
void chunk_set_block(Chunk* chunk, Vector2u position, BlockInstance blockValue, bool isWall, bool update_lighting);
BlockExtraResult chunk_set_block_extrapolating(Chunk* chunk, Vector2i position, BlockInstance blockValue, bool isWall, bool update_lighting);

// Block getter functions
BlockInstance chunk_get_block(Chunk* chunk, Vector2u position, bool isWall);
BlockInstance chunk_get_block_extrapolating(Chunk* chunk, Vector2i position, bool isWall);

// Light setter functions
void chunk_set_light(Chunk* chunk, Vector2u position, uint8_t value);
void chunk_set_light_extrapolating(Chunk* chunk, Vector2i position, uint8_t value);

// Light getter functions
uint8_t chunk_get_light(Chunk* chunk, Vector2u position);
uint8_t chunk_get_light_extrapolating(Chunk* chunk, Vector2i position);

// Neighbor (4 corners) getter functions
void chunk_get_block_neighbors(Chunk* chunk, Vector2u position, bool isWall, BlockInstance output[4]);
void chunk_get_block_neighbors_extra(Chunk* chunk, Vector2u position, bool isWall, BlockExtraResult output[4]);
void chunk_get_light_neighbors(Chunk* chunk, Vector2u position, uint8_t output[4]);

// Neighbor (8 corners) getter functions
void chunk_get_block_neighbors_with_corners(Chunk* chunk, Vector2u position, bool isWall, BlockInstance output[8]);
void chunk_get_light_neighbors_with_corners(Chunk* chunk, Vector2u position, uint8_t output[8]);

#endif