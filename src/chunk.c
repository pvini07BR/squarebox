#include "block_tick_list.h"
#include "chunk_manager.h"
#include "container_vector.h"
#include "types.h"
#include "block_registry.h"
#include "chunk.h"
#include "texture_atlas.h"
#include "block_models.h"
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

static unsigned int posmod(int v, int m);
static void reset_meshes(Chunk* chunk, int blockVertexCount, int wallVertexCount);
static void build_quad(Chunk* chunk, size_t* offsets, BlockInstance* blocks, Mesh* mesh, bool isWall, uint8_t x, uint8_t y, uint8_t brightness);

static Material matDefault;
static bool loadedMatDefault = false;

int seed = 0;
bool wallAmbientOcclusion = true;
bool smoothLighting = true;
unsigned int wallBrightness = 128;
unsigned int wallAOvalue = 64;

static bool chance_at(fnl_noise_type noiseType, float frequency, int gx, int gy, float threshold, int seed_offset) {
    fnl_state noise = fnlCreateState();
    noise.seed = seed + seed_offset;
    noise.frequency = frequency;
    noise.noise_type = noiseType;

    return fnlGetNoise2D(&noise, gx, gy) > threshold;
}

void chunk_init(Chunk* chunk)
{
    if (chunk == NULL) return;

    container_vector_init(&chunk->containerVec);
    block_tick_list_clear(&chunk->blockTickList);

    chunk->initializedMeshes = false;

    if (!loadedMatDefault) {
        matDefault = LoadMaterialDefault();
        loadedMatDefault = true;
    }
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

            for (int y = 0; y < CHUNK_WIDTH; y++) {
                int i = x + y * CHUNK_WIDTH;
                int gy = chunk->position.y * CHUNK_WIDTH + y;

                BlockInstance newInst = { 0,0 };

                if (gy == (surfaceY - 1)) {
                    if (chance_at(FNL_NOISE_VALUE, 32.0f, gx, gy, 0.0f, w)) {
                        newInst.id = BLOCK_GRASS;
                    }
                    else if (chance_at(FNL_NOISE_VALUE, 32.0f, gx, gy, -0.25f, w)) {
                        newInst.id = BLOCK_FLOWER;
                    }
                    else if (chance_at(FNL_NOISE_VALUE, 32.0f, gx, gy, -0.4f, w)) {
                        newInst.id = BLOCK_PEBBLES;
                    }
                }
                else if (gy == surfaceY) {
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

void chunk_gen_liquid_mesh(Chunk* chunk) {
    if (!chunk) return;

    for (int i = 0; i < CHUNK_AREA; i++) {
        BlockRegistry* rg = br_get_block_registry(chunk->blocks[i].id);
        if (!rg) continue;
        if (!(rg->flags & BLOCK_FLAG_LIQUID)) continue;

        int x = i % CHUNK_WIDTH;
        int y = i / CHUNK_WIDTH;

        FlowingLiquidState* state = (FlowingLiquidState*)&chunk->blocks[i].state;
        float value = 0.125f + (state->level / 7.0f) * (1.0f - 0.125f);
        if (chunk->blocks[i].id == BLOCK_WATER_SOURCE) value = 1.0f;

        BlockExtraResult neighbors[4];
        chunk_get_block_neighbors_extra(chunk, (Vector2u) { x, y }, false, neighbors);

        float left_value = value;
        float right_value = value;

        NeighborDirection dirs[] = { NEIGHBOR_LEFT, NEIGHBOR_RIGHT };
        for (int n = 0; n < 2; n++) {
            float* value = n == 0 ? &left_value : &right_value;

            BlockExtraResult neigh = neighbors[dirs[n]];
            BlockRegistry* nrg = neigh.reg;
            if (!nrg) continue;
            if (!(nrg->flags & BLOCK_FLAG_LIQUID)) continue;

            FlowingLiquidState* neighState = &neigh.block->state;
            float val = 0.125f + (neighState->level / 7.0f) * (1.0f - 0.125f);

            if (neigh.block->id == BLOCK_WATER_SOURCE || (neigh.block->id == BLOCK_WATER_FLOWING && neighState->falling)) {
                *value = 1.0f;
            }
            else {
                if (*value < val) *value = val;
            }
        }

        float x0 = x * TILE_SIZE;
        float x1 = x0 + TILE_SIZE;

        float bottom = y * TILE_SIZE + TILE_SIZE;
        float y_top_left = bottom - TILE_SIZE * left_value;
        float y_top_right = bottom - TILE_SIZE * right_value;
        float y_bottom = bottom;

        int base = i * 6 * 3;
        int colorBase = i * 6 * 4;

        chunk->liquidMesh.vertices[base + 0] = x0;
        chunk->liquidMesh.vertices[base + 1] = y_top_left;
        chunk->liquidMesh.vertices[base + 2] = 0.0f;

        chunk->liquidMesh.vertices[base + 3] = x1;
        chunk->liquidMesh.vertices[base + 4] = y_top_right;
        chunk->liquidMesh.vertices[base + 5] = 0.0f;

        chunk->liquidMesh.vertices[base + 6] = x1;
        chunk->liquidMesh.vertices[base + 7] = y_bottom;
        chunk->liquidMesh.vertices[base + 8] = 0.0f;

        chunk->liquidMesh.vertices[base + 9] = x0;
        chunk->liquidMesh.vertices[base + 10] = y_top_left;
        chunk->liquidMesh.vertices[base + 11] = 0.0f;

        chunk->liquidMesh.vertices[base + 12] = x1;
        chunk->liquidMesh.vertices[base + 13] = y_bottom;
        chunk->liquidMesh.vertices[base + 14] = 0.0f;

        chunk->liquidMesh.vertices[base + 15] = x0;
        chunk->liquidMesh.vertices[base + 16] = y_bottom;
        chunk->liquidMesh.vertices[base + 17] = 0.0f;

        float lightFactor = chunk->light[i] / 15.0f;

        for (int v = 0; v < 6; v++) {
            int c = colorBase + v * 4;
            chunk->liquidMesh.colors[c + 0] = 0;
            chunk->liquidMesh.colors[c + 1] = 0;
            chunk->liquidMesh.colors[c + 2] = 255 * lightFactor;
            chunk->liquidMesh.colors[c + 3] = 100;
        }
    }

    UploadMesh(&chunk->liquidMesh, false);
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
        bvar = br_get_block_variant(chunk->walls[i].id, chunk->walls[i].state);
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

    chunk_gen_liquid_mesh(chunk);

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

    rlPopMatrix();
}

void chunk_draw_liquids(Chunk* chunk) {
    if (!chunk) return;

    rlPushMatrix();

    rlTranslatef(
        chunk->position.x * CHUNK_WIDTH * TILE_SIZE,
        chunk->position.y * CHUNK_WIDTH * TILE_SIZE,
        0.0f
    );

    if (chunk->initializedMeshes) {
        rlDrawRenderBatchActive();
        DrawMesh(chunk->liquidMesh, matDefault, MatrixIdentity());
    }

    rlPopMatrix();
}

void chunk_tick(Chunk* chunk, uint8_t tick_value) {
    if (!chunk) return;

    bool changed = false;
    size_t count = chunk->blockTickList.count;

    for (int i = 0; i < count; i++) {
        BlockTickListEntry entry = chunk->blockTickList.entries[i];

        BlockInstance* ptr = chunk_get_block_ptr(chunk, entry.position, entry.isWall);
        if (!ptr) {
            block_tick_list_remove_by_index(&chunk->blockTickList, i);
            continue;
        }

        BlockRegistry* brg = br_get_block_registry(ptr->id);
        if (!brg || brg->tick_callback == NULL) {
            block_tick_list_remove_by_index(&chunk->blockTickList, i);
            continue;
        }

        BlockExtraResult neighbors[4];
        chunk_get_block_neighbors_extra(chunk, entry.position, entry.isWall, neighbors);

        BlockExtraResult result = {
            .block = ptr,
            .reg = brg,
            .chunk = chunk,
            .position = entry.position,
            .idx = entry.position.x + (entry.position.y * CHUNK_WIDTH)
        };

        uint8_t mod = tick_value % brg->tick_speed;
        if (mod == (brg->tick_speed-1)) {
            bool did_change = brg->tick_callback(result, neighbors, entry.isWall);
            if (changed == false) changed = did_change;
        }
    }

    if (changed) {
        chunk_manager_update_lighting();
    }
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

DownProjectionResult chunk_get_block_projected_downwards(Chunk* chunk, Vector2u startPoint, bool isWall, bool goToNeighbor) {
    DownProjectionResult empty = { { NULL, NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX }, { NULL, NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX } };
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
                    .block = isWall ? &chunk->walls[idx] : &chunk->blocks[idx],
                    .chunk = chunk,
                    .position = (Vector2u){ startPoint.x, y },
                    .idx = idx
                },
                .down = down
            };
        }
        else { continue; }
    }
    
    if (goToNeighbor) {
        return chunk_get_block_projected_downwards(chunk->neighbors.down, (Vector2u) { startPoint.x, 0 }, isWall, goToNeighbor);
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
    if (!chunk) return (BlockExtraResult){ NULL, NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX };

    if (position.x >= 0 && position.y >= 0 && position.x < CHUNK_WIDTH && position.y < CHUNK_WIDTH) {
        BlockInstance* inst = chunk_get_block_ptr(chunk, (Vector2u) { position.x, position.y }, isWall);
		return (BlockExtraResult) {
            .block = inst,
            .reg = br_get_block_registry(inst->id),
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

        if (neighbor == NULL) return (BlockExtraResult) { NULL, NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX };

        Vector2u relPos = {
            .x = posmod(position.x, CHUNK_WIDTH),
            .y = posmod(position.y, CHUNK_WIDTH)
        };

        BlockInstance* inst = chunk_get_block_ptr(neighbor, relPos, isWall);
		return (BlockExtraResult) {
            .block = inst,
            .reg = br_get_block_registry(inst->id),
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

void chunk_set_block(Chunk* chunk, Vector2u position, BlockInstance blockValue, bool isWall, bool update_lighting) {
    BlockInstance* ptr = chunk_get_block_ptr(chunk, position, isWall);
    if (!ptr) return;
    if (ptr->id == blockValue.id && ptr->state == blockValue.state) return;

    bool can_place = true;

    BlockRegistry* new_br = br_get_block_registry(blockValue.id);
    if (!new_br) return;

    BlockExtraResult self = {
        .block = &blockValue,
        .reg = new_br,
        .chunk = chunk,
        .position = position,
        .idx = position.x + (position.y * CHUNK_WIDTH)
    };

    BlockInstance* other_inst = chunk_get_block_ptr(chunk, position, !isWall);
    if (!other_inst) return;
    BlockRegistry* other_br = br_get_block_registry(other_inst->id);
    if (!other_br) return;

    BlockExtraResult other = {
        .block = other_inst,
        .reg = other_br,
        .chunk = chunk,
        .position = position,
        .idx = position.x + (position.y * CHUNK_WIDTH)
    };

    // Resolve state for new block before placing
    if (new_br->state_resolver != NULL) {
        BlockExtraResult neighbors[4];
        chunk_get_block_neighbors_extra(chunk, position, isWall, neighbors);

        can_place = new_br->state_resolver(self, other, neighbors, isWall);
    }
    if (!can_place) return;

    // Handle previous block (destroy)
    if (ptr->id > 0) {
        BlockRegistry* old_br = br_get_block_registry(ptr->id);
        if (old_br) {
            if (old_br->tick_callback != NULL) {
                block_tick_list_remove(&chunk->blockTickList, (BlockTickListEntry) { .position = position, .isWall = isWall });
            }

            if (old_br->destroy_callback) {
                BlockExtraResult res = {
                    .block = ptr,
                    .reg = old_br,
                    .chunk = chunk,
                    .position = position,
                    .idx = position.x + (position.y * CHUNK_WIDTH)
                };
                old_br->destroy_callback(res);
            }
        }
    }

    // Set the block
    *ptr = blockValue;

    // Resolve state for neighboring blocks
    BlockExtraResult neighbors[4];
    chunk_get_block_neighbors_extra(chunk, position, isWall, neighbors);
    BlockExtraResult otherNeighbors[4];
    chunk_get_block_neighbors_extra(chunk, position, !isWall, otherNeighbors);

    for (int i = 0; i < 4; i++) {
        if (neighbors[i].block == NULL) continue;
        BlockRegistry* nbr_br = neighbors[i].reg;
        if (!nbr_br || nbr_br->state_resolver == NULL) continue;

        BlockExtraResult neighbors2[4];
        chunk_get_block_neighbors_extra(neighbors[i].chunk, neighbors[i].position, isWall, neighbors2);

        if (nbr_br->state_resolver) {
            bool r = nbr_br->state_resolver(neighbors[i], otherNeighbors[i], neighbors2, isWall);
            if (!r) *neighbors[i].block = (BlockInstance){ 0, 0 };
        }
    }

    // Resolve state for the other block (wall or block)
    if (other_br->state_resolver) {
        bool r = other_br->state_resolver(other, self, otherNeighbors, !isWall);
        if (!r) *other.block = (BlockInstance){ 0, 0 };
    }

    if (update_lighting) chunk_manager_update_lighting();

    // Add tick for new block if applicable
    if (new_br->tick_callback != NULL) {
        if (!block_tick_list_contains(&chunk->blockTickList, (BlockTickListEntry){ .position = position, .isWall = isWall })) {
            bool ret = block_tick_list_add(&chunk->blockTickList, (BlockTickListEntry){ .position = position, .isWall = isWall });
        }
    }
}

BlockExtraResult chunk_set_block_extrapolating(Chunk* chunk, Vector2i position, BlockInstance blockValue, bool isWall, bool update_lighting) {
    BlockExtraResult result = chunk_get_block_extrapolating_ptr(chunk, position, isWall);
    if (!result.block || !result.chunk) return result;
    chunk_set_block(result.chunk, result.position, blockValue, isWall, update_lighting);
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
        UnloadMesh(chunk->liquidMesh);
        chunk->initializedMeshes = false;
    }

    chunk->wallMesh = (Mesh){ 0 };
    chunk->blockMesh = (Mesh){ 0 };
    chunk->liquidMesh = (Mesh){ 0 };

    chunk->blockMesh.vertexCount = blockVertexCount;
    chunk->wallMesh.vertexCount = wallVertexCount;
    chunk->liquidMesh.vertexCount = CHUNK_AREA * 6;

    chunk->blockMesh.triangleCount = blockVertexCount * 3;
    chunk->wallMesh.triangleCount = wallVertexCount * 3;
    chunk->liquidMesh.triangleCount = chunk->liquidMesh.vertexCount * 3;

    chunk->blockMesh.vertices = (float*)MemAlloc(blockVertexCount * 3 * sizeof(float));
    chunk->wallMesh.vertices = (float*)MemAlloc(wallVertexCount * 3 * sizeof(float));
    chunk->liquidMesh.vertices = (float*)MemAlloc(chunk->liquidMesh.vertexCount * 3 * sizeof(float));

    chunk->blockMesh.texcoords = (float*)MemAlloc(blockVertexCount * 2 * sizeof(float));
    chunk->wallMesh.texcoords = (float*)MemAlloc(wallVertexCount * 2 * sizeof(float));
    chunk->liquidMesh.texcoords = (float*)MemAlloc(chunk->liquidMesh.vertexCount * 2 * sizeof(float));

    chunk->blockMesh.colors = (unsigned char*)MemAlloc(blockVertexCount * 4 * sizeof(unsigned char));
    chunk->wallMesh.colors = (unsigned char*)MemAlloc(wallVertexCount * 4 * sizeof(unsigned char));
    chunk->liquidMesh.colors = (unsigned char*)MemAlloc(chunk->liquidMesh.vertexCount * 4 * sizeof(unsigned char));

    chunk->initializedMeshes = true;
}

void build_quad(Chunk* chunk, size_t* offsets, BlockInstance* blocks, Mesh* mesh, bool isWall, uint8_t x, uint8_t y, uint8_t brightness) {
    int i = x + (y * CHUNK_WIDTH);
	BlockRegistry* brg = br_get_block_registry(blocks[i].id);
    if (blocks[i].id <= 0 || brg->flags & BLOCK_FLAG_LIQUID) return;
    
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