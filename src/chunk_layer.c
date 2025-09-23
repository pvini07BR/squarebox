#include "chunk_layer.h"
#include "raymath.h"
#include "registries/block_models.h"
#include "registries/block_registry.h"
#include "texture_atlas.h"
#include "types.h"

#include <raylib.h>
#include <rlgl.h>

void chunk_layer_init(ChunkLayer* layer, uint8_t brightness) {
    if (!layer) return;

    for (int i = 0; i < CHUNK_AREA; i++) {
        layer->blocks[i].data = NULL;
        layer->blocks[i].id = BLOCK_AIR;
        layer->blocks[i].state = 0;
    }

    layer->initializedMesh = false;
    layer->brightness = brightness;
}

void chunk_layer_genmesh(ChunkLayer* layer, uint8_t lightmap[CHUNK_AREA], unsigned int chunk_pos_seed) {
    if (!layer) return;

    // Get total amount of vertices needed
    int vertexCount = 0;
    for (int i = 0; i < CHUNK_AREA; i++) {
        BlockRegistry* rg = br_get_block_registry(layer->blocks[i].id);
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

        uint8_t brightness = layer->brightness;

        uint8_t lightValue = (uint8_t)(((float)lightmap[i] / 15.0f) * 255.0f);
        uint8_t reduction = 255 - lightValue;

        if (brightness > reduction) brightness -= reduction;
        else brightness = 0;

        Color c = (Color) { brightness, brightness, brightness, 255 };
        Color colors[] = { c, c, c, c };

        int x = i % CHUNK_WIDTH;
        int y = i / CHUNK_WIDTH;

        unsigned int h = chunk_pos_seed;
        h ^= x * 374761393u;
        h ^= y * 668265263u;
        h = (h ^ (h >> 13)) * 1274126177u;

        bool flipUVH = (brg->flags & BLOCK_FLAG_FLIP_H) && (h & 1) ? true : false;
        bool flipUVV = (brg->flags & BLOCK_FLAG_FLIP_V) && (h & 2) ? true : false;

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