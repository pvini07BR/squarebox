#ifndef CHUNK_LAYER_H
#define CHUNK_LAYER_H

#include "types.h"
#include <raylib.h>
#include <stdbool.h>
#include <stddef.h>

#define CHUNK_VERTEX_COUNT CHUNK_AREA * 6

typedef struct {
    BlockInstance blocks[CHUNK_AREA];
    size_t vertexOffsets[CHUNK_AREA];
    Mesh mesh;
    bool initializedMesh;
    uint8_t brightness;
} ChunkLayer;

void chunk_layer_init(ChunkLayer* layer, uint8_t brightness);
void chunk_layer_genmesh(ChunkLayer* layer, uint8_t lightmap[CHUNK_AREA]);
void chunk_layer_draw(ChunkLayer* layer);
void chunk_layer_free(ChunkLayer* layer);

#endif