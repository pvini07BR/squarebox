#include "chunk_layer.h"
#include "game_settings.h"
#include "registries/block_models.h"
#include "registries/block_registry.h"
#include "texture_atlas.h"
#include "types.h"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#include <stdlib.h>

void chunk_layer_init(ChunkLayer* layer) {
    if (!layer) return;

    for (int i = 0; i < CHUNK_AREA; i++) {
        layer->blocks[i].data = NULL;
        layer->blocks[i].id = BLOCK_AIR;
        layer->blocks[i].state = 0;
    }

    layer->initializedMesh = false;
}

void chunk_layer_genmesh(ChunkLayer* layer, ChunkLayerEnum layer_id, ChunkLayerEnum front_layer_id, void* c, unsigned int chunk_pos_seed, uint8_t brightness) {
    if (!layer || !c) return;
    Chunk* chunk = (Chunk*)c;

    // Get total amount of vertices needed
    int vertexCount = 0;
    for (int i = 0; i < CHUNK_AREA; i++) {
        BlockVariant bvar = br_get_block_variant(layer->blocks[i].id, layer->blocks[i].state);
        layer->vertexOffsets[i] = vertexCount;
        vertexCount += block_models_get_vertex_count(bvar.model_idx);
    }

    if (layer->initializedMesh == true) {
        UnloadMesh(layer->mesh);
        layer->initializedMesh = false;
    }

    // Reset mesh
    layer->mesh = (Mesh){0};
    layer->mesh.vertexCount = vertexCount;
    layer->mesh.triangleCount = vertexCount * 3;
    layer->mesh.vertices = (float*)MemAlloc(vertexCount * 3 * sizeof(float));
    layer->mesh.texcoords = (float*)MemAlloc(vertexCount * 2 * sizeof(float));
    layer->mesh.colors = (unsigned char*)MemAlloc(vertexCount * 4 * sizeof(unsigned char));
    layer->initializedMesh = true;

    // Now generate the quads for the mesh
    for (int i = 0; i < CHUNK_AREA; i++) {
        BlockInstance block = layer->blocks[i];
        BlockRegistry* brg = br_get_block_registry(block.id);

        if (block.id <= 0 || brg->flags & BLOCK_FLAG_LIQUID) continue;
        
        int x = i % CHUNK_WIDTH;
        int y = i / CHUNK_WIDTH;

        uint8_t cornerValues[4] = { brightness, brightness, brightness, brightness };

        if (!get_game_settings()->smooth_lighting) {
            uint8_t lightValue = (uint8_t)((chunk->light[i] / 15.0f) * 255.0f);
            uint8_t reduction = 255 - lightValue;

            for (int i = 0; i < 4; i++) {
                if (cornerValues[i] > reduction) cornerValues[i] -= reduction;
                else cornerValues[i] = 0;
            }
        }
        else {
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

        if (get_game_settings()->wall_ao && front_layer_id > layer_id && brg->lightLevel <= 0) {
            BlockExtraResult neighbors[8];
            chunk_get_block_neighbors_with_corners_extra(chunk, (Vector2u) { x, y }, front_layer_id, neighbors);
            
            int aoRules[8][2] = {
                {0, 1},     // Top
                {1, 2},     // Right
                {2, 3},     // Bottom
                {0, 3},     // Left

                {0, -1},    // Top Left
                {1, -1},    // Top Right
                {2, -1},    // Bottom Right
                {3, -1},    // Bottom Left
            };

            for (int dir = 0; dir < 8; dir++) {
                if (!neighbors[dir].reg) continue;
                BlockRegistry* reg = neighbors[dir].reg;
                if ((!(reg->lightLevel == BLOCK_LIGHT_TRANSPARENT) && (reg->flags & BLOCK_FLAG_FULL_BLOCK) && (reg->lightLevel <= 0))) {
                    for (int c = 0; c < 2; c++) {
                        int corner = aoRules[dir][c];
                        if (corner >= 0) {
                            cornerValues[corner] = fminf(cornerValues[corner], (uint8_t)get_game_settings()->wall_ao_brightness);
                        }
                    }
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

        unsigned int h = chunk_pos_seed;
        h ^= x * TILE_SIZE * 374761393u;
        h ^= y * TILE_SIZE * 668265263u;
        h = (h ^ (h >> 13)) * 1274126177u;

        srand(h);

        bool flipUVH = (brg->flags & BLOCK_FLAG_FLIP_H) && (rand() % 2) ? true : false;
        bool flipUVV = (brg->flags & BLOCK_FLAG_FLIP_V) && (rand() % 2) ? true : false;

        uint8_t variantIdx = block.state;
        if (brg->variant_selector) {
            variantIdx = brg->variant_selector(block.state);
        }
        BlockVariant bvar = br_get_block_variant(block.id, variantIdx);

        bm_set_block_model(
            layer->vertexOffsets,
            &layer->mesh,
            (Vector2u) { x, y },
            colors,
            bvar.model_idx,
            bvar.atlas_idx,
            flipUVH,
            flipUVV,
            bvar.flipH,
            bvar.flipV,
            bvar.rotation
        );
    }

    UploadMesh(&layer->mesh, false);
}

void chunk_layer_draw(ChunkLayer* layer) {
    if (!layer) return;

    rlDrawRenderBatchActive();

    if (layer->initializedMesh) {
        DrawMesh(layer->mesh, texture_atlas_get_material(), MatrixIdentity());
    }

    for (int i = 0; i < CHUNK_AREA; i++) {
        BlockInstance* block = &layer->blocks[i];
        if (!block->data) continue;
        BlockRegistry* rg = br_get_block_registry(block->id);
        if (rg->overlay_draw) {
            int x = i % CHUNK_WIDTH;
            int y = i / CHUNK_WIDTH;

            rg->overlay_draw(block->data, (Vector2) { x * TILE_SIZE, y * TILE_SIZE }, block->state);
        }
    }
}

void chunk_layer_free(ChunkLayer* layer) {
    if (!layer) return;

    for (int i = 0; i < CHUNK_AREA; i++) {
        BlockInstance* block = &layer->blocks[i];
        if (!block->data) continue;
        BlockRegistry* rg = br_get_block_registry(block->id);
        if (rg->destroy_callback) {
            BlockExtraResult result;
            result.block = block;
            rg->destroy_callback(result);
        }
    }

    if (layer->initializedMesh) {
        UnloadMesh(layer->mesh);
        layer->initializedMesh = false;
    }
}