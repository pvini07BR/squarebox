#ifndef BLOCK_MODELS_H
#define BLOCK_MODELS_H

#include "types.h"

#include <stdint.h>
#include <stddef.h>

#include <raylib.h>

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
void block_models_build_mesh(Mesh* output, BlockVariant variant);

void bm_set_block_model(size_t* offsets, Mesh* mesh, Vector2u position, Color colors[4], BlockVariant variant);
void block_models_free();

#endif