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
    float brightness;
} ChunkLayer;

void chunk_layer_init(ChunkLayer* layer, float brightness);
void chunk_layer_genmesh(ChunkLayer* layer);
void chunk_layer_draw(ChunkLayer* layer);
void chunk_layer_free(ChunkLayer* layer);

#endif