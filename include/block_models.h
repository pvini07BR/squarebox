#ifndef BLOCK_MODELS_H
#define BLOCK_MODELS_H

#include "chunk.h"

#include <stdint.h>

#include <raylib.h>

typedef enum {
    BLOCK_MODEL_QUAD,
    BLOCK_MODEL_SLAB,
    BLOCK_MODEL_STAIRS,
    BLOCK_MODEL_NUB,
    BLOCK_MODEL_TORCH,
    BLOCK_MODEL_TORCH_WALL,
    BLOCK_MODEL_COUNT
} BlockModelEnum;

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
void block_models_build_mesh(Mesh* output, size_t modelIdx, size_t atlasIdx, bool flipUVH, bool flipUVV, bool flipModelH, bool flipModelV, int rotation);
void bm_set_block_model(size_t* offsets, Mesh* mesh, Vector2u position, Color colors[4],
                       size_t modelIdx, size_t atlasIdx,
                       bool flipUV_H, bool flipUV_V,
                       bool flipModelH, bool flipModelV,
                       int rotation);
void block_models_free();

#endif