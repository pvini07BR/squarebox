#include "chunk_layer.h"
#include "raymath.h"
#include "registries/block_models.h"
#include "registries/block_registry.h"
#include "texture_atlas.h"
#include "types.h"

#include <raylib.h>
#include <rlgl.h>

void chunk_layer_init(ChunkLayer* layer, float brightness) {
    if (!layer) return;

    for (int i = 0; i < CHUNK_AREA; i++) {
        layer->blocks[i].data = NULL;
        layer->blocks[i].id = BLOCK_AIR;
        layer->blocks[i].state = 0;
    }

    layer->initializedMesh = false;
    layer->brightness = brightness;
}

void chunk_layer_genmesh(ChunkLayer* layer) {
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

    Color c = (Color) { layer->brightness * 255, layer->brightness * 255, layer->brightness * 255, 255 };
    Color colors[] = { c, c, c, c };

    for (int i = 0; i < CHUNK_AREA; i++) {
        BlockInstance block = layer->blocks[i];
        BlockRegistry* brg = br_get_block_registry(block.id);

        if (block.id <= 0 || brg->flags & BLOCK_FLAG_LIQUID) continue;

        int x = i % CHUNK_WIDTH;
        int y = i / CHUNK_WIDTH;

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
            false,
            false,
            bvar.flipH,
            bvar.flipV,
            bvar.rotation
        );
    }

    UploadMesh(&layer->mesh, false);
}

void chunk_layer_draw(ChunkLayer* layer) {
    if (!layer) return;

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

            rg->overlay_draw(block->data, (Vector2) { x * TILE_SIZE, y * TILE_SIZE });
        }
    }
}

void chunk_layer_free(ChunkLayer* layer) {
    if (!layer) return;

    if (layer->initializedMesh) {
        UnloadMesh(layer->mesh);
        layer->initializedMesh = false;
    }
}