#include "chunk.h"
#include "chunk_manager.h"
#include "container_vector.h"
#include "defines.h"
#include "block_registry.h"
#include "texture_atlas.h"
#include "block_models.h"
#include "liquid_spread.h"
#include "block_state_bitfields.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <string.h>

#define FNL_IMPL
#include <thirdparty/FastNoiseLite.h>

unsigned int posmod(int v, int m);
void reset_meshes(Chunk* chunk, int blockVertexCount, int wallVertexCount);
void build_quad(Chunk* chunk, size_t* offsets, BlockInstance* blocks, Mesh* mesh, bool isWall, uint8_t x, uint8_t y, uint8_t brightness);

int seed = 0;
bool wallAmbientOcclusion = true;
bool smoothLighting = true;
unsigned int wallBrightness = 128;
unsigned int wallAOvalue = 64;

void chunk_init(Chunk* chunk)
{
    if (chunk == NULL) return;

    container_vector_init(&chunk->containerVec);
    liquid_spread_list_init(&chunk->liquidSpreadList);

    chunk->initializedMeshes = false;
}

static bool chance_at(int gx, int gy, float threshold, int seed_offset) {
    fnl_state noise = fnlCreateState();
    noise.seed = seed + seed_offset;
    noise.frequency = 0.1f;
    noise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    return fnlGetNoise2D(&noise, gx, gy) > threshold;
}

void chunk_regenerate(Chunk* chunk) {
    if (!chunk) return;

    for (int i = 0; i < CHUNK_AREA; i++) {
        chunk->blocks[i] = (BlockInstance){ 0, 0 };
        chunk->walls[i] = (BlockInstance){ 0, 0 };
    }

    fnl_state terrainNoise = fnlCreateState();
    terrainNoise.seed = seed;
    terrainNoise.frequency = 0.008f;
    terrainNoise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    terrainNoise.fractal_type = FNL_FRACTAL_FBM;

    fnl_state detailNoise = fnlCreateState();
    detailNoise.seed = seed + 1337;
    detailNoise.frequency = 0.025f;
    detailNoise.noise_type = FNL_NOISE_OPENSIMPLEX2;
    detailNoise.fractal_type = FNL_FRACTAL_FBM;

    for (int w = 0; w < 2; w++) {
        for (int x = 0; x < CHUNK_WIDTH; x++) {
            int gx = chunk->position.x * CHUNK_WIDTH + x;

            float base = fnlGetNoise2D(&terrainNoise, gx * 0.5f, 0.0f);
            float detail = fnlGetNoise2D(&detailNoise, gx, 0.0f);
            int surfaceY = (int)roundf(base * 32.0f + detail * 8.0f);
            if (w == 1) surfaceY += (w * 2);

            for (int y = 0; y < CHUNK_WIDTH; y++) {
                int i = x + y * CHUNK_WIDTH;
                int gy = chunk->position.y * CHUNK_WIDTH + y;

                BlockInstance newInst = { 0,0 };

                if (gy == surfaceY) {
                    newInst.id = BLOCK_GRASS_BLOCK;
                }
                else if (gy > surfaceY && gy <= surfaceY + 4) {
                    newInst.id = BLOCK_DIRT;
                }
                else if (gy > surfaceY + 4) {
                    newInst.id = BLOCK_STONE;
                }

                if (w == 0) chunk->blocks[i] = newInst;
                else chunk->walls[i] = newInst;
                chunk->light[i] = 0;
            }
        }
    }
}

void chunk_decorate(Chunk* chunk) {
    for (int x = 0; x < CHUNK_WIDTH; x++) {
        int gx = chunk->position.x * CHUNK_WIDTH + x;

        DownProjectionResult result = chunk_get_block_projected_downwards(chunk, (Vector2u) { x, 0 }, false);
        if (result.replaced.block && result.down.block) {
            BlockRegistry* brg = br_get_block_registry(result.down.block->id);
            if (brg->flags & BLOCK_FLAG_PLANTABLE) {
                *result.replaced.block = (BlockInstance){ BLOCK_GRASS, 0 };
            }
        }
    }
}

void chunk_genmesh(Chunk* chunk) {
    if (chunk == NULL) return;

    int blockVertexCount = 0;
    int wallVertexCount = 0;

    for (int i = 0; i < CHUNK_AREA; i++) {
        BlockRegistry* rg = br_get_block_registry(chunk->blocks[i].id);
        BlockVariant bvar = br_get_block_variant(chunk->blocks[i].id, chunk->blocks[i].state);
        chunk->blockOffsets[i] = blockVertexCount;
        blockVertexCount += block_models_get_vertex_count(bvar.model_idx);

        rg = br_get_block_registry(chunk->walls[i].id);
        chunk->wallOffsets[i] = wallVertexCount;
        wallVertexCount += block_models_get_vertex_count(bvar.model_idx);
    }

    reset_meshes(chunk, blockVertexCount, wallVertexCount);

    for (int i = 0; i < CHUNK_AREA; i++) {
        int x = i % CHUNK_WIDTH;
        int y = i / CHUNK_WIDTH;

        BlockRegistry* brg = br_get_block_registry(chunk->walls[i].id);

        // Blocks that emit light will not be darkened when its placed as a wall.
        build_quad(chunk, chunk->wallOffsets, chunk->walls, &chunk->wallMesh, true, x, y, brg->lightLevel <= 0 ? wallBrightness : 255);
        build_quad(chunk, chunk->blockOffsets, chunk->blocks, &chunk->blockMesh, false, x, y, 255);
    }

    UploadMesh(&chunk->blockMesh, false);
    UploadMesh(&chunk->wallMesh, false);
}

void chunk_draw(Chunk* chunk) {
    if (!chunk) return;

    rlPushMatrix();

    rlTranslatef(
        chunk->position.x * CHUNK_WIDTH * TILE_SIZE,
        chunk->position.y * CHUNK_WIDTH * TILE_SIZE,
        0.0f
    );

    if (chunk->initializedMeshes) {
        DrawMesh(chunk->wallMesh, texture_atlas_get_material(), MatrixIdentity());
        DrawMesh(chunk->blockMesh, texture_atlas_get_material(), MatrixIdentity());
    }

    for (int i = 0; i < CHUNK_AREA; i++) {
		BlockRegistry* brg = br_get_block_registry(chunk->blocks[i].id);
        if (!brg) continue;
		float lightFactor = chunk->light[i] / 15.0f;

        if (brg->flags & BLOCK_FLAG_LIQUID_SOURCE) {
            int x = i % CHUNK_WIDTH;
            int y = i / CHUNK_WIDTH;
            DrawRectangle(
                x * TILE_SIZE,
                y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE,
                (Color){ 0, 0, 255 * lightFactor, 100 }
			);
		} else if (brg->flags & BLOCK_FLAG_LIQUID_FLOWING) {
            int x = i % CHUNK_WIDTH;
            int y = i / CHUNK_WIDTH;

			FlowingLiquidState* state = (FlowingLiquidState*)&chunk->blocks[i].state;
			float value = state->level / 7.0f;

            DrawRectangle(
                x * TILE_SIZE,
                ceilf(y * TILE_SIZE + (TILE_SIZE * (1.0f - value))),
                TILE_SIZE,
                TILE_SIZE * value,
                (Color){ state->falling ? 0 : 255 * lightFactor, state->falling ? 255 * lightFactor : 0, 0, 100 }
             );
        }
    }

    rlPopMatrix();
}

// Verifica se pode fluir para baixo
bool can_flow_down(BlockExtraResult target, BlockRegistry* targetRegistry, int currentLevel) {
    // Pode fluir para ar
    if (target.block->id == BLOCK_AIR) {
        return true;
    }

    // Pode fluir através de blocos substituíveis (grama, etc.)
    if ((targetRegistry->flags & BLOCK_FLAG_REPLACEABLE) &&
        !(targetRegistry->flags & (BLOCK_FLAG_LIQUID_SOURCE | BLOCK_FLAG_LIQUID_FLOWING))) {
        return true;
    }

    // Pode melhorar líquido flowing existente
    if (targetRegistry->flags & BLOCK_FLAG_LIQUID_FLOWING) {
        FlowingLiquidState* existingState = (FlowingLiquidState*)&target.block->state;

        // Pode fluir se o nível existente é menor que 7 (pode melhorar)
        if (existingState->level < 7) {
            return true;
        }
    }

    // NÃO pode fluir para sources
    return false;
}

static inline bool is_replaceable(BlockRegistry* r) {
    return r && (r->flags & BLOCK_FLAG_REPLACEABLE);
}
static inline bool is_source(BlockRegistry* r) {
    return r && (r->flags & BLOCK_FLAG_LIQUID_SOURCE);
}
static inline bool is_flowing(BlockRegistry* r) {
    return r && (r->flags & BLOCK_FLAG_LIQUID_FLOWING);
}

void chunk_tick(Chunk* chunk) {
    if (!chunk) return;

    bool changed = false;
    int count = chunk->liquidSpreadList.count;

    for (int li = count - 1; li >= 0; li--) {
        uint8_t idx = chunk->liquidSpreadList.indices[li];
        int x = idx % CHUNK_WIDTH;
        int y = idx / CHUNK_WIDTH;

        BlockInstance* binst = chunk_get_block_ptr(chunk, (Vector2u) { x, y }, false);
        if (!binst) {
            liquid_spread_list_remove(&chunk->liquidSpreadList, li);
            continue;
        }

        BlockRegistry* br = br_get_block_registry(binst->id);
        if (!br || !(br->flags & (BLOCK_FLAG_LIQUID_SOURCE | BLOCK_FLAG_LIQUID_FLOWING))) {
            liquid_spread_list_remove(&chunk->liquidSpreadList, li);
            continue;
        }

        BlockExtraResult neighbors[4];
        chunk_get_block_neighbors_extra(chunk, (Vector2u) { x, y }, false, neighbors);

        BlockRegistry* registries[4];
        for (int j = 0; j < 4; j++) {
            if (neighbors[j].block == NULL) {
                registries[j] = NULL;
                continue;
            }
            registries[j] = br_get_block_registry(neighbors[j].block->id);
        }

        if (br->flags & BLOCK_FLAG_LIQUID_FLOWING) {
            bool left_is_source = registries[NEIGHBOR_LEFT] && is_source(registries[NEIGHBOR_LEFT]);
            bool right_is_source = registries[NEIGHBOR_RIGHT] && is_source(registries[NEIGHBOR_RIGHT]);
            if (left_is_source && right_is_source) {
                binst->id = BLOCK_WATER_SOURCE;
                binst->state = 0;
                br = br_get_block_registry(binst->id);
                changed = true;

                bool bottom_was_replaceable = is_replaceable(registries[NEIGHBOR_BOTTOM]);
                bool placed_bottom = false;
                bool bottom_is_source = registries[NEIGHBOR_BOTTOM] && is_source(registries[NEIGHBOR_BOTTOM]);

                if (bottom_was_replaceable &&
                    !(registries[NEIGHBOR_BOTTOM] && (registries[NEIGHBOR_BOTTOM]->flags & BLOCK_FLAG_LIQUID_SOURCE)))
                {
                    if (!(registries[NEIGHBOR_BOTTOM] && is_flowing(registries[NEIGHBOR_BOTTOM]) &&
                        ((FlowingLiquidState*)&neighbors[NEIGHBOR_BOTTOM].block->state)->falling == true))
                    {
                        *neighbors[NEIGHBOR_BOTTOM].block =
                            (BlockInstance){ .id = BLOCK_WATER_FLOWING, .state = get_flowing_liquid_state(7, true) };
                        liquid_spread_list_add(&neighbors[NEIGHBOR_BOTTOM].chunk->liquidSpreadList, neighbors[NEIGHBOR_BOTTOM].idx);
                        changed = true;
                        placed_bottom = true;
                    }
                }

                if (!bottom_was_replaceable || placed_bottom || bottom_is_source) {
                    NeighborDirection dirs[] = { NEIGHBOR_LEFT, NEIGHBOR_RIGHT };
                    for (int d = 0; d < 2; d++) {
                        NeighborDirection dir = dirs[d];
                        BlockRegistry* r = registries[dir];
                        if (!r) continue;
                        if (r->flags & BLOCK_FLAG_LIQUID_SOURCE) continue;

                        bool neighbor_is_flowing = (r->flags & BLOCK_FLAG_LIQUID_FLOWING) != 0;
                        uint8_t newLevel = 6;

                        if ((r->flags & BLOCK_FLAG_REPLACEABLE) || neighbor_is_flowing) {
                            if (neighbor_is_flowing) {
                                FlowingLiquidState* ns = (FlowingLiquidState*)&neighbors[dir].block->state;
                                if (ns->level >= newLevel) continue;
                            }
                            *neighbors[dir].block = (BlockInstance){ .id = BLOCK_WATER_FLOWING, .state = get_flowing_liquid_state(newLevel, false) };
                            liquid_spread_list_add(&neighbors[dir].chunk->liquidSpreadList, neighbors[dir].idx);
                            changed = true;
                        }
                    }
                }

                continue;
            }
        }

        if (br->flags & BLOCK_FLAG_LIQUID_SOURCE) {
            bool bottom_was_replaceable = is_replaceable(registries[NEIGHBOR_BOTTOM]);
            bool placed_bottom = false;
            bool bottom_is_source = registries[NEIGHBOR_BOTTOM] && is_source(registries[NEIGHBOR_BOTTOM]); // NOVO: detecta source embaixo

            if (bottom_was_replaceable &&
                !(registries[NEIGHBOR_BOTTOM] && (registries[NEIGHBOR_BOTTOM]->flags & BLOCK_FLAG_LIQUID_SOURCE)))
            {
                if (!(registries[NEIGHBOR_BOTTOM] && is_flowing(registries[NEIGHBOR_BOTTOM]) &&
                    ((FlowingLiquidState*)&neighbors[NEIGHBOR_BOTTOM].block->state)->falling == true))
                {
                    *neighbors[NEIGHBOR_BOTTOM].block =
                        (BlockInstance){ .id = BLOCK_WATER_FLOWING, .state = get_flowing_liquid_state(7, true) };
                    liquid_spread_list_add(&neighbors[NEIGHBOR_BOTTOM].chunk->liquidSpreadList, neighbors[NEIGHBOR_BOTTOM].idx);
                    changed = true;
                    placed_bottom = true;
                }
            }

            if (!bottom_was_replaceable || placed_bottom || bottom_is_source) {
                NeighborDirection dirs[] = { NEIGHBOR_LEFT, NEIGHBOR_RIGHT };
                for (int d = 0; d < 2; d++) {
                    NeighborDirection dir = dirs[d];
                    BlockRegistry* r = registries[dir];
                    if (!r) continue;

                    if (r->flags & BLOCK_FLAG_LIQUID_SOURCE) continue;

                    bool neighbor_is_flowing = (r->flags & BLOCK_FLAG_LIQUID_FLOWING) != 0;
                    uint8_t newLevel = 6;

                    if ((r->flags & BLOCK_FLAG_REPLACEABLE) || neighbor_is_flowing) {
                        if (neighbor_is_flowing) {
                            FlowingLiquidState* ns = (FlowingLiquidState*)&neighbors[dir].block->state;
                            if (ns->level >= newLevel) continue;
                        }
                        *neighbors[dir].block = (BlockInstance){ .id = BLOCK_WATER_FLOWING, .state = get_flowing_liquid_state(newLevel, false) };
                        liquid_spread_list_add(&neighbors[dir].chunk->liquidSpreadList, neighbors[dir].idx);
                        changed = true;
                    }
                }
            }
        }

        else if (br->flags & BLOCK_FLAG_LIQUID_FLOWING) {
            FlowingLiquidState* fs = (FlowingLiquidState*)&binst->state;

            if (is_replaceable(registries[NEIGHBOR_BOTTOM]) &&
                !(registries[NEIGHBOR_BOTTOM] && (registries[NEIGHBOR_BOTTOM]->flags & BLOCK_FLAG_LIQUID_SOURCE)))
            {
                if (!(is_flowing(registries[NEIGHBOR_BOTTOM]) &&
                    ((FlowingLiquidState*)&neighbors[NEIGHBOR_BOTTOM].block->state)->falling == true))
                {
                    *neighbors[NEIGHBOR_BOTTOM].block =
                        (BlockInstance){ .id = BLOCK_WATER_FLOWING, .state = get_flowing_liquid_state(7, true) };
                    liquid_spread_list_add(&neighbors[NEIGHBOR_BOTTOM].chunk->liquidSpreadList, neighbors[NEIGHBOR_BOTTOM].idx);
                    changed = true;
                    continue;
                }
            }

            if (fs->level > 0 && !is_replaceable(registries[NEIGHBOR_BOTTOM])) {
                NeighborDirection dirs[] = { NEIGHBOR_LEFT, NEIGHBOR_RIGHT };
                for (int d = 0; d < 2; d++) {
                    NeighborDirection dir = dirs[d];
                    BlockRegistry* r = registries[dir];
                    if (!r) continue;
                    if (r->flags & BLOCK_FLAG_LIQUID_SOURCE) continue;

                    uint8_t targetLevel = fs->level - 1;
                    bool neighbor_is_flowing = (r->flags & BLOCK_FLAG_LIQUID_FLOWING) != 0;

                    if ((r->flags & BLOCK_FLAG_REPLACEABLE) || neighbor_is_flowing) {
                        if (neighbor_is_flowing) {
                            FlowingLiquidState* ns = (FlowingLiquidState*)&neighbors[dir].block->state;
                            if (ns->level >= targetLevel) continue;
                        }
                        *neighbors[dir].block = (BlockInstance){ .id = BLOCK_WATER_FLOWING, .state = get_flowing_liquid_state(targetLevel, false) };
                        liquid_spread_list_add(&neighbors[dir].chunk->liquidSpreadList, neighbors[dir].idx);
                        changed = true;
                    }
                }
            }
        }
    }

    if (changed) chunk_manager_update_lighting();
}


void chunk_free(Chunk* chunk)
{
    if (!chunk) return;
    if (chunk->initializedMeshes) {
        UnloadMesh(chunk->wallMesh);
        UnloadMesh(chunk->blockMesh);
        chunk->initializedMeshes = false;
    }

    container_vector_free(&chunk->containerVec);
	liquid_spread_list_free(&chunk->liquidSpreadList);
}

void chunk_fill_light(Chunk* chunk, Vector2u startPoint, uint8_t newLightValue) {
    if (!chunk) return;
    if (newLightValue < 1 || newLightValue > 15) return;
    if (startPoint.x < 0 || startPoint.x >= CHUNK_WIDTH) return;
    if (startPoint.y < 0 || startPoint.y >= CHUNK_WIDTH) return;

    uint8_t current = chunk_get_light(chunk, startPoint);
    if (current >= newLightValue) return;

    chunk_set_light(chunk, startPoint, newLightValue);

    uint8_t decayAmount = 4;

    BlockInstance binst = chunk_get_block(chunk, startPoint, false);
    BlockRegistry* br = br_get_block_registry(binst.id);
    BlockVariant bvar = br_get_block_variant(binst.id, binst.state);

    if (br->lightLevel == BLOCK_LIGHT_TRANSPARENT || !(br->flags & BLOCK_FLAG_FULL_BLOCK)) {
        decayAmount = 1;
    }

    Vector2i neighbors[] = {
        { -1, 0 }, { 1, 0 }, { 0, 1 }, { 0, -1 }
    };

    for (int i = 0; i < 4; i++) {
        Chunk* nextChunk = chunk;

        Vector2i neighPos = {
            .x = startPoint.x + neighbors[i].x,
            .y = startPoint.y + neighbors[i].y
        };

        if (neighPos.x < 0) {
            nextChunk = chunk->neighbors.left;
            neighPos.x = posmod(neighPos.x, CHUNK_WIDTH);
        }
        else if (neighPos.x >= CHUNK_WIDTH) {
            nextChunk = chunk->neighbors.right;
            neighPos.x = posmod(neighPos.x, CHUNK_WIDTH);
        }
        if (neighPos.y < 0) {
            nextChunk = chunk->neighbors.up;
            neighPos.y = posmod(neighPos.y, CHUNK_WIDTH);
        }
        else if (neighPos.y >= CHUNK_WIDTH) {
            nextChunk = chunk->neighbors.down;
            neighPos.y = posmod(neighPos.y, CHUNK_WIDTH);
        }

        if (nextChunk) chunk_fill_light(nextChunk, (Vector2u) { neighPos.x, neighPos.y }, newLightValue - decayAmount);
    }
}

DownProjectionResult chunk_get_block_projected_downwards(Chunk* chunk, Vector2u startPoint, bool isWall) {
    DownProjectionResult empty = { { NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX }, { NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX } };
    if (!chunk) return empty;

    for (int y = startPoint.y; y < CHUNK_WIDTH; y++) {
        if (y == startPoint.y) {
            BlockRegistry* br = br_get_block_registry(chunk_get_block(chunk, startPoint, isWall).id);
            if (!(br->flags & BLOCK_FLAG_REPLACEABLE)) return empty;
        }

        BlockExtraResult down = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { startPoint.x, y + 1 }, isWall);
        if (down.block == NULL) return empty;

        BlockRegistry* br = br_get_block_registry(down.block->id);
        if (br == NULL) return empty;

        if (!(br->flags & BLOCK_FLAG_REPLACEABLE)) {
            uint8_t idx = startPoint.x + (y * CHUNK_WIDTH);
            return (DownProjectionResult) {
                .replaced = (BlockExtraResult){
                    .block = &chunk->blocks[idx],
                    .chunk = chunk,
                    .position = (Vector2u){ startPoint.x, y },
                    .idx = idx
                },
                .down = down
            };
        }
        else { continue; }
    }
    
    if (chunk->neighbors.down) {
        return chunk_get_block_projected_downwards(chunk->neighbors.down, (Vector2u) { startPoint.x, 0 }, isWall);
    }
    else { return empty; }
}

BlockInstance* chunk_get_block_ptr(Chunk* chunk, Vector2u position, bool isWall) {
    if (!chunk) return NULL;
    if (position.x < 0 || position.x >= CHUNK_WIDTH || position.y < 0 || position.y >= CHUNK_WIDTH) return NULL;
    if (!isWall)
        return &chunk->blocks[position.x + (position.y * CHUNK_WIDTH)];
    else
        return &chunk->walls[position.x + (position.y * CHUNK_WIDTH)];
}

BlockExtraResult chunk_get_block_extrapolating_ptr(Chunk* chunk, Vector2i position, bool isWall) {
    if (!chunk) return (BlockExtraResult){ NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX };

    if (position.x >= 0 && position.y >= 0 && position.x < CHUNK_WIDTH && position.y < CHUNK_WIDTH) {
		return (BlockExtraResult) {
            .block = chunk_get_block_ptr(chunk, (Vector2u) { position.x, position.y }, isWall),
            .chunk = chunk,
            .position = (Vector2u) { (unsigned int)position.x, (unsigned int)position.y },
			.idx = (uint8_t)(position.x + (position.y * CHUNK_WIDTH))
        };
    }
    else {
        Chunk* neighbor = NULL;

        if (position.x < 0 && position.y < 0) neighbor = (Chunk*)chunk->neighbors.upLeft;
        else if (position.x >= CHUNK_WIDTH && position.y < 0) neighbor = (Chunk*)chunk->neighbors.upRight;
        else if (position.x < 0 && position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.downLeft;
        else if (position.x >= CHUNK_WIDTH && position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.downRight;
        else if (position.x < 0) neighbor = (Chunk*)chunk->neighbors.left;
        else if (position.x >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.right;
        else if (position.y < 0) neighbor = (Chunk*)chunk->neighbors.up;
        else if (position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.down;

        if (neighbor == NULL) return (BlockExtraResult) { NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX };

        Vector2u relPos = {
            .x = posmod(position.x, CHUNK_WIDTH),
            .y = posmod(position.y, CHUNK_WIDTH)
        };

		return (BlockExtraResult) {
            .block = chunk_get_block_ptr(neighbor, relPos, isWall),
            .chunk = neighbor,
            .position = relPos,
			.idx = (uint8_t)(relPos.x + (relPos.y * CHUNK_WIDTH))
        };
    }
}

uint8_t* chunk_get_light_ptr(Chunk* chunk, Vector2u position) {
    if (!chunk) return NULL;
    if (position.x < 0 || position.x >= CHUNK_WIDTH || position.y < 0 || position.y >= CHUNK_WIDTH) return NULL;
    return &chunk->light[position.x + (position.y * CHUNK_WIDTH)];
}

LightExtraResult chunk_get_light_extrapolating_ptr(Chunk* chunk, Vector2i position) {
    if (!chunk) return (LightExtraResult){ NULL, NULL, { UINT8_MAX, UINT8_MAX } };

    if (position.x >= 0 && position.y >= 0 && position.x < CHUNK_WIDTH && position.y < CHUNK_WIDTH) {
        return (LightExtraResult) {
            .light = chunk_get_light_ptr(chunk, (Vector2u) { position.x, position.y }),
            .chunk = chunk,
            .position = (Vector2u) { (unsigned int)position.x, (unsigned int)position.y }
        };
    }
    else {
        Chunk* neighbor = NULL;

        if (position.x < 0 && position.y < 0) neighbor = (Chunk*)chunk->neighbors.upLeft;
        else if (position.x >= CHUNK_WIDTH && position.y < 0) neighbor = (Chunk*)chunk->neighbors.upRight;
        else if (position.x < 0 && position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.downLeft;
        else if (position.x >= CHUNK_WIDTH && position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.downRight;
        else if (position.x < 0) neighbor = (Chunk*)chunk->neighbors.left;
        else if (position.x >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.right;
        else if (position.y < 0) neighbor = (Chunk*)chunk->neighbors.up;
        else if (position.y >= CHUNK_WIDTH) neighbor = (Chunk*)chunk->neighbors.down;

        if (neighbor == NULL) return (LightExtraResult) { NULL, NULL, { UINT8_MAX, UINT8_MAX } };

        Vector2u relPos = {
            .x = posmod(position.x, CHUNK_WIDTH),
            .y = posmod(position.y, CHUNK_WIDTH)
        };

        return (LightExtraResult) {
            .light = chunk_get_light_ptr(neighbor, relPos),
            .chunk = neighbor,
            .position = relPos
        };
    }
}

void chunk_set_block(Chunk* chunk, Vector2u position, BlockInstance blockValue, bool isWall) {
    BlockInstance* ptr = chunk_get_block_ptr(chunk, position, isWall);
    if (!ptr) return;
    if (ptr->id == blockValue.id && ptr->state == blockValue.state) return;

	BlockRegistry* brg = br_get_block_registry(blockValue.id);
    if (brg->flags & BLOCK_FLAG_GRAVITY_AFFECTED) {
		DownProjectionResult where = chunk_get_block_projected_downwards(chunk, position, isWall);
        if (where.replaced.block && where.replaced.chunk) {
            *where.replaced.block = blockValue;
            ptr = where.replaced.block;
            chunk = where.replaced.chunk;
            position = where.replaced.position;
		}
    }
    else {
        *ptr = blockValue;
    }

    BlockExtraResult neighbors[4];
	chunk_get_block_neighbors_extra(chunk, position, isWall, neighbors);
    for (int i = 0; i < 4; i++) {
        if (neighbors[i].block == NULL) continue;
		BlockRegistry* brg = br_get_block_registry(neighbors[i].block->id);
        if (brg->state_resolver) {
            BlockInstance neighbors2[4];
            chunk_get_block_neighbors(neighbors[i].chunk, neighbors[i].position, isWall, neighbors2);

            bool result = brg->state_resolver(
                neighbors[i].block,
                neighbors[i].chunk,
                neighbors[i].position.x + (neighbors[i].position.y * CHUNK_WIDTH),
                neighbors2,
                isWall
            );

            if (!result) *neighbors[i].block = (BlockInstance){ 0, 0 };
        }
    }

    chunk_manager_update_lighting();
}

BlockExtraResult chunk_set_block_extrapolating(Chunk* chunk, Vector2i position, BlockInstance blockValue, bool isWall) {
    BlockExtraResult result = chunk_get_block_extrapolating_ptr(chunk, position, isWall);
    if (!result.block || !result.chunk) return result;
    chunk_set_block(result.chunk, result.position, blockValue, isWall);
    return result;
}

BlockInstance chunk_get_block(Chunk* chunk, Vector2u position, bool isWall) {
    BlockInstance* ptr = chunk_get_block_ptr(chunk, position, isWall);
    if (!ptr) return (BlockInstance){ 0, 0 };
    return *ptr;
}

BlockInstance chunk_get_block_extrapolating(Chunk* chunk, Vector2i position, bool isWall) {
    BlockExtraResult result = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x, position.y }, isWall);
    if (!result.block || !result.chunk) return (BlockInstance){ 0, 0 };
    return chunk_get_block(result.chunk, result.position, isWall);
}

void chunk_set_light(Chunk* chunk, Vector2u position, uint8_t value) {
    uint8_t* ptr = chunk_get_light_ptr(chunk, position);
    if (!ptr) return;
    *ptr = value;
}

void chunk_set_light_extrapolating(Chunk* chunk, Vector2i position, uint8_t value) {
    LightExtraResult result = chunk_get_light_extrapolating_ptr(chunk, (Vector2i) { position.x, position.y });
    if (!result.light || !result.chunk) return;
    chunk_set_light(result.chunk, result.position, value);
}

uint8_t chunk_get_light(Chunk* chunk, Vector2u position) {
    uint8_t* ptr = chunk_get_light_ptr(chunk, position);
    if (!ptr) return 0;
    return *ptr;
}

uint8_t chunk_get_light_extrapolating(Chunk* chunk, Vector2i position) {
    LightExtraResult result = chunk_get_light_extrapolating_ptr(chunk, (Vector2i) { position.x, position.y });
    if (!result.light || !result.chunk) return 0;
    return chunk_get_light(result.chunk, result.position);
}

void chunk_get_block_neighbors(Chunk* chunk, Vector2u position, bool isWall, BlockInstance output[4]) {
    output[NEIGHBOR_TOP] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y - 1 }, isWall);
    output[NEIGHBOR_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y }, isWall);
    output[NEIGHBOR_BOTTOM] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y + 1 }, isWall);
    output[NEIGHBOR_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y }, isWall);
}

void chunk_get_block_neighbors_extra(Chunk* chunk, Vector2u position, bool isWall, BlockExtraResult output[4]) {
    output[NEIGHBOR_TOP] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x, position.y - 1 }, isWall);
    output[NEIGHBOR_RIGHT] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x + 1, position.y }, isWall);
    output[NEIGHBOR_BOTTOM] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x, position.y + 1 }, isWall);
    output[NEIGHBOR_LEFT] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x - 1, position.y }, isWall);
}

void chunk_get_block_neighbors_with_corners(Chunk* chunk, Vector2u position, bool isWall, BlockInstance output[8]) {
    output[NEIGHBOR_TOP] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y - 1 }, isWall);
    output[NEIGHBOR_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y }, isWall);
    output[NEIGHBOR_BOTTOM] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y + 1 }, isWall);
    output[NEIGHBOR_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y }, isWall);

    output[NEIGHBOR_TOP_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y - 1 }, isWall);
    output[NEIGHBOR_TOP_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y - 1 }, isWall);
    output[NEIGHBOR_BOTTOM_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y + 1 }, isWall);
    output[NEIGHBOR_BOTTOM_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y + 1 }, isWall);
}

void chunk_get_light_neighbors(Chunk* chunk, Vector2u position, uint8_t output[4]) {
    output[NEIGHBOR_TOP] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x, position.y - 1 });
    output[NEIGHBOR_RIGHT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x + 1, position.y });
    output[NEIGHBOR_BOTTOM] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x, position.y + 1 });
    output[NEIGHBOR_LEFT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x - 1, position.y });
}

void chunk_get_light_neighbors_with_corners(Chunk* chunk, Vector2u position, uint8_t output[8]) {
    output[NEIGHBOR_TOP] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x, position.y - 1 });
    output[NEIGHBOR_RIGHT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x + 1, position.y });
    output[NEIGHBOR_BOTTOM] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x, position.y + 1 });
    output[NEIGHBOR_LEFT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x - 1, position.y });

    output[NEIGHBOR_TOP_LEFT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x - 1, position.y - 1 });
    output[NEIGHBOR_TOP_RIGHT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x + 1, position.y - 1 });
    output[NEIGHBOR_BOTTOM_RIGHT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x + 1, position.y + 1 });
    output[NEIGHBOR_BOTTOM_LEFT] = chunk_get_light_extrapolating(chunk, (Vector2i) { position.x - 1, position.y + 1 });
}

unsigned int posmod(int v, int m) {
    int r = v % m;
    return (unsigned int)(r < 0 ? r + m : r);
}

void reset_meshes(Chunk* chunk, int blockVertexCount, int wallVertexCount) {
    if (!chunk) return;

    if (chunk->initializedMeshes == true) {
        UnloadMesh(chunk->blockMesh);
        UnloadMesh(chunk->wallMesh);
        chunk->initializedMeshes = false;
    }

    chunk->wallMesh = (Mesh){ 0 };
    chunk->blockMesh = (Mesh){ 0 };

    chunk->blockMesh.vertexCount = blockVertexCount;
    chunk->wallMesh.vertexCount = wallVertexCount;

    chunk->blockMesh.triangleCount = blockVertexCount * 3;
    chunk->wallMesh.triangleCount = wallVertexCount * 3;

    chunk->blockMesh.vertices = (float*)MemAlloc(blockVertexCount * 3 * sizeof(float));
    chunk->wallMesh.vertices = (float*)MemAlloc(wallVertexCount * 3 * sizeof(float));

    chunk->blockMesh.texcoords = (float*)MemAlloc(blockVertexCount * 2 * sizeof(float));
    chunk->wallMesh.texcoords = (float*)MemAlloc(wallVertexCount * 2 * sizeof(float));

    chunk->blockMesh.colors = (unsigned char*)MemAlloc(blockVertexCount * 4 * sizeof(unsigned char));
    chunk->wallMesh.colors = (unsigned char*)MemAlloc(wallVertexCount * 4 * sizeof(unsigned char));

    chunk->initializedMeshes = true;
}

void build_quad(Chunk* chunk, size_t* offsets, BlockInstance* blocks, Mesh* mesh, bool isWall, uint8_t x, uint8_t y, uint8_t brightness) {
    int i = x + (y * CHUNK_WIDTH);
	BlockRegistry* brg = br_get_block_registry(blocks[i].id);
    if (blocks[i].id <= 0 || brg->flags & BLOCK_FLAG_LIQUID_SOURCE || brg->flags & BLOCK_FLAG_LIQUID_FLOWING) return;
    
    // Start by brightness value
    uint8_t cornerValues[4] = { brightness, brightness, brightness, brightness };

    // Flipping the block texture when requested
    unsigned int h = (unsigned int)(chunk->position.x * 73856093 ^ chunk->position.y * 19349663);
    h ^= x * 374761393u;
    h ^= y * 668265263u;
    h ^= (unsigned int)isWall * 1442695040888963407ull;
    h = (h ^ (h >> 13)) * 1274126177u;

    bool flipUVH = (brg->flags & BLOCK_FLAG_FLIP_H) && (h & 1) ? true : false;
    bool flipUVV = (brg->flags & BLOCK_FLAG_FLIP_V) && (h & 2) ? true : false;

    bool flipTriangles = false;

    // Calculating brightness based on light values
    if (!smoothLighting) {
        uint8_t lightValue = (uint8_t)((chunk->light[i] / 15.0f) * 255.0f);
        uint8_t reduction = 255 - lightValue;

        for (int i = 0; i < 4; i++) {
            if (cornerValues[i] > reduction) cornerValues[i] -= reduction;
            else cornerValues[i] = 0;
        }
    } else {
        // Do not apply smooth lighting to blocks that emits light
        if (brg->lightLevel > 0) {
            uint8_t lightValue = (uint8_t)((chunk->light[i] / 15.0f) * 255.0f);
            uint8_t reduction = 255 - lightValue;

            for (int i = 0; i < 4; i++) {
                if (cornerValues[i] > reduction) cornerValues[i] -= reduction;
                else cornerValues[i] = 0;
            }
        } else {
            uint8_t neighbors[8];
            chunk_get_light_neighbors_with_corners(chunk, (Vector2u) { x, y }, neighbors);
    
            // 0 = Top Left
            // 1 = Top Right
            // 2 = Bottom Right
            // 3 = Bottom Left
    
            int cornerNeighbors[4][3] = {
                {NEIGHBOR_LEFT, NEIGHBOR_TOP_LEFT, NEIGHBOR_TOP},           // Top Left
                {NEIGHBOR_RIGHT, NEIGHBOR_TOP_RIGHT, NEIGHBOR_TOP},         // Top Right
                {NEIGHBOR_RIGHT, NEIGHBOR_BOTTOM_RIGHT, NEIGHBOR_BOTTOM},   // Bottom Right
                {NEIGHBOR_LEFT, NEIGHBOR_BOTTOM_LEFT, NEIGHBOR_BOTTOM}      // Bottom Left
            };
    
            for (int corner = 0; corner < 4; corner++) {
                float lightSum = (float)chunk->light[i];
                for (int n = 0; n < 3; n++) {
                    lightSum += (float)neighbors[cornerNeighbors[corner][n]];
                }
                float average = lightSum / 4.0f;
    
                uint8_t lightValue = (uint8_t)((average / 15.0f) * 255.0f);
    
                uint8_t reduction = 255 - lightValue;
                if (cornerValues[corner] > reduction) cornerValues[corner] -= reduction;
                else cornerValues[corner] = 0;
            }
        }
    }

    // Wall "ambient occulsion" for walls only
    if (wallAmbientOcclusion && isWall && brg->lightLevel <= 0) {
        BlockInstance neighbors[8];
        chunk_get_block_neighbors_with_corners(chunk, (Vector2u) { x, y }, false, neighbors);

        BlockRegistry* registries[8];
        for (int i = 0; i < 8; i++) registries[i] = br_get_block_registry(neighbors[i].id);

        struct {
            int corners[2];
            bool flipTri;
        } aoRules[8] = {
            {{0, 1}, false},    // Top
            {{1, 2}, false},    // Right
            {{2, 3}, false},    // Bottom
            {{0, 3}, false},    // Left

            {{0, -1}, true},    // Top Left
            {{1, -1}, false},   // Top Right
            {{2, -1}, true},    // Bottom Right
            {{3, -1}, false}    // Bottom Left
        };

        for (int dir = 0; dir < 8; dir++) {
            if (registries[dir] == NULL) continue;
            if ((!(registries[dir]->lightLevel == BLOCK_LIGHT_TRANSPARENT) && (registries[dir]->lightLevel <= 0))) {
                for (int c = 0; c < 2; c++) {
                    int corner = aoRules[dir].corners[c];
                    if (corner >= 0) {
                        cornerValues[corner] = fminf(cornerValues[corner], wallAOvalue);
                    }
                }

                flipTriangles = aoRules[dir].flipTri;
            }
        }
    }

    Color colors[4];
    for (int i = 0; i < 4; i++) {
        colors[i] = (Color){
            .r = cornerValues[i],
            .g = cornerValues[i],
            .b = cornerValues[i],
            .a = 255
        };
    }

    // Block state rendering
    BlockVariant bvar = br_get_block_variant(blocks[i].id, blocks[i].state);

    bm_set_block_model(offsets, mesh, (Vector2u) { x, y }, colors, bvar.model_idx, bvar.atlas_idx, flipUVH, flipUVV, bvar.flipH, bvar.flipV, bvar.rotation);
}