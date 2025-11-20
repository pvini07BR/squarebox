#ifndef CHUNK_LAYER_H
#define CHUNK_LAYER_H

#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>

#include "types.h"

#define CHUNK_VERTEX_COUNT CHUNK_AREA * 6

typedef struct {
    BlockInstance blocks[CHUNK_AREA];
    size_t vertexOffsets[CHUNK_AREA];
    Mesh mesh;
    bool initializedMesh;
} ChunkLayer;

void chunk_layer_init(ChunkLayer* layer);
void chunk_layer_genmesh(ChunkLayer* layer, ChunkLayerEnum layer_id, ChunkLayerEnum front_layer_id, void* c, unsigned int chunk_pos_seed, uint8_t brightness);
void chunk_layer_draw(ChunkLayer* layer);

void chunk_layer_free_mesh(ChunkLayer* layer);
void chunk_layer_free_block_data(ChunkLayer* layer);

#endif