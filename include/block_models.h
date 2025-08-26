#ifndef BLOCK_MODELS_H
#define BLOCK_MODELS_H

#include "chunk.h"

#include <stdint.h>

#include <raylib.h>

#define MODEL_COUNT 3

typedef struct {
    float x, y;
    float u, v;
} Vertex2D;

typedef struct {
    Vertex2D* vertices;
    size_t vertexCount;
} BlockModel;

void block_models_init();
int block_models_get_vertex_count(size_t model_idx);
void block_models_build_mesh(Mesh* output, size_t idx, size_t atlasIdx, bool flipH, bool flipV);
void bm_set_block_model(size_t* offsets, Mesh* mesh, Vector2u position, Color color, size_t modelIdx, size_t atlasIdx);
void block_models_free();

#endif