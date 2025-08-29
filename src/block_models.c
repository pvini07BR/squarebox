#include "block_models.h"
#include "raylib.h"
#include "raymath.h"
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

	float unit = 1.0f / (float)TILE_SIZE;

	Vector2 p0 = { unit * 14, unit * 14 };
	Vector2 p1 = { unit * 14, unit * 32 };
	Vector2 p2 = { unit * 18, unit * 32 };
	Vector2 p3 = { unit * 18, unit * 14 };

	models[BLOCK_MODEL_TORCH].vertices[0] = (Vertex2D){ p0.x * TILE_SIZE, p0.y * TILE_SIZE, p0.x, p0.y };
	models[BLOCK_MODEL_TORCH].vertices[1] = (Vertex2D){ p1.x * TILE_SIZE, p1.y * TILE_SIZE, p1.x, p1.y };
	models[BLOCK_MODEL_TORCH].vertices[2] = (Vertex2D){ p2.x * TILE_SIZE, p2.y * TILE_SIZE, p2.x, p2.y };

	models[BLOCK_MODEL_TORCH].vertices[3] = (Vertex2D){ p0.x * TILE_SIZE, p0.y * TILE_SIZE, p0.x, p0.y };
	models[BLOCK_MODEL_TORCH].vertices[4] = (Vertex2D){ p2.x * TILE_SIZE, p2.y * TILE_SIZE, p2.x, p2.y };
	models[BLOCK_MODEL_TORCH].vertices[5] = (Vertex2D){ p3.x * TILE_SIZE, p3.y * TILE_SIZE, p3.x, p3.y };

	Vector2 p = Vector2Rotate((Vector2){ models[BLOCK_MODEL_TORCH].vertices[0].x, models[BLOCK_MODEL_TORCH].vertices[0].y }, PI / 2.0f);

	// Torch on the wall model
	models[BLOCK_MODEL_TORCH_WALL] = (BlockModel){
		.vertexCount = 6,
		.vertices = malloc(sizeof(Vertex2D) * 6)
	};

	Vector2 new_p0 = (Vector2){ unit * 28, unit * -1.25f };
	Vector2 new_p1 = (Vector2){ unit * 28, unit * 15.25f };
	Vector2 new_p2 = (Vector2){ unit * 32, unit * 15.25f };
	Vector2 new_p3 = (Vector2){ unit * 32, unit * -1.25f };

	Vector2 rp0 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p0, new_p2), -PI / 8.0f), new_p2);
	Vector2 rp1 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p1, new_p2), -PI / 8.0f), new_p2);
	Vector2 rp2 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p2, new_p2), -PI / 8.0f), new_p2);
	Vector2 rp3 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p3, new_p2), -PI / 8.0f), new_p2);

	models[BLOCK_MODEL_TORCH_WALL].vertices[0] = (Vertex2D){ rp0.x * TILE_SIZE, rp0.y * TILE_SIZE, p0.x, p0.y };
	models[BLOCK_MODEL_TORCH_WALL].vertices[1] = (Vertex2D){ rp1.x * TILE_SIZE, rp1.y * TILE_SIZE, p1.x, p1.y };
	models[BLOCK_MODEL_TORCH_WALL].vertices[2] = (Vertex2D){ rp2.x * TILE_SIZE, rp2.y * TILE_SIZE, p2.x, p2.y };

	models[BLOCK_MODEL_TORCH_WALL].vertices[3] = (Vertex2D){ rp0.x * TILE_SIZE, rp0.y * TILE_SIZE, p0.x, p0.y };
	models[BLOCK_MODEL_TORCH_WALL].vertices[4] = (Vertex2D){ rp2.x * TILE_SIZE, rp2.y * TILE_SIZE, p2.x, p2.y };
	models[BLOCK_MODEL_TORCH_WALL].vertices[5] = (Vertex2D){ rp3.x * TILE_SIZE, rp3.y * TILE_SIZE, p3.x, p3.y };
}

int block_models_get_vertex_count(size_t model_idx)
{
	if (model_idx < 0 || model_idx >= BLOCK_MODEL_COUNT) return 0;
	return models[model_idx].vertexCount;
}

void block_models_build_mesh(Mesh* output, size_t modelIdx, size_t atlasIdx, bool flipUVH, bool flipUVV, bool flipModelH, bool flipModelV, int rotation)
{
    if (!output) return;
    if (modelIdx >= BLOCK_MODEL_COUNT) return;

	output->vertexCount = models[modelIdx].vertexCount;
	output->triangleCount = output->vertexCount / 3;
	output->vertices = malloc(sizeof(float) * 3 * output->vertexCount);
	output->texcoords = malloc(sizeof(float) * 2 * output->vertexCount);

	bm_set_block_model(NULL, output, (Vector2u){0,0}, NULL, modelIdx, atlasIdx, flipUVH, flipUVV, flipModelH, flipModelV, rotation);
	UploadMesh(output, false);
}

static inline void swap_vertex_data(Mesh* mesh, int a, int b)
{
    // swap positions (3 floats)
    for (int k = 0; k < 3; k++) {
        float tmpf = mesh->vertices[a * 3 + k];
        mesh->vertices[a * 3 + k] = mesh->vertices[b * 3 + k];
        mesh->vertices[b * 3 + k] = tmpf;
    }
    // swap texcoords (2 floats)
    for (int k = 0; k < 2; k++) {
        float tmpf = mesh->texcoords[a * 2 + k];
        mesh->texcoords[a * 2 + k] = mesh->texcoords[b * 2 + k];
        mesh->texcoords[b * 2 + k] = tmpf;
    }
	if (mesh->colors == NULL) return;
    // swap colors (4 unsigned char)
    for (int k = 0; k < 4; k++) {
        unsigned char tmpc = mesh->colors[a * 4 + k];
        mesh->colors[a * 4 + k] = mesh->colors[b * 4 + k];
        mesh->colors[b * 4 + k] = tmpc;
    }
}

void bm_set_block_model(size_t* offsets, Mesh* mesh, Vector2u position, Color colors[4],
                       size_t modelIdx, size_t atlasIdx,
                       bool flipUV_H, bool flipUV_V,
                       bool flipModelH, bool flipModelV,
                       int rotation)
{
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return;
    if (modelIdx >= BLOCK_MODEL_COUNT) return;
    if (!mesh) return;

    int rot = rotation & 3;
    int i = position.x + (position.y * CHUNK_WIDTH);
	int vertexOffset = 0;
	if (offsets) {
		vertexOffset = offsets[i];
	}
    Rectangle uvRect = texture_atlas_get_uv(atlasIdx, flipUV_H, flipUV_V);

    const float tileHalf = (float)TILE_SIZE * 0.5f;
    const float baseX = (float)position.x * (float)TILE_SIZE;
    const float baseY = (float)position.y * (float)TILE_SIZE;

    const int vcount = models[modelIdx].vertexCount;

    for (int v = 0; v < vcount; v++) {
        float vx = models[modelIdx].vertices[v].x;
        float vy = models[modelIdx].vertices[v].y;

        float lx = vx - tileHalf;
        float ly = vy - tileHalf;

        if (flipModelH) lx = -lx;
        if (flipModelV) ly = -ly;

        float rx, ry;
        switch (rot) {
        default:
        case 0: rx = lx;  ry = ly;  break;
        case 1: rx = -ly; ry = lx;  break;
        case 2: rx = -lx; ry = -ly; break;
        case 3: rx =  ly; ry = -lx; break;
        }

        mesh->vertices[(vertexOffset + v) * 3 + 0] = baseX + tileHalf + rx;
        mesh->vertices[(vertexOffset + v) * 3 + 1] = baseY + tileHalf + ry;
        mesh->vertices[(vertexOffset + v) * 3 + 2] = 0.0f;

        float u_rel = models[modelIdx].vertices[v].u;
        float v_rel = models[modelIdx].vertices[v].v;

        if (modelIdx > 0) {
            float lu = u_rel - 0.5f;
            float lv = v_rel - 0.5f;
            float ru, rv;
            switch (rot) {
            default:
            case 0: ru = lu;  rv = lv;  break;
            case 1: ru = -lv; rv = lu;  break;
            case 2: ru = -lu; rv = -lv; break;
            case 3: ru = lv;  rv = -lu; break;
            }
            u_rel = 0.5f + ru;
            v_rel = 0.5f + rv;
        }

        if (flipModelH) u_rel = 1.0f - u_rel;
        if (flipModelV) v_rel = 1.0f - v_rel;

        mesh->texcoords[(vertexOffset + v) * 2 + 0] = uvRect.x + u_rel * uvRect.width;
        mesh->texcoords[(vertexOffset + v) * 2 + 1] = uvRect.y + v_rel * uvRect.height;

		if (colors) {
			int colorIdx = 0;
			if (modelIdx != BLOCK_MODEL_STAIRS) {
				if (v == 0) colorIdx = 0;    // Top left
				if (v == 1) colorIdx = 3;    // Bottom left
				if (v == 2) colorIdx = 2;    // Bottom right
				if (v == 3) colorIdx = 0;    // Top left
				if (v == 4) colorIdx = 2;    // Bottom right
				if (v == 5) colorIdx = 1;    // Top right
			} else {
				colorIdx = 0;
			}

			mesh->colors[(vertexOffset + v) * 4 + 0] = colors[colorIdx].r;
			mesh->colors[(vertexOffset + v) * 4 + 1] = colors[colorIdx].g;
			mesh->colors[(vertexOffset + v) * 4 + 2] = colors[colorIdx].b;
			mesh->colors[(vertexOffset + v) * 4 + 3] = colors[colorIdx].a;
		}
    }

    if (flipModelH ^ flipModelV) {
        int triCount = vcount / 3;
        for (int t = 0; t < triCount; t++) {
            int a = vertexOffset + t * 3 + 0;
            int b = vertexOffset + t * 3 + 1;
            int c = vertexOffset + t * 3 + 2;
            swap_vertex_data(mesh, b, c);
        }
    }
}

void block_models_free() {
	for (int i = 0; i < BLOCK_MODEL_COUNT; i++) {
		if (models[i].vertices) free(models[i].vertices);
	}
}
