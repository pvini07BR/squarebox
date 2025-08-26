#include "block_models.h"
#include "block_registry.h"
#include "texture_atlas.h"
#include "defines.h"

#include <stdlib.h>

static BlockModel models[MODEL_COUNT];

void block_models_init() {
	// Common block quad model
	models[0] = (BlockModel) {
		.vertexCount = 6,
		.vertices = malloc(sizeof(Vertex2D) * 6)
	};
	models[0].vertices[0] = (Vertex2D){ 0,         0,         0, 0 };
	models[0].vertices[1] = (Vertex2D){ TILE_SIZE, TILE_SIZE, 1, 1 };
	models[0].vertices[2] = (Vertex2D){ 0,         TILE_SIZE, 0, 1 };
	
	models[0].vertices[3] = (Vertex2D){ 0,         0,         0, 0 };
	models[0].vertices[4] = (Vertex2D){ TILE_SIZE, TILE_SIZE, 1, 1 };
	models[0].vertices[5] = (Vertex2D){ TILE_SIZE, 0,         1, 0 };

	// Slab model
	models[1] = (BlockModel){
		.vertexCount = 6,
		.vertices = malloc(sizeof(Vertex2D) * 6)
	};
	models[1].vertices[0] = (Vertex2D){ 0,                  TILE_SIZE / 2.0f, 0.0f, 0.5f };
	models[1].vertices[1] = (Vertex2D){ TILE_SIZE,          TILE_SIZE,        1.0f, 1.0f };
	models[1].vertices[2] = (Vertex2D){ 0,                  TILE_SIZE,        0.0f, 1.0f };
		   													           
	models[1].vertices[3] = (Vertex2D){ 0,                  TILE_SIZE / 2.0f, 0.0f, 0.5f };
	models[1].vertices[4] = (Vertex2D){ TILE_SIZE,          TILE_SIZE,        1.0f, 1.0f };
	models[1].vertices[5] = (Vertex2D){ TILE_SIZE,          TILE_SIZE / 2.0f, 1.0f, 0.5f };

	// Stairs model
	models[2] = (BlockModel){
		.vertexCount = 12,
		.vertices = malloc(sizeof(Vertex2D) * 12)
	};

	models[2].vertices[0] =  (Vertex2D){ 0,                TILE_SIZE / 2.0f, 0.0f, 0.5f };
	models[2].vertices[1] =  (Vertex2D){ TILE_SIZE,        TILE_SIZE / 2.0f, 1.0f, 0.5f };
	models[2].vertices[2] =  (Vertex2D){ 0,                TILE_SIZE,        0.0f, 1.0f };
							 					          
	models[2].vertices[3] =  (Vertex2D){ 0,                TILE_SIZE,        0.0f, 1.0f };
	models[2].vertices[4] =  (Vertex2D){ TILE_SIZE,        TILE_SIZE / 2.0f, 1.0f, 0.5f };
	models[2].vertices[5] =  (Vertex2D){ TILE_SIZE,        TILE_SIZE,        1.0f, 1.0f };
							 
	models[2].vertices[6] =  (Vertex2D){ TILE_SIZE / 2.0f, 0,                0.5f, 0.0f };
	models[2].vertices[7] =  (Vertex2D){ TILE_SIZE,        0,                1.0f, 0.0f };
	models[2].vertices[8] =  (Vertex2D){ TILE_SIZE / 2.0f, TILE_SIZE / 2.0f, 0.5f, 0.5f };

	models[2].vertices[9] =  (Vertex2D){ TILE_SIZE / 2.0f, TILE_SIZE / 2.0f, 0.5f, 0.5f };
	models[2].vertices[10] = (Vertex2D){ TILE_SIZE,        0,                1.0f, 0.0f };
	models[2].vertices[11] = (Vertex2D){ TILE_SIZE,        TILE_SIZE / 2.0f, 1.0f, 0.5f };
}

int block_models_get_vertex_count(size_t model_idx)
{
	if (model_idx < 0 || model_idx >= MODEL_COUNT) return 0;
	return models[model_idx].vertexCount;
}

void block_models_build_mesh(Mesh* output, size_t modelIdx, size_t atlasIdx, bool flipH, bool flipV) {
	if (output == NULL) return;
	if (modelIdx >= MODEL_COUNT) return;

	// Calcula UV base do atlas
	Rectangle uvRect = texture_atlas_get_uv(atlasIdx, flipH, flipV);

	output->vertexCount = models[modelIdx].vertexCount;
	output->triangleCount = models[modelIdx].vertexCount / 3;
	output->vertices = malloc(models[modelIdx].vertexCount * 3 * sizeof(float));
	output->texcoords = malloc(models[modelIdx].vertexCount * 2 * sizeof(float));

	for (int i = 0; i < models[modelIdx].vertexCount; i++) {
		output->vertices[(i * 3)] = models[modelIdx].vertices[i].x;
		output->vertices[(i * 3) + 1] = models[modelIdx].vertices[i].y;
		output->vertices[(i * 3) + 2] = 0.0f;

		float u_rel = models[modelIdx].vertices[i].u;
		float v_rel = models[modelIdx].vertices[i].v;

		float u = uvRect.x + u_rel * uvRect.width;
		float v = uvRect.y + v_rel * uvRect.height;

		output->texcoords[(i * 2)] = u;
		output->texcoords[(i * 2) + 1] = v;
	}

	UploadMesh(output, false);
}

void bm_set_block_model(size_t* offsets, Mesh* mesh, Vector2u position, Color color, size_t modelIdx, size_t atlasIdx)
{
	if (position.x < 0 || position.x >= CHUNK_WIDTH || position.y < 0 || position.y >= CHUNK_WIDTH) return;
	if (modelIdx < 0 || modelIdx >= MODEL_COUNT) return;
	if (!mesh) return;

	int i = position.x + (position.y * CHUNK_WIDTH);
	int vertexOffset = offsets[i];

	Rectangle uvRect = texture_atlas_get_uv(atlasIdx, false, false);

	for (int v = 0; v < models[modelIdx].vertexCount; v++) {
		mesh->vertices[(vertexOffset + v) * 3 + 0] = models[modelIdx].vertices[v].x + (position.x * TILE_SIZE);
		mesh->vertices[(vertexOffset + v) * 3 + 1] = models[modelIdx].vertices[v].y + (position.y * TILE_SIZE);
		mesh->vertices[(vertexOffset + v) * 3 + 2] = 0.0f;

		mesh->texcoords[(vertexOffset + v) * 2 + 0] = uvRect.x + models[modelIdx].vertices[v].u * uvRect.width;
		mesh->texcoords[(vertexOffset + v) * 2 + 1] = uvRect.y + models[modelIdx].vertices[v].v * uvRect.height;

		mesh->colors[(vertexOffset + v) * 4 + 0] = color.r;
		mesh->colors[(vertexOffset + v) * 4 + 1] = color.g;
		mesh->colors[(vertexOffset + v) * 4 + 2] = color.b;
		mesh->colors[(vertexOffset + v) * 4 + 3] = color.a;
	}
}

void block_models_free() {
	for (int i = 0; i < MODEL_COUNT; i++) {
		if (models[i].vertices) free(models[i].vertices);
	}
}
