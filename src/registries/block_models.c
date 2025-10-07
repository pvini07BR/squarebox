#include "registries/block_models.h"
#include "raylib.h"
#include "raymath.h"
#include "texture_atlas.h"
#include "types.h"

#include <stdio.h>
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

	Vector2 p0 = { unit * 9, unit * 2 };
	Vector2 p1 = { unit * 9, unit * 32 };
	Vector2 p2 = { unit * 24, unit * 32 };
	Vector2 p3 = { unit * 24, unit * 2 };

	models[BLOCK_MODEL_TORCH].vertices[0] = (Vertex2D){ p0.x * TILE_SIZE, p0.y * TILE_SIZE, p0.x, p0.y };
	models[BLOCK_MODEL_TORCH].vertices[1] = (Vertex2D){ p1.x * TILE_SIZE, p1.y * TILE_SIZE, p1.x, p1.y };
	models[BLOCK_MODEL_TORCH].vertices[2] = (Vertex2D){ p2.x * TILE_SIZE, p2.y * TILE_SIZE, p2.x, p2.y };

	models[BLOCK_MODEL_TORCH].vertices[3] = (Vertex2D){ p0.x * TILE_SIZE, p0.y * TILE_SIZE, p0.x, p0.y };
	models[BLOCK_MODEL_TORCH].vertices[4] = (Vertex2D){ p2.x * TILE_SIZE, p2.y * TILE_SIZE, p2.x, p2.y };
	models[BLOCK_MODEL_TORCH].vertices[5] = (Vertex2D){ p3.x * TILE_SIZE, p3.y * TILE_SIZE, p3.x, p3.y };

	// Torch on the wall model
	models[BLOCK_MODEL_TORCH_WALL_RIGHT] = (BlockModel){
		.vertexCount = 6,
		.vertices = malloc(sizeof(Vertex2D) * 6)
	};

	Vector2 translation = (Vector2){ unit * 12, -(unit * 10) };

	Vector2 new_p0 = Vector2Add(p0, translation);
	Vector2 new_p1 = Vector2Add(p1, translation);
	Vector2 new_p2 = Vector2Add(p2, translation);
	Vector2 new_p3 = Vector2Add(p3, translation);

	Vector2 rp0 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p0, new_p2), -PI / 8.0f), new_p2);
	Vector2 rp1 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p1, new_p2), -PI / 8.0f), new_p2);
	Vector2 rp2 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p2, new_p2), -PI / 8.0f), new_p2);
	Vector2 rp3 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p3, new_p2), -PI / 8.0f), new_p2);

	models[BLOCK_MODEL_TORCH_WALL_RIGHT].vertices[0] = (Vertex2D){ rp0.x * TILE_SIZE, rp0.y * TILE_SIZE, p0.x, p0.y };
	models[BLOCK_MODEL_TORCH_WALL_RIGHT].vertices[1] = (Vertex2D){ rp1.x * TILE_SIZE, rp1.y * TILE_SIZE, p1.x, p1.y };
	models[BLOCK_MODEL_TORCH_WALL_RIGHT].vertices[2] = (Vertex2D){ rp2.x * TILE_SIZE, rp2.y * TILE_SIZE, p2.x, p2.y };

	models[BLOCK_MODEL_TORCH_WALL_RIGHT].vertices[3] = (Vertex2D){ rp0.x * TILE_SIZE, rp0.y * TILE_SIZE, p0.x, p0.y };
	models[BLOCK_MODEL_TORCH_WALL_RIGHT].vertices[4] = (Vertex2D){ rp2.x * TILE_SIZE, rp2.y * TILE_SIZE, p2.x, p2.y };
	models[BLOCK_MODEL_TORCH_WALL_RIGHT].vertices[5] = (Vertex2D){ rp3.x * TILE_SIZE, rp3.y * TILE_SIZE, p3.x, p3.y };

	models[BLOCK_MODEL_TORCH_WALL_LEFT] = (BlockModel){
		.vertexCount = 6,
		.vertices = malloc(sizeof(Vertex2D) * 6)
	};

	translation = (Vector2){ unit * -28, -(unit * 10) };

	new_p0 = Vector2Add(p0, translation);
	new_p1 = Vector2Add(p1, translation);
	new_p2 = Vector2Add(p2, translation);
	new_p3 = Vector2Add(p3, translation);

	rp0 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p0, new_p1), PI / 8.0f), new_p2);
	rp1 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p1, new_p1), PI / 8.0f), new_p2);
	rp2 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p2, new_p1), PI / 8.0f), new_p2);
	rp3 = Vector2Add(Vector2Rotate(Vector2Subtract(new_p3, new_p1), PI / 8.0f), new_p2);

	models[BLOCK_MODEL_TORCH_WALL_LEFT].vertices[0] = (Vertex2D){ rp0.x * TILE_SIZE, rp0.y * TILE_SIZE, p0.x, p0.y };
	models[BLOCK_MODEL_TORCH_WALL_LEFT].vertices[1] = (Vertex2D){ rp1.x * TILE_SIZE, rp1.y * TILE_SIZE, p1.x, p1.y };
	models[BLOCK_MODEL_TORCH_WALL_LEFT].vertices[2] = (Vertex2D){ rp2.x * TILE_SIZE, rp2.y * TILE_SIZE, p2.x, p2.y };

	models[BLOCK_MODEL_TORCH_WALL_LEFT].vertices[3] = (Vertex2D){ rp0.x * TILE_SIZE, rp0.y * TILE_SIZE, p0.x, p0.y };
	models[BLOCK_MODEL_TORCH_WALL_LEFT].vertices[4] = (Vertex2D){ rp2.x * TILE_SIZE, rp2.y * TILE_SIZE, p2.x, p2.y };
	models[BLOCK_MODEL_TORCH_WALL_LEFT].vertices[5] = (Vertex2D){ rp3.x * TILE_SIZE, rp3.y * TILE_SIZE, p3.x, p3.y };
}

int block_models_get_vertex_count(size_t model_idx)
{
	if (model_idx >= BLOCK_MODEL_COUNT) return 0;
	return models[model_idx].vertexCount;
}

void block_models_build_mesh(Mesh* output, BlockVariant variant)
{
    if (!output) return;
    if (variant.model_idx >= BLOCK_MODEL_COUNT) return;

	output->vertexCount = models[variant.model_idx].vertexCount;
	output->triangleCount = output->vertexCount / 3;
	output->vertices = malloc(sizeof(float) * 3 * output->vertexCount);
	output->texcoords = malloc(sizeof(float) * 2 * output->vertexCount);

	bm_set_block_model(NULL, output, (Vector2u){0,0}, NULL, variant, false, false);
	UploadMesh(output, false);
}

void bm_set_block_model(size_t* offsets, Mesh* mesh, Vector2u position, Color colors[4], BlockVariant variant, bool flipUVH, bool flipUVV)
{
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return;
    if (variant.model_idx >= BLOCK_MODEL_COUNT) return;
    if (!mesh) return;

    size_t index = position.x + (position.y * CHUNK_WIDTH);
    size_t vertexOffset = 0;
    if (offsets) {
        vertexOffset = offsets[index];
    }
    BlockModel* model = &models[variant.model_idx];

    for (size_t v = 0; v < model->vertexCount; v++) {
        Vertex2D vert = model->vertices[v];
        float vert_x = vert.x;
        float vert_y = vert.y;

        float vert_u = vert.u;
        float vert_v = vert.v;

        uint8_t rot = variant.rotation & 3;
        switch (rot) {
            case 0:
                break;
            case 1: {
                float tmp = vert_x;
                vert_x = TILE_SIZE - vert_y;
                vert_y = tmp;
                if (variant.uv_lock) {
                    float old_v = vert_v;
                    vert_u = 1.0f - old_v;
                    vert_v = tmp / TILE_SIZE;
                }
                break;
            }
            case 2:
                vert_x = TILE_SIZE - vert_x;
                vert_y = TILE_SIZE - vert_y;
                if (variant.uv_lock) {
                    vert_u = 1.0f - vert_u;
                    vert_v = 1.0f - vert_v;
                }
                break;
            case 3: {
                float tmp = vert_x;
                vert_x = vert_y;
                vert_y = TILE_SIZE - tmp;
                if (variant.uv_lock) {
                    float old_tmp = tmp;
                    vert_u = vert_v;
                    vert_v = 1.0f - (old_tmp / TILE_SIZE);
                }
                break;
            }
        }

        mesh->vertices[(v + vertexOffset) * 3 + 0] = vert_x + (position.x * TILE_SIZE);
        mesh->vertices[(v + vertexOffset) * 3 + 1] = vert_y + (position.y * TILE_SIZE);
        mesh->vertices[(v + vertexOffset) * 3 + 2] = 0.0f;

		Vector2 p = texture_atlas_get_uv(variant.atlas_idx, variant.atlas_variant, (Vector2) { vert_u, vert_v }, flipUVH, flipUVV);

		mesh->texcoords[(v + vertexOffset) * 2 + 0] = p.x;
        mesh->texcoords[(v + vertexOffset) * 2 + 1] = p.y;

        if (colors) {
            if (variant.model_idx == BLOCK_MODEL_QUAD) {
                int colorIdx = 0;

                if (v == 0) colorIdx = 0;    // Top left
                if (v == 1) colorIdx = 3;    // Bottom left
                if (v == 2) colorIdx = 2;    // Bottom right
                if (v == 3) colorIdx = 0;    // Top left
                if (v == 4) colorIdx = 2;    // Bottom right
                if (v == 5) colorIdx = 1;    // Top right

                mesh->colors[(vertexOffset + v) * 4 + 0] = colors[colorIdx].r;
                mesh->colors[(vertexOffset + v) * 4 + 1] = colors[colorIdx].g;
                mesh->colors[(vertexOffset + v) * 4 + 2] = colors[colorIdx].b;
                mesh->colors[(vertexOffset + v) * 4 + 3] = colors[colorIdx].a;
            }
            else {
                Color average = (Color){
                    .r = (colors[0].r + colors[1].r + colors[2].r + colors[3].r) / 4.0f,
                    .g = (colors[0].g + colors[1].g + colors[2].g + colors[3].g) / 4.0f,
                    .b = (colors[0].b + colors[1].b + colors[2].b + colors[3].b) / 4.0f,
                    .a = (colors[0].a + colors[1].a + colors[2].a + colors[3].a) / 4.0f
                };

                mesh->colors[(vertexOffset + v) * 4 + 0] = average.r;
                mesh->colors[(vertexOffset + v) * 4 + 1] = average.g;
                mesh->colors[(vertexOffset + v) * 4 + 2] = average.b;
                mesh->colors[(vertexOffset + v) * 4 + 3] = average.a;
            }
        }
    }
}

void block_models_free() {
	for (int i = 0; i < BLOCK_MODEL_COUNT; i++) {
		if (models[i].vertices) free(models[i].vertices);
	}
}
