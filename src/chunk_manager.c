#include "chunk_manager.h"
#include "chunk_layer.h"
#include "game.h"
#include "registries/block_registry.h"
#include "chunk.h"
#include "types.h"
#include "world_manager.h"

#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <memory.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

static bool initialized = false;

static uint8_t chunk_view_width = 5;
static uint8_t chunk_view_height = 3;
static size_t chunk_count = 0;

static Chunk* chunks = NULL;
static Vector2i currentChunkPos = { 0, 0 };

static unsigned int tick_counter = 0;

void chunk_manager_init(Vector2i center, uint8_t cvw, uint8_t cvh) {
    chunk_view_width = cvw;
    chunk_view_height = cvh;
    chunk_count = chunk_view_width * chunk_view_height;

    chunks = (Chunk*)malloc(sizeof(Chunk) * chunk_count);
    if (!chunks) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for the Chunks.\n");
        return;
    }

    for (size_t c = 0; c < chunk_count; c++) chunks[c].initialized = false;

    initialized = true;

	chunk_manager_relocate(center);
}

void chunk_manager_relocate(Vector2i newCenter) {
    if (!initialized) return;

    int cw = chunk_view_width;
    int ch = chunk_view_height;
    size_t count = chunk_count;

    int min_x = newCenter.x - (cw / 2);
    int min_y = newCenter.y - (ch / 2);
    int max_x = min_x + cw - 1;
    int max_y = min_y + ch - 1;

    Chunk* new_chunks = (Chunk*)malloc(sizeof(Chunk) * count);
    if (!new_chunks) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for relocating Chunks.\n");
        return;
    }
    for (size_t i = 0; i < count; i++) {
        memset(&new_chunks[i], 0, sizeof(Chunk));
        new_chunks[i].initialized = false;
    }

    bool* occupied = (bool*)malloc(sizeof(bool) * count);
    if (!occupied) {
        free(new_chunks);
        TraceLog(LOG_ERROR, "Failed to allocate memory for relocate helper.\n");
        return;
    }
    for (size_t i = 0; i < count; i++) occupied[i] = false;

    for (size_t i = 0; i < count; i++) {
        Chunk* old = &chunks[i];
        if (!old->initialized) continue;

        int px = old->position.x;
        int py = old->position.y;

        if (px >= min_x && px <= max_x && py >= min_y && py <= max_y) {
            int nx = px - min_x;
            int ny = py - min_y;
            size_t new_index = (size_t)(ny * cw + nx);

            memcpy(&new_chunks[new_index], old, sizeof(Chunk));
            occupied[new_index] = true;

            old->initialized = false;
        }
    }

    for (size_t i = 0; i < count; i++) {
        Chunk* old = &chunks[i];
        if (old->initialized) {
            if (!game_is_demo_mode()) world_manager_save_chunk(old);
            chunk_free(old);
        }
    }

    for (size_t i = 0; i < count; i++) {
        if (!occupied[i]) {
            int x = i % cw;
            int y = i / cw;
            int chunk_x = min_x + x;
            int chunk_y = min_y + y;

            memset(&new_chunks[i], 0, sizeof(Chunk));
            chunk_init(&new_chunks[i], (Vector2i) { chunk_x, chunk_y });
            if (game_is_demo_mode()) {
                chunk_regenerate(&new_chunks[i]);
            } else {
                ChunkLoadStatus status = world_manager_load_chunk(&new_chunks[i]);
                if (status == CHUNK_LOAD_ERROR_NOT_FOUND) {
                    chunk_regenerate(&new_chunks[i]);
                } else if (status == CHUNK_LOAD_ERROR_FATAL) {
                    chunk_free(&new_chunks[i]);
                }
            }
			chunk_update_tick_list(&new_chunks[i]);
            occupied[i] = true;
        }
    }

    free(chunks);
    free(occupied);

    chunks = new_chunks;
    currentChunkPos = newCenter;

    for (int y = 0; y < ch; y++) {
        for (int x = 0; x < cw; x++) {
            size_t idx = y * cw + x;
            Chunk* c = &chunks[idx];

            c->neighbors.up = (y > 0) ? &chunks[(y - 1) * cw + x] : NULL;
            c->neighbors.right = (x < cw - 1) ? &chunks[y * cw + (x + 1)] : NULL;
            c->neighbors.down = (y < ch - 1) ? &chunks[(y + 1) * cw + x] : NULL;
            c->neighbors.left = (x > 0) ? &chunks[y * cw + (x - 1)] : NULL;

            c->neighbors.upLeft = (x > 0 && y > 0) ? &chunks[(y - 1) * cw + (x - 1)] : NULL;
            c->neighbors.upRight = (x < cw - 1 && y > 0) ? &chunks[(y - 1) * cw + (x + 1)] : NULL;
            c->neighbors.downLeft = (x > 0 && y < ch - 1) ? &chunks[(y + 1) * cw + (x - 1)] : NULL;
            c->neighbors.downRight = (x < cw - 1 && y < ch - 1) ? &chunks[(y + 1) * cw + (x + 1)] : NULL;
        }
    }

    chunk_manager_update_lighting();
}

void chunk_manager_set_view(uint8_t new_view_width, uint8_t new_view_height) {
    if (!initialized) return;
    if (new_view_width == chunk_view_width && new_view_height == chunk_view_height) return;

    size_t old_count = chunk_count;

    int new_cw = new_view_width;
    int new_ch = new_view_height;
    size_t new_count = (size_t)new_cw * (size_t)new_ch;

    int new_min_x = currentChunkPos.x - (new_cw / 2);
    int new_min_y = currentChunkPos.y - (new_ch / 2);
    int new_max_x = new_min_x + new_cw - 1;
    int new_max_y = new_min_y + new_ch - 1;

    Chunk* new_chunks = (Chunk*)malloc(sizeof(Chunk) * new_count);
    if (!new_chunks) {
        TraceLog(LOG_ERROR, "Failed to allocate memory for new chunk view.\n");
        return;
    }
    for (size_t i = 0; i < new_count; i++) {
        memset(&new_chunks[i], 0, sizeof(Chunk));
        new_chunks[i].initialized = false;
    }

    bool* occupied = (bool*)malloc(sizeof(bool) * new_count);
    if (!occupied) {
        free(new_chunks);
        TraceLog(LOG_ERROR, "Failed to allocate helper memory.\n");
        return;
    }
    for (size_t i = 0; i < new_count; i++) occupied[i] = false;

    for (size_t i = 0; i < old_count; i++) {
        Chunk* old = &chunks[i];
        if (!old->initialized) continue;

        int px = old->position.x;
        int py = old->position.y;

        if (px >= new_min_x && px <= new_max_x && py >= new_min_y && py <= new_max_y) {
            int nx = px - new_min_x;
            int ny = py - new_min_y;
            size_t new_index = (size_t)(ny * new_cw + nx);

            memcpy(&new_chunks[new_index], old, sizeof(Chunk));
            occupied[new_index] = true;

            old->initialized = false;
        }
    }

    for (size_t i = 0; i < old_count; i++) {
        Chunk* old = &chunks[i];
        if (old->initialized) {
            if (!game_is_demo_mode()) world_manager_save_chunk(old);
            chunk_free(old);
            old->initialized = false;
        }
    }

    for (size_t i = 0; i < new_count; i++) {
        if (!occupied[i]) {
            int x = i % new_cw;
            int y = i / new_cw;
            int chunk_x = new_min_x + x;
            int chunk_y = new_min_y + y;

            memset(&new_chunks[i], 0, sizeof(Chunk));
            chunk_init(&new_chunks[i], (Vector2i) { chunk_x, chunk_y });
            if (game_is_demo_mode()) {
                chunk_regenerate(&new_chunks[i]);
            } else {
                ChunkLoadStatus status = world_manager_load_chunk(&new_chunks[i]);
                if (status == CHUNK_LOAD_ERROR_NOT_FOUND) {
                    chunk_regenerate(&new_chunks[i]);
                } else if (status == CHUNK_LOAD_ERROR_FATAL) {
                    chunk_free(&new_chunks[i]);
                }
            }
			chunk_update_tick_list(&new_chunks[i]);
            occupied[i] = true;
        }
    }

    free(chunks);
    free(occupied);

    chunks = new_chunks;
    chunk_view_width = new_view_width;
    chunk_view_height = new_view_height;
    chunk_count = new_count;

    for (int y = 0; y < new_ch; y++) {
        for (int x = 0; x < new_cw; x++) {
            size_t idx = y * new_cw + x;
            Chunk* c = &chunks[idx];

            c->neighbors.up = (y > 0) ? &chunks[(y - 1) * new_cw + x] : NULL;
            c->neighbors.right = (x < new_cw - 1) ? &chunks[y * new_cw + (x + 1)] : NULL;
            c->neighbors.down = (y < new_ch - 1) ? &chunks[(y + 1) * new_cw + x] : NULL;
            c->neighbors.left = (x > 0) ? &chunks[y * new_cw + (x - 1)] : NULL;

            c->neighbors.upLeft = (x > 0 && y > 0) ? &chunks[(y - 1) * new_cw + (x - 1)] : NULL;
            c->neighbors.upRight = (x < new_cw - 1 && y > 0) ? &chunks[(y - 1) * new_cw + (x + 1)] : NULL;
            c->neighbors.downLeft = (x > 0 && y < new_ch - 1) ? &chunks[(y + 1) * new_cw + (x - 1)] : NULL;
            c->neighbors.downRight = (x < new_cw - 1 && y < new_ch - 1) ? &chunks[(y + 1) * new_cw + (x + 1)] : NULL;
        }
    }

    chunk_manager_update_lighting();
}

void chunk_manager_update_lighting() {
    if (!initialized) return;

    for (size_t c = 0; c < chunk_count; c++) {
        for (int i = 0; i < CHUNK_AREA; i++) chunks[c].light[i] = 0;
    }

    for (size_t c = 0; c < chunk_count; c++) {
        for (int i = 0; i < CHUNK_AREA; i++) {
            BlockInstance b = chunks[c].layers[CHUNK_LAYER_FOREGROUND].blocks[i];
            BlockInstance w = chunks[c].layers[CHUNK_LAYER_BACKGROUND].blocks[i];

            BlockRegistry* bbr = br_get_block_registry(b.id);
            BlockRegistry* wbr = br_get_block_registry(w.id);

            int x = i % CHUNK_WIDTH;
            int y = i / CHUNK_WIDTH;

            if ((bbr->lightLevel == BLOCK_LIGHT_TRANSPARENT || !(bbr->flags & BLOCK_FLAG_FULL_BLOCK)) && (wbr->lightLevel == BLOCK_LIGHT_TRANSPARENT || !(wbr->flags & BLOCK_FLAG_FULL_BLOCK))) {
                chunk_fill_light(&chunks[c], (Vector2u) { x, y }, 15);
            }
            else if (bbr->lightLevel > 0 || wbr->lightLevel > 0) {
                uint8_t maxLight = fmax(bbr->lightLevel, wbr->lightLevel);
                chunk_fill_light(&chunks[c], (Vector2u) { x, y }, maxLight);
            }
        }
    }

    for (size_t c = 0; c < chunk_count; c++) chunk_genmesh(&chunks[c]);
}

void chunk_manager_draw(bool draw_lines) {
    if (!initialized) return;

    for (size_t i = 0; i < chunk_count; i++) {
        chunk_draw(&chunks[i]);
    }

    if (draw_lines) {
        for (size_t i = 0; i < chunk_count; i++) {
            rlPushMatrix();
            rlTranslatef(
                chunks[i].position.x * CHUNK_WIDTH * TILE_SIZE,
                chunks[i].position.y * CHUNK_WIDTH * TILE_SIZE,
                0.0f
            );

            DrawRectangleLines(
                0,
                0,
                CHUNK_WIDTH * TILE_SIZE,
                CHUNK_WIDTH * TILE_SIZE,
                WHITE
            );

            rlPopMatrix();
        }
    }
}

void chunk_manager_draw_liquids() {
    if (!initialized) return;

    for (size_t i = 0; i < chunk_count; i++) {
        chunk_draw_liquids(&chunks[i]);
    }
}

void chunk_manager_tick() {
    if (!initialized) return;

    for (size_t i = 0; i < chunk_count; i++) {
		chunk_tick(&chunks[i], tick_counter);
    }

    tick_counter++;
}

void chunk_manager_clear(bool saveChunks) {
    if (!initialized) return;
    for (size_t c = 0; c < chunk_count; c++) {
        if (chunks[c].initialized) {
            if (saveChunks) world_manager_save_chunk(&chunks[c]);
            chunk_free(&chunks[c]);
        }
	}
}

void chunk_manager_free() {
    if (!initialized) return;

    if (chunks) {
        for (size_t c = 0; c < chunk_count; c++) {
            if (chunks[c].initialized) {
                if (!game_is_demo_mode()) world_manager_save_chunk(&chunks[c]);
                chunk_free(&chunks[c]);
            }
        }
        free(chunks);
        chunks = NULL;
    }

    initialized = false;
}

uint8_t chunk_manager_get_view_width()
{
    return chunk_view_width;
}

uint8_t chunk_manager_get_view_height()
{
    return chunk_view_height;
}

bool chunk_manager_interact(Vector2i position, ChunkLayerEnum layer, ItemSlot holdingItem) {
    if (!initialized) return false;

    Vector2i chunkPos = {
        (int)floorf((float)position.x / (float)CHUNK_WIDTH),
        (int)floorf((float)position.y / (float)CHUNK_WIDTH)
    };
    Chunk* chunk = chunk_manager_get_chunk(chunkPos);

    Vector2u relPos = {
        .x = ((position.x % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH,
        .y = ((position.y % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH
    };

    BlockInstance* inst = chunk_get_block_ptr(chunk, relPos, layer);
    if (!inst) return false;
    BlockRegistry* brg = br_get_block_registry(inst->id);
    if (!brg) return false;

	// If the block has an interact callback, call it
    if (brg->interact_callback) {
        bool val = brg->interact_callback(
            (BlockExtraResult) {
                .block = inst,
                .reg = brg,
                .chunk = chunk,
                .position = relPos,
                .idx = relPos.x + (relPos.y * CHUNK_WIDTH)
            },
            holdingItem
        );
        if (val) {
            chunk_manager_update_lighting();
            return true;
        }
        else { return false; }
    }
    return false;
}

void chunk_manager_set_block_safe(Vector2i position, BlockInstance blockValue, ChunkLayerEnum layer) {
    if (!initialized) return;

    Vector2i chunkPos = {
        (int)floorf((float)position.x / (float)CHUNK_WIDTH),
        (int)floorf((float)position.y / (float)CHUNK_WIDTH)
    };
    Chunk* chunk = chunk_manager_get_chunk(chunkPos);

    Vector2u relPos = {
        .x = ((position.x % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH,
        .y = ((position.y % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH
    };

    if (blockValue.id > 0) {
		BlockRegistry* current_br = br_get_block_registry(chunk_get_block(chunk, relPos, layer).id);
        if (current_br->flags & BLOCK_FLAG_REPLACEABLE) {
            bool canPlace = false;

            BlockInstance neighbors[4];

            chunk_get_block_neighbors(chunk, relPos, layer, neighbors);

            for (int i = 0; i < 4; i++) {
                if (neighbors[i].id > 0) {
                    canPlace = true;
                    break;
                }
            }

            if (layer == CHUNK_LAYER_BACKGROUND) {
                chunk_get_block_neighbors(chunk, relPos, CHUNK_LAYER_FOREGROUND, neighbors);

                for (int i = 0; i < 4; i++) {
                    if (neighbors[i].id > 0) {
                        canPlace = true;
                        break;
                    }
                }
            }

            if (layer == CHUNK_LAYER_FOREGROUND) {
                BlockRegistry* br = br_get_block_registry(chunk_get_block(chunk, relPos, CHUNK_LAYER_BACKGROUND).id);
                if (br->flags & BLOCK_FLAG_SOLID) canPlace = true;
            } else {
                BlockRegistry* br = br_get_block_registry(chunk_get_block(chunk, relPos, CHUNK_LAYER_FOREGROUND).id);
                if (br->flags & BLOCK_FLAG_SOLID) canPlace = true;
            }

            if (canPlace) {
                chunk_set_block(chunk, relPos, blockValue, layer, true);
            }
        }
    }
    else {
        BlockInstance current = chunk_get_block(chunk, relPos, layer);
        if (current.id > 0) {
            chunk_set_block(chunk, relPos, blockValue, layer, true);
        }
    }
}

Chunk* chunk_manager_get_chunk(Vector2i position) {
    if (!initialized) return NULL;

    Vector2i localChunkPos = {
        position.x - (currentChunkPos.x - (chunk_view_width / 2)),
        position.y - (currentChunkPos.y - (chunk_view_height / 2))
    };

    if (localChunkPos.x >= 0 && localChunkPos.x < chunk_view_width && localChunkPos.y >= 0 && localChunkPos.y < chunk_view_height) {
        int chunkIndex = localChunkPos.y * chunk_view_width + localChunkPos.x;
        return &chunks[chunkIndex];
    }
    else {
        return NULL;
    }
}

void chunk_manager_set_block(Vector2i position, BlockInstance blockValue, ChunkLayerEnum layer) {
    if (!initialized) return;

    Vector2i chunkPos = {
        (int)floorf((float)position.x / (float)CHUNK_WIDTH),
        (int)floorf((float)position.y / (float)CHUNK_WIDTH)
    };

    Vector2i localChunkPos = {
        chunkPos.x - (currentChunkPos.x - chunk_view_width / 2),
        chunkPos.y - (currentChunkPos.y - chunk_view_height / 2)
    };

    if (localChunkPos.x >= 0 && localChunkPos.x < chunk_view_width && localChunkPos.y >= 0 && localChunkPos.y < chunk_view_height) {
        int chunkIndex = localChunkPos.y * chunk_view_width + localChunkPos.x;

        chunk_set_block(
            &chunks[chunkIndex],
            (Vector2u){
                .x = ((position.x % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH,
                .y = ((position.y % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH
            },
            blockValue,
            layer,
            true
        );
    }
}

BlockInstance chunk_manager_get_block(Vector2i position, ChunkLayerEnum layer) {
	if (!initialized) return (BlockInstance) { 0, 0, NULL };

    Vector2i chunkPos = {
        (int)floorf((float)position.x / (float)CHUNK_WIDTH),
        (int)floorf((float)position.y / (float)CHUNK_WIDTH)
    };

    Vector2i localChunkPos = {
        chunkPos.x - (currentChunkPos.x - chunk_view_width / 2),
        chunkPos.y - (currentChunkPos.y - chunk_view_height / 2)
    };

    if (localChunkPos.x >= 0 && localChunkPos.x < chunk_view_width && localChunkPos.y >= 0 && localChunkPos.y < chunk_view_height) {
        int chunkIndex = localChunkPos.y * chunk_view_width + localChunkPos.x;

        return chunk_get_block(
            &chunks[chunkIndex],
            (Vector2u){
                .x = ((position.x % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH,
                .y = ((position.y % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH
            },
            layer
        );
    }
    return (BlockInstance) { 0, 0, NULL };
}

uint8_t chunk_manager_get_light(Vector2i position) {
	if (!initialized) return 0;

    Vector2i chunkPos = {
        (int)floorf((float)position.x / (float)CHUNK_WIDTH),
        (int)floorf((float)position.y / (float)CHUNK_WIDTH)
    };

    Vector2i localChunkPos = {
        chunkPos.x - (currentChunkPos.x - chunk_view_width / 2),
        chunkPos.y - (currentChunkPos.y - chunk_view_height / 2)
    };

    if (localChunkPos.x >= 0 && localChunkPos.x < chunk_view_width && localChunkPos.y >= 0 && localChunkPos.y < chunk_view_height) {
        int chunkIndex = localChunkPos.y * chunk_view_width + localChunkPos.x;

        return chunk_get_light(
            &chunks[chunkIndex],
            (Vector2u){
                .x = ((position.x % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH,
                .y = ((position.y % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH
            }
        );
    }
    return 0;
}

void chunk_manager_set_light(Vector2i position, uint8_t value) {
    if (!initialized) return;

    Vector2i chunkPos = {
        (int)floorf((float)position.x / (float)CHUNK_WIDTH),
        (int)floorf((float)position.y / (float)CHUNK_WIDTH)
    };

    Vector2i localChunkPos = {
        chunkPos.x - (currentChunkPos.x - chunk_view_width / 2),
        chunkPos.y - (currentChunkPos.y - chunk_view_height / 2)
    };

    if (localChunkPos.x >= 0 && localChunkPos.x < chunk_view_width && localChunkPos.y >= 0 && localChunkPos.y < chunk_view_height) {
        int chunkIndex = localChunkPos.y * chunk_view_width + localChunkPos.x;

        chunk_set_light(
            &chunks[chunkIndex],
            (Vector2u) {
                .x = ((position.x % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH,
                .y = ((position.y % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH
            },
            value
        );
    }
}