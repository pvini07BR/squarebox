#include "block_models.h"
#include "block_registry.h"
#include "texture_atlas.h"
#include "defines.h"

#include <stdlib.h>

static BlockModel models[BLOCK_MODEL_COUNT];

void block_models_init() {
	// Common block quad model
	models[BLOCK_MODEL_QUAD] = (BlockModel) {
		.vertexCount = 6,
		.vertices = malloc(sizeof(Vertex2D) * 6)
	};
	models[BLOCK_MODEL_QUAD].vertices[0] = (Vertex2D){ 0,         0,         0, 0 };
	models[BLOCK_MODEL_QUAD].vertices[1] = (Vertex2D){ 0,         TILE_SIZE, 0, 1 };
	models[BLOCK_MODEL_QUAD].vertices[2] = (Vertex2D){ TILE_SIZE, TILE_SIZE, 1, 1 };
	
	models[BLOCK_MODEL_QUAD].vertices[3] = (Vertex2D){ 0,         0,         0, 0 };
	models[BLOCK_MODEL_QUAD].vertices[4] = (Vertex2D){ TILE_SIZE, TILE_SIZE, 1, 1 };
	models[BLOCK_MODEL_QUAD].vertices[5] = (Vertex2D){ TILE_SIZE, 0,         1, 0 };

	// Slab model
	models[BLOCK_MODEL_SLAB] = (BlockModel){
		.vertexCount = 6,
		.vertices = malloc(sizeof(Vertex2D) * 6)
	};
	models[BLOCK_MODEL_SLAB].vertices[0] = (Vertex2D){ 0,                  TILE_SIZE / 2.0f, 0.0f, 0.5f };
	models[BLOCK_MODEL_SLAB].vertices[1] = (Vertex2D){ 0,                  TILE_SIZE,        0.0f, 1.0f };
	models[BLOCK_MODEL_SLAB].vertices[2] = (Vertex2D){ TILE_SIZE,          TILE_SIZE,        1.0f, 1.0f };
		   													           
	models[BLOCK_MODEL_SLAB].vertices[3] = (Vertex2D){ 0,                  TILE_SIZE / 2.0f, 0.0f, 0.5f };
	models[BLOCK_MODEL_SLAB].vertices[4] = (Vertex2D){ TILE_SIZE,          TILE_SIZE,        1.0f, 1.0f };
	models[BLOCK_MODEL_SLAB].vertices[5] = (Vertex2D){ TILE_SIZE,          TILE_SIZE / 2.0f, 1.0f, 0.5f };

	// Stairs model
	models[BLOCK_MODEL_STAIRS] = (BlockModel){
		.vertexCount = 12,
		.vertices = malloc(sizeof(Vertex2D) * 12)
	};

	models[BLOCK_MODEL_STAIRS].vertices[0] = (Vertex2D){ 0.0f,              TILE_SIZE / 2.0f, 0.0f, 0.5f };
	models[BLOCK_MODEL_STAIRS].vertices[1] = (Vertex2D){ 0.0f,              TILE_SIZE,        0.0f, 1.0f };
	models[BLOCK_MODEL_STAIRS].vertices[2] = (Vertex2D){ TILE_SIZE,         TILE_SIZE,        1.0f, 1.0f };

	models[BLOCK_MODEL_STAIRS].vertices[3] = (Vertex2D){ 0.0f,              TILE_SIZE / 2.0f, 0.0f, 0.5f };
	models[BLOCK_MODEL_STAIRS].vertices[4] = (Vertex2D){ TILE_SIZE,         TILE_SIZE,        1.0f, 1.0f };
	models[BLOCK_MODEL_STAIRS].vertices[5] = (Vertex2D){ TILE_SIZE,         TILE_SIZE / 2.0f, 1.0f, 0.5f };
					 
	models[BLOCK_MODEL_STAIRS].vertices[6] =  (Vertex2D){ TILE_SIZE / 2.0f, 0.0f,             0.5f, 0.0f };
	models[BLOCK_MODEL_STAIRS].vertices[7] =  (Vertex2D){ TILE_SIZE / 2.0f, TILE_SIZE / 2.0f, 0.5f, 0.5f };
	models[BLOCK_MODEL_STAIRS].vertices[8] =  (Vertex2D){ TILE_SIZE,        TILE_SIZE / 2.0f, 1.0f, 0.5f };
		   
	models[BLOCK_MODEL_STAIRS].vertices[9] =  (Vertex2D){ TILE_SIZE / 2.0f, 0.0f,             0.5f, 0.0f };
	models[BLOCK_MODEL_STAIRS].vertices[10] = (Vertex2D){ TILE_SIZE,        TILE_SIZE / 2.0f, 1.0f, 0.5f };
	models[BLOCK_MODEL_STAIRS].vertices[11] = (Vertex2D){ TILE_SIZE,        0.0f,             1.0f, 0.0f };

	// Nub model
	models[BLOCK_MODEL_NUB] = (BlockModel){
		.vertexCount = 6,
		.vertices = malloc(sizeof(Vertex2D) * 6)
	};

	models[BLOCK_MODEL_NUB].vertices[0] = (Vertex2D){ TILE_SIZE / 2.0f, TILE_SIZE / 2.0f, 0.5f, 0.5f };
	models[BLOCK_MODEL_NUB].vertices[1] = (Vertex2D){ TILE_SIZE / 2.0f, TILE_SIZE,        0.5f, 1.0f };
	models[BLOCK_MODEL_NUB].vertices[2] = (Vertex2D){ TILE_SIZE,        TILE_SIZE,        1.0f, 1.0f };

	models[BLOCK_MODEL_NUB].vertices[3] = (Vertex2D){ TILE_SIZE / 2.0f, TILE_SIZE / 2.0f, 0.5f, 0.5f };
	models[BLOCK_MODEL_NUB].vertices[4] = (Vertex2D){ TILE_SIZE,        TILE_SIZE,        1.0f, 1.0f };
	models[BLOCK_MODEL_NUB].vertices[5] = (Vertex2D){ TILE_SIZE,        TILE_SIZE / 2.0f, 1.0f, 0.5f };

	// Torch model
	models[BLOCK_MODEL_TORCH] = (BlockModel){
		.vertexCount = 6,
		.vertices = malloc(sizeof(Vertex2D) * 6)
	};

	models[BLOCK_MODEL_TORCH].vertices[0] = (Vertex2D){ TILE_SIZE * 0.4375f, TILE_SIZE * 0.4375f, 0.4375f, 0.4375f };
	models[BLOCK_MODEL_TORCH].vertices[1] = (Vertex2D){ TILE_SIZE * 0.4375f, TILE_SIZE,           0.4375f, 1.0f    };
	models[BLOCK_MODEL_TORCH].vertices[2] = (Vertex2D){ TILE_SIZE * 0.5625f, TILE_SIZE,           0.5625f, 1.0f    };

	models[BLOCK_MODEL_TORCH].vertices[3] = (Vertex2D){ TILE_SIZE * 0.4375f, TILE_SIZE * 0.4375f, 0.4375f, 0.4375f };
	models[BLOCK_MODEL_TORCH].vertices[4] = (Vertex2D){ TILE_SIZE * 0.5625f, TILE_SIZE,           0.5625f, 1.0f    };
	models[BLOCK_MODEL_TORCH].vertices[5] = (Vertex2D){ TILE_SIZE * 0.5625f, TILE_SIZE * 0.4375f, 0.5625f, 0.4375f };
}

int block_models_get_vertex_count(size_t model_idx)
{
	if (model_idx < 0 || model_idx >= BLOCK_MODEL_COUNT) return 0;
	return models[model_idx].vertexCount;
}

void block_models_build_mesh(Mesh* output, size_t modelIdx, size_t atlasIdx, bool flipH, bool flipV) {
	if (output == NULL) return;
	if (modelIdx >= BLOCK_MODEL_COUNT) return;

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

void bm_set_block_model(size_t* offsets, Mesh* mesh, Vector2u position, Color colors[4], size_t modelIdx, size_t atlasIdx, bool flipUV_H, bool flipUV_V, int rotation, bool rotateUVs)
{
	if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return;
	if (modelIdx >= BLOCK_MODEL_COUNT) return;
	if (!mesh) return;

	// normalize rotation to 0..3
	int rot = rotation & 3;

	int i = position.x + (position.y * CHUNK_WIDTH);
	int vertexOffset = offsets[i];

	Rectangle uvRect = texture_atlas_get_uv(atlasIdx, flipUV_H, flipUV_V);

	const float tileHalf = (float)TILE_SIZE * 0.5f;
	const float baseX = (float)position.x * (float)TILE_SIZE;
	const float baseY = (float)position.y * (float)TILE_SIZE;

	for (int v = 0; v < models[modelIdx].vertexCount; v++) {
		float vx = models[modelIdx].vertices[v].x;
		float vy = models[modelIdx].vertices[v].y;

		float lx = vx - tileHalf;
		float ly = vy - tileHalf;
		float rx, ry;

		switch (rot) {
		default:
		case 0: // 0°
			rx = lx; ry = ly;
			break;
		case 1: // 90° CCW
			rx = -ly; ry = lx;
			break;
		case 2: // 180°
			rx = -lx; ry = -ly;
			break;
		case 3: // 270° CCW
			rx = ly; ry = -lx;
			break;
		}

		mesh->vertices[(vertexOffset + v) * 3 + 0] = baseX + tileHalf + rx;
		mesh->vertices[(vertexOffset + v) * 3 + 1] = baseY + tileHalf + ry;
		mesh->vertices[(vertexOffset + v) * 3 + 2] = 0.0f;

		if (rotateUVs) {
			float u = models[modelIdx].vertices[v].u;
			float vuv = models[modelIdx].vertices[v].v;

			float lu = u - 0.5f;
			float lv = vuv - 0.5f;
			float ru, rv;

			switch (rot) {
			default:
			case 0:	// 0° CCW
				ru = lu; rv = lv;
				break;
			case 1: // 90° CCW
				ru = -lv; rv = lu;
				break;
			case 2:	// 180° CCW
				ru = -lu; rv = -lv;
				break;
			case 3: // 270° CCW
				ru = lv; rv = -lu;
				break;
			}

			float finalU = uvRect.x + (0.5f + ru) * uvRect.width;
			float finalV = uvRect.y + (0.5f + rv) * uvRect.height;

			mesh->texcoords[(vertexOffset + v) * 2 + 0] = finalU;
			mesh->texcoords[(vertexOffset + v) * 2 + 1] = finalV;
		}
		else {
			mesh->texcoords[(vertexOffset + v) * 2 + 0] = uvRect.x + models[modelIdx].vertices[v].u * uvRect.width;
			mesh->texcoords[(vertexOffset + v) * 2 + 1] = uvRect.y + models[modelIdx].vertices[v].v * uvRect.height;
		}

		int colorIdx = 0;
		if (modelIdx != BLOCK_MODEL_STAIRS) {
			if (v == 0) colorIdx = 0;	// Top left
			if (v == 1) colorIdx = 3;	// Bottom left
			if (v == 2) colorIdx = 2;	// Bottom right
			if (v == 3) colorIdx = 0;	// Top left
			if (v == 4) colorIdx = 2;	// Bottom right
			if (v == 5) colorIdx = 1;	// Top right
		}
		/*
		else {
			if (v == 0) colorIdx = 0;	// Bottom quad top left
			if (v == 1) colorIdx = 2;	// Bottom quad top right
			if (v == 2) colorIdx = 3;	// Bottom quad bottom left
			if (v == 3) colorIdx = 3;	// Bottom quad bottom left
			if (v == 4) colorIdx = 2;	// Bottom quad top right
			if (v == 5) colorIdx = 2;	// Bottom quad bottom right

			if (v == 6) colorIdx = 0;	// Upper quad top left
			if (v == 7) colorIdx = 1;	// Upper quad top right
			if (v == 8) colorIdx = 0;	// Upper quad bottom left
			if (v == 9) colorIdx = 0;	// Upper quad bottom left
			if (v == 10) colorIdx = 1;  // Upper quad top right
			if (v == 11) colorIdx = 2;	// Upper quad bottom right
		}
		*/

		mesh->colors[(vertexOffset + v) * 4 + 0] = colors[colorIdx].r;
		mesh->colors[(vertexOffset + v) * 4 + 1] = colors[colorIdx].g;
		mesh->colors[(vertexOffset + v) * 4 + 2] = colors[colorIdx].b;
		mesh->colors[(vertexOffset + v) * 4 + 3] = colors[colorIdx].a;
	}
}

void block_models_free() {
	for (int i = 0; i < BLOCK_MODEL_COUNT; i++) {
		if (models[i].vertices) free(models[i].vertices);
	}
}
