#include "chunk.h"

#include "chunk_layer.h"
#include "game_settings.h"
#include "world_manager.h"
#include "registries/block_registry.h"
#include "lists/block_tick_list.h"
#include "block_state_bitfields.h"
#include "chunk_manager.h"
#include "types.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define FNL_IMPL
#include <thirdparty/FastNoiseLite.h>

static unsigned int posmod(int v, int m);

static Material matDefault;
static bool loadedMatDefault = false;

static bool chance_at(fnl_noise_type noiseType, float frequency, int gx, int gy, float threshold, int seed_offset) {
    fnl_state noise = fnlCreateState();
    noise.seed = get_world_info()->seed + seed_offset;
    noise.frequency = frequency;
    noise.noise_type = noiseType;

    return fnlGetNoise2D(&noise, gx, gy) > threshold;
}

void chunk_init(Chunk* chunk, Vector2i position)
{
    if (chunk == NULL) return;

    chunk->position = position;

    block_tick_list_clear(&chunk->blockTickList);

    chunk_layer_init(&chunk->layers[CHUNK_LAYER_FOREGROUND]);
    chunk_layer_init(&chunk->layers[CHUNK_LAYER_BACKGROUND]);

    // The liquid mesh won't change the amount of vertices so it doesn't need to allocate again
    chunk->liquidMesh = (Mesh){ 0 };
    chunk->liquidMesh.vertexCount = CHUNK_AREA * 6;
    chunk->liquidMesh.triangleCount = chunk->liquidMesh.vertexCount * 3;
    chunk->liquidMesh.vertices = (float*)MemAlloc(chunk->liquidMesh.vertexCount * 3 * sizeof(float));
    chunk->liquidMesh.colors = (unsigned char*)MemAlloc(chunk->liquidMesh.vertexCount * 4 * sizeof(unsigned char));

    UploadMesh(&chunk->liquidMesh, true);

    if (!loadedMatDefault) {
        matDefault = LoadMaterialDefault();
        loadedMatDefault = true;
    }

    chunk->initialized = true;
}

void chunk_regenerate(Chunk* chunk) {
    if (!chunk) return;

    for (int i = 0; i < CHUNK_AREA; i++) {
        chunk->layers[CHUNK_LAYER_BACKGROUND].blocks[i] = (BlockInstance){ 0, 0, NULL };
        chunk->layers[CHUNK_LAYER_FOREGROUND].blocks[i] = (BlockInstance){ 0, 0, NULL };
        chunk->light[i] = 0;
    }

    if (get_world_info()->preset == WORLD_GEN_PRESET_DEFAULT) {
        fnl_state terrainNoise = fnlCreateState();
        terrainNoise.seed = get_world_info()->seed;
        terrainNoise.frequency = 0.008f;
        terrainNoise.noise_type = FNL_NOISE_OPENSIMPLEX2;
        terrainNoise.fractal_type = FNL_FRACTAL_FBM;
    
        fnl_state detailNoise = fnlCreateState();
        detailNoise.seed = get_world_info()->seed + 1337;
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
    
                    BlockInstance newInst = { 0, 0, NULL };
    
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
    
                    if (w == 0) chunk->layers[CHUNK_LAYER_FOREGROUND].blocks[i] = newInst;
                    else chunk->layers[CHUNK_LAYER_BACKGROUND].blocks[i] = newInst;
                }
            }
        }
    } else if (get_world_info()->preset == WORLD_GEN_PRESET_FLAT) {
        if (chunk->position.y < 0) return;

        for (int w = 0; w < 2; w++) {
            for (int y = 0; y < CHUNK_WIDTH; y++) {
                for (int x = 0; x < CHUNK_WIDTH; x++) {
                    BlockInstance newInst = { 0, 0, NULL };
                    
                    int gy = chunk->position.y * CHUNK_WIDTH + y;
                    
                    if (gy == CHUNK_WIDTH / 2) {
                        newInst.id = BLOCK_GRASS_BLOCK;
                    } else if (gy > (CHUNK_WIDTH / 2) && gy <= (CHUNK_WIDTH / 2) + 10) {
                        newInst.id = BLOCK_DIRT;
                    } else if (gy > (CHUNK_WIDTH / 2) + 10) {
                        newInst.id = BLOCK_STONE;
                    }

                    int i = x + y * CHUNK_WIDTH;
                    
                    if (w == 0) chunk->layers[CHUNK_LAYER_FOREGROUND].blocks[i] = newInst;
                    else chunk->layers[CHUNK_LAYER_BACKGROUND].blocks[i] = newInst;
                }
            }
        }
    } else if (get_world_info()->preset == WORLD_GEN_PRESET_EMPTY) {
        if (chunk->position.x == 0 && chunk->position.y == 0) {
            for (int x = 0; x < CHUNK_WIDTH; x++) {
                int i = x + 1 * CHUNK_WIDTH;
                chunk->layers[CHUNK_LAYER_FOREGROUND].blocks[i] = (BlockInstance) { BLOCK_STONE, 0, NULL };
            }
        }
    }
}

void chunk_gen_liquid_mesh(Chunk* chunk) {
    if (!chunk) return;

    for (int v = 0; v < chunk->liquidMesh.vertexCount; v++) {
        int vi = v * 3;
        chunk->liquidMesh.vertices[vi + 0] = 0.0f;
        chunk->liquidMesh.vertices[vi + 1] = 0.0f;
        chunk->liquidMesh.vertices[vi + 2] = 0.0f;

        int ci = v * 4;
        chunk->liquidMesh.colors[ci + 0] = 0;
        chunk->liquidMesh.colors[ci + 1] = 0;
        chunk->liquidMesh.colors[ci + 2] = 0;
        chunk->liquidMesh.colors[ci + 3] = 0;
    }

    for (int i = 0; i < CHUNK_AREA; i++) {
        BlockRegistry* rg = br_get_block_registry(chunk->layers[CHUNK_LAYER_FOREGROUND].blocks[i].id);
        if (!rg) continue;
        if (!(rg->flags & BLOCK_FLAG_LIQUID)) continue;

        int x = i % CHUNK_WIDTH;
        int y = i / CHUNK_WIDTH;

        FlowingLiquidState* state = (FlowingLiquidState*)&chunk->layers[CHUNK_LAYER_FOREGROUND].blocks[i].state;
        float value = 0.125f + (state->level / 7.0f) * (1.0f - 0.125f);
        if (chunk->layers[CHUNK_LAYER_FOREGROUND].blocks[i].id == BLOCK_WATER_SOURCE) value = 1.0f;

        BlockExtraResult neighbors[4];
        chunk_get_block_neighbors_extra(chunk, (Vector2u) { x, y }, CHUNK_LAYER_FOREGROUND, neighbors);

        float left_value = value;
        float right_value = value;

        NeighborDirection dirs[] = { NEIGHBOR_LEFT, NEIGHBOR_RIGHT };
        for (int n = 0; n < 2; n++) {
            float* value = n == 0 ? &left_value : &right_value;

            BlockExtraResult neigh = neighbors[dirs[n]];
            BlockRegistry* nrg = neigh.reg;
            if (!nrg) continue;
            if (!(nrg->flags & BLOCK_FLAG_LIQUID)) continue;

            FlowingLiquidState* neighState = (FlowingLiquidState*)&neigh.block->state;
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

    UpdateMeshBuffer(chunk->liquidMesh, 0, chunk->liquidMesh.vertices, chunk->liquidMesh.vertexCount * 3 * sizeof(float), 0);
    UpdateMeshBuffer(chunk->liquidMesh, 3, chunk->liquidMesh.colors, chunk->liquidMesh.vertexCount * 4 * sizeof(unsigned char), 0);
}

void chunk_genmesh(Chunk* chunk) {
    if (chunk == NULL) return;
    unsigned int seed = (unsigned int)(chunk->position.x * 73856093 ^ chunk->position.y * 19349663);

    for (int i = 0; i < CHUNK_LAYER_COUNT; i++) {
        seed ^= (unsigned int)i * 1442695040888963407ull;

        int front_layer_id = i + 1;
        if (front_layer_id >= CHUNK_LAYER_COUNT) front_layer_id = CHUNK_LAYER_COUNT - 1;

        chunk_layer_genmesh(
            &chunk->layers[i],
            i,
            front_layer_id,
            chunk,
            seed,
            i == CHUNK_LAYER_BACKGROUND ? (uint8_t)get_game_settings()->wall_brightness : 255
        );
    }

    chunk_gen_liquid_mesh(chunk);
}

void chunk_update_tick_list(Chunk* chunk) {
    if (!chunk) return;
    block_tick_list_clear(&chunk->blockTickList);
    for (int i = 0; i < CHUNK_AREA; i++) {
        for (int l = 0; l < CHUNK_LAYER_COUNT; l++) {
            BlockInstance* binst = &chunk->layers[l].blocks[i];
            if (binst->id == BLOCK_AIR) continue;
            BlockRegistry* brg = br_get_block_registry(binst->id);
            if (!brg) continue;
            if (brg->tick_callback == NULL) continue;
            BlockTickListEntry entry = {
                .position = { i % CHUNK_WIDTH, i / CHUNK_WIDTH },
                .layer = l
            };
            block_tick_list_add(&chunk->blockTickList, entry);
        }
	}
}

void chunk_draw(Chunk* chunk) {
    if (!chunk) return;

    rlPushMatrix();

    rlTranslatef(
        chunk->position.x * CHUNK_WIDTH * TILE_SIZE,
        chunk->position.y * CHUNK_WIDTH * TILE_SIZE,
        0.0f
    );

    chunk_layer_draw(&chunk->layers[CHUNK_LAYER_BACKGROUND]);
    chunk_layer_draw(&chunk->layers[CHUNK_LAYER_FOREGROUND]);

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

    rlDrawRenderBatchActive();
    DrawMesh(chunk->liquidMesh, matDefault, MatrixIdentity());

    rlPopMatrix();
}

void chunk_tick(Chunk* chunk, uint8_t tick_value) {
    if (!chunk) return;

    bool changed = false;
    size_t count = chunk->blockTickList.count;

    for (size_t i = 0; i < count; i++) {
        BlockTickListEntry entry = chunk->blockTickList.entries[i];

        BlockInstance* ptr = chunk_get_block_ptr(chunk, entry.position, entry.layer);
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
        chunk_get_block_neighbors_extra(chunk, entry.position, entry.layer, neighbors);

        BlockExtraResult result = {
            .block = ptr,
            .reg = brg,
            .chunk = chunk,
            .position = entry.position,
            .idx = entry.position.x + (entry.position.y * CHUNK_WIDTH)
        };

        ChunkLayerEnum otherLayer = entry.layer == CHUNK_LAYER_BACKGROUND ? CHUNK_LAYER_FOREGROUND : CHUNK_LAYER_BACKGROUND;
        BlockInstance* other_inst = chunk_get_block_ptr(chunk, entry.position, otherLayer);
        BlockRegistry* other_br = br_get_block_registry(other_inst->id);

        BlockExtraResult other = {
            .block = other_inst,
            .reg = other_br,
            .chunk = chunk,
            .position = entry.position,
            .idx = entry.position.x + (entry.position.y * CHUNK_WIDTH)
        };

        uint8_t mod = tick_value % brg->tick_speed;
        if (mod == (brg->tick_speed-1)) {
            bool did_change = brg->tick_callback(result, other, neighbors, entry.layer);
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

    for (int i = 0; i < CHUNK_LAYER_COUNT; i++) {
        chunk_layer_free(&chunk->layers[i]);
    }

    UnloadMesh(chunk->liquidMesh);

    chunk->initialized = false;
}

void chunk_fill_light(Chunk* chunk, Vector2u startPoint, uint8_t newLightValue) {
    if (!chunk) return;
    if (newLightValue < 1 || newLightValue > 15) return;
    if (startPoint.x >= CHUNK_WIDTH) return;
    if (startPoint.y >= CHUNK_WIDTH) return;

    uint8_t current = chunk_get_light(chunk, startPoint);
    if (current >= newLightValue) return;

    chunk_set_light(chunk, startPoint, newLightValue);

    uint8_t decayAmount = 4;

    BlockInstance binst = chunk_get_block(chunk, startPoint, CHUNK_LAYER_FOREGROUND);
    BlockRegistry* br = br_get_block_registry(binst.id);

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

void chunk_propagate_power_wire(Chunk* chunk, Vector2u startPoint, ChunkLayerEnum layer, uint8_t newPowerValue) {
    if (!chunk) return;
    if (startPoint.x >= CHUNK_WIDTH || startPoint.y >= CHUNK_WIDTH) return;

    if (newPowerValue > 15) return;
    if (newPowerValue < 1) {
        chunk_genmesh(chunk);
        return;
    }

    int i = startPoint.x + startPoint.y * CHUNK_WIDTH;

    BlockInstance* bptr = &chunk->layers[layer].blocks[i];
    if (bptr->id != BLOCK_POWER_WIRE) return;

    PowerWireState* s = (PowerWireState*)&bptr->state;
    uint8_t current = s->power;
    if (current >= newPowerValue) return;

    s->power = newPowerValue;

    BlockExtraResult neighbors[4];
    chunk_get_block_neighbors_extra(chunk, startPoint, layer, neighbors);

    for (int i = 0; i < 4; i++) {
        if (neighbors[i].chunk) {
            chunk_propagate_power_wire(neighbors[i].chunk, neighbors[i].position, layer, newPowerValue - 1);
        }
    }

    ChunkLayerEnum otherLayer = layer == CHUNK_LAYER_BACKGROUND ? CHUNK_LAYER_FOREGROUND : CHUNK_LAYER_BACKGROUND;
    chunk_propagate_power_wire(chunk, startPoint, otherLayer, newPowerValue - 1);
}

void chunk_propagate_remove_power_wire(Chunk* chunk, Vector2u point, ChunkLayerEnum layer) {
    if (!chunk) return;
    if (point.x >= CHUNK_WIDTH) return;
    if (point.y >= CHUNK_WIDTH) return;

    int idx = (int)point.x + (int)point.y * CHUNK_WIDTH;

    BlockInstance* bptr = &chunk->layers[layer].blocks[idx];
    if (bptr->id != BLOCK_POWER_WIRE) return;

    PowerWireState* s = (PowerWireState*)&bptr->state;
    uint8_t old_power = s->power;
    if (old_power == 0) {
        chunk_genmesh(chunk);
        return;
    }

    uint8_t max_power = 0;

    BlockExtraResult neighbors[4];
    chunk_get_block_neighbors_extra(chunk, point, layer, neighbors);

    for (int i = 0; i < 4; i++) {
        BlockExtraResult neigh = neighbors[i];
        if (!neigh.block || !neigh.reg) continue;

        if (neigh.block->id == BLOCK_POWER_WIRE) {
            PowerWireState* ns = (PowerWireState*)&neigh.block->state;
            if (ns->power > 0) {
                uint8_t cand = ns->power - 1;
                if (cand > max_power) max_power = cand;
            }
        }
    }

    if (max_power < old_power) {
        s->power = max_power;

        for (int j = 0; j < 4; j++) {
            BlockExtraResult neigh = neighbors[j];
            if (!neigh.block || !neigh.reg) continue;
            if (neigh.block->id != BLOCK_POWER_WIRE) continue;

            if (neigh.chunk) {
                chunk_propagate_remove_power_wire(neigh.chunk, neigh.position, layer);
            }
        }
    }
}

DownProjectionResult chunk_get_block_projected_downwards(Chunk* chunk, Vector2u startPoint, ChunkLayerEnum layer, bool goToNeighbor) {
    DownProjectionResult empty = { { NULL, NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX }, { NULL, NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX } };
    if (!chunk) return empty;

    for (unsigned int y = startPoint.y; y < CHUNK_WIDTH; y++) {
        if (y == startPoint.y) {
            BlockRegistry* br = br_get_block_registry(chunk_get_block(chunk, startPoint, layer).id);
            if (!(br->flags & BLOCK_FLAG_REPLACEABLE)) return empty;
        }

        BlockExtraResult down = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { startPoint.x, y + 1 }, layer);
        if (down.block == NULL) return empty;

        BlockRegistry* br = br_get_block_registry(down.block->id);
        if (br == NULL) return empty;

        if (!(br->flags & BLOCK_FLAG_REPLACEABLE)) {
            uint8_t idx = startPoint.x + (y * CHUNK_WIDTH);
            return (DownProjectionResult) {
                .replaced = (BlockExtraResult){
                    .block = &chunk->layers[layer].blocks[idx],
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
        return chunk_get_block_projected_downwards(chunk->neighbors.down, (Vector2u) { startPoint.x, 0 }, layer, goToNeighbor);
    }
    else { return empty; }
}

BlockInstance* chunk_get_block_ptr(Chunk* chunk, Vector2u position, ChunkLayerEnum layer) {
    if (!chunk) return NULL;
    if (position.x >= CHUNK_WIDTH ||position.y >= CHUNK_WIDTH) return NULL;
    return &chunk->layers[layer].blocks[position.x + (position.y * CHUNK_WIDTH)];
}

BlockExtraResult chunk_get_block_extrapolating_ptr(Chunk* chunk, Vector2i position, ChunkLayerEnum layer) {
    if (!chunk) return (BlockExtraResult){ NULL, NULL, NULL, { UINT8_MAX, UINT8_MAX }, UINT8_MAX };

    if (position.x >= 0 && position.y >= 0 && position.x < CHUNK_WIDTH && position.y < CHUNK_WIDTH) {
        BlockInstance* inst = chunk_get_block_ptr(chunk, (Vector2u) { position.x, position.y }, layer);
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

        BlockInstance* inst = chunk_get_block_ptr(neighbor, relPos, layer);
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
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return NULL;
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

void chunk_set_block(Chunk* chunk, Vector2u position, BlockInstance blockValue, ChunkLayerEnum layer, bool update_lighting) {
    BlockInstance* ptr = chunk_get_block_ptr(chunk, position, layer);
    if (!ptr) return;
    if (ptr->id == blockValue.id && ptr->state == blockValue.state) return;

    bool can_place = true;

    BlockRegistry* new_br = br_get_block_registry(blockValue.id);
    if (!new_br) return;

    BlockExtraResult neighbors[4];
    chunk_get_block_neighbors_extra(chunk, position, layer, neighbors);

    BlockExtraResult self = {
        .block = &blockValue,
        .reg = new_br,
        .chunk = chunk,
        .position = position,
        .idx = position.x + (position.y * CHUNK_WIDTH)
    };

    ChunkLayerEnum otherLayer = layer == CHUNK_LAYER_FOREGROUND ? CHUNK_LAYER_BACKGROUND : CHUNK_LAYER_FOREGROUND;

    BlockInstance* other_inst = chunk_get_block_ptr(chunk, position, otherLayer);
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
        chunk_get_block_neighbors_extra(chunk, position, layer, neighbors);

        can_place = new_br->state_resolver(self, other, neighbors, layer);
    }
    if (!can_place) return;

    // Handle previous block (destroy)
    if (ptr->id > 0) {
        BlockRegistry* old_br = br_get_block_registry(ptr->id);
        if (old_br) {
            if (old_br->tick_callback != NULL) {
                block_tick_list_remove(&chunk->blockTickList, (BlockTickListEntry) { position, layer });
            }

            if (old_br->destroy_callback) {
                BlockExtraResult res = {
                    .block = ptr,
                    .reg = old_br,
                    .chunk = chunk,
                    .position = position,
                    .idx = position.x + (position.y * CHUNK_WIDTH)
                };
                old_br->destroy_callback(res, other, neighbors, layer);
            }
            if (old_br->free_data) {
                old_br->free_data(ptr->data);
            }
        }
    }

    // Set the block
    *ptr = blockValue;

    // Resolve state for neighboring blocks
    BlockExtraResult otherNeighbors[4];
    chunk_get_block_neighbors_extra(chunk, position, otherLayer, otherNeighbors);

    for (int i = 0; i < 4; i++) {
        if (neighbors[i].block == NULL) continue;
        BlockRegistry* nbr_br = neighbors[i].reg;
        if (!nbr_br || nbr_br->state_resolver == NULL) continue;

        BlockExtraResult neighbors2[4];
        chunk_get_block_neighbors_extra(neighbors[i].chunk, neighbors[i].position, layer, neighbors2);

        if (nbr_br->state_resolver) {
            bool r = nbr_br->state_resolver(neighbors[i], otherNeighbors[i], neighbors2, layer);
            if (!r) *neighbors[i].block = (BlockInstance){ 0, 0, NULL };
        }
    }

    // Resolve state for the other block (wall or block)
    if (other_br->state_resolver) {
        bool r = other_br->state_resolver(other, self, otherNeighbors, otherLayer);
        if (!r) *other.block = (BlockInstance){ 0, 0, NULL };
    }

    if (update_lighting) chunk_manager_update_lighting();

    // Add tick for new block if applicable
    if (new_br->tick_callback != NULL) {
        if (!block_tick_list_contains(&chunk->blockTickList, (BlockTickListEntry){ position, layer })) {
            block_tick_list_add(&chunk->blockTickList, (BlockTickListEntry){ position, layer });
        }
    }
}

BlockExtraResult chunk_set_block_extrapolating(Chunk* chunk, Vector2i position, BlockInstance blockValue, ChunkLayerEnum layer, bool update_lighting) {
    BlockExtraResult result = chunk_get_block_extrapolating_ptr(chunk, position, layer);
    if (!result.block || !result.chunk) return result;
    chunk_set_block(result.chunk, result.position, blockValue, layer, update_lighting);
    return result;
}

BlockInstance chunk_get_block(Chunk* chunk, Vector2u position, ChunkLayerEnum layer) {
    BlockInstance* ptr = chunk_get_block_ptr(chunk, position, layer);
    if (!ptr) return (BlockInstance){ 0, 0, NULL };
    return *ptr;
}

BlockInstance chunk_get_block_extrapolating(Chunk* chunk, Vector2i position, ChunkLayerEnum layer) {
    BlockExtraResult result = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x, position.y }, layer);
    if (!result.block || !result.chunk) return (BlockInstance){ 0, 0, NULL };
    return chunk_get_block(result.chunk, result.position, layer);
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

void chunk_get_block_neighbors(Chunk* chunk, Vector2u position, ChunkLayerEnum layer, BlockInstance output[4]) {
    output[NEIGHBOR_TOP] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y - 1 }, layer);
    output[NEIGHBOR_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y }, layer);
    output[NEIGHBOR_BOTTOM] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y + 1 }, layer);
    output[NEIGHBOR_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y }, layer);
}

void chunk_get_block_neighbors_extra(Chunk* chunk, Vector2u position, ChunkLayerEnum layer, BlockExtraResult output[4]) {
    output[NEIGHBOR_TOP] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x, position.y - 1 }, layer);
    output[NEIGHBOR_RIGHT] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x + 1, position.y }, layer);
    output[NEIGHBOR_BOTTOM] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x, position.y + 1 }, layer);
    output[NEIGHBOR_LEFT] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x - 1, position.y }, layer);
}

void chunk_get_block_neighbors_with_corners(Chunk* chunk, Vector2u position, ChunkLayerEnum layer, BlockInstance output[8]) {
    output[NEIGHBOR_TOP] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y - 1 }, layer);
    output[NEIGHBOR_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y }, layer);
    output[NEIGHBOR_BOTTOM] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x, position.y + 1 }, layer);
    output[NEIGHBOR_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y }, layer);

    output[NEIGHBOR_TOP_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y - 1 }, layer);
    output[NEIGHBOR_TOP_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y - 1 }, layer);
    output[NEIGHBOR_BOTTOM_RIGHT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x + 1, position.y + 1 }, layer);
    output[NEIGHBOR_BOTTOM_LEFT] = chunk_get_block_extrapolating(chunk, (Vector2i) { position.x - 1, position.y + 1 }, layer);
}

void chunk_get_block_neighbors_with_corners_extra(Chunk* chunk, Vector2u position, ChunkLayerEnum layer, BlockExtraResult output[8]) {
    output[NEIGHBOR_TOP] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x, position.y - 1 }, layer);
    output[NEIGHBOR_RIGHT] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x + 1, position.y }, layer);
    output[NEIGHBOR_BOTTOM] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x, position.y + 1 }, layer);
    output[NEIGHBOR_LEFT] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x - 1, position.y }, layer);

    output[NEIGHBOR_TOP_LEFT] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x - 1, position.y - 1 }, layer);
    output[NEIGHBOR_TOP_RIGHT] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x + 1, position.y - 1 }, layer);
    output[NEIGHBOR_BOTTOM_RIGHT] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x + 1, position.y + 1 }, layer);
    output[NEIGHBOR_BOTTOM_LEFT] = chunk_get_block_extrapolating_ptr(chunk, (Vector2i) { position.x - 1, position.y + 1 }, layer);
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