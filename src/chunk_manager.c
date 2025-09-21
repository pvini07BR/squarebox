#include "chunk_manager.h"
#include "block_registry.h"
#include "chunk.h"
#include "types.h"

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

static size_t chunk_view_width = 5;
static size_t chunk_view_height = 3;
static size_t chunk_count = 0;

static Chunk* chunks = NULL;
static Vector2i currentChunkPos = { 0, 0 };

static unsigned int tick_counter = 0;

bool drawChunkLines = true;

void chunk_manager_init(size_t chunk_view_width, size_t chunk_view_height) {
    chunk_manager_set_view(chunk_view_width, chunk_view_height);
}

void chunk_manager_draw() {
    for (int i = 0; i < chunk_count; i++) {
        chunk_draw(&chunks[i]);
    }

    if (drawChunkLines) {
        for (int i = 0; i < chunk_count; i++) {
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
    for (int i = 0; i < chunk_count; i++) {
        chunk_draw_liquids(&chunks[i]);
    }
}

void chunk_manager_tick() {
    for (int i = 0; i < chunk_count; i++) {
		chunk_tick(&chunks[i], tick_counter);
    }

    tick_counter++;
}

void chunk_manager_free() {
    if (chunks) {
        for (int c = 0; c < chunk_count; c++) {
            chunk_free(&chunks[c]);
        }
        free(chunks);
        chunks = NULL;
    }
}

void chunk_manager_set_view(size_t new_view_width, size_t new_view_height) {
    if (chunks == NULL) {
        chunk_view_width = new_view_width;
        chunk_view_height = new_view_height;
        chunk_count = chunk_view_width * chunk_view_height;

        chunks = (Chunk*)malloc(sizeof(Chunk) * chunk_count);
        if (!chunks) {
            fprintf(stderr, "[ERROR] Failed to allocate memory for the Chunks.\n");
            return;
        }

        for (int c = 0; c < chunk_count; c++) {
            chunk_init(&chunks[c]);
            chunks[c].position = (Vector2i){ .x = INT_MAX, .y = INT_MAX };
        }
    }
    else {
        for (int c = 0; c < chunk_count; c++) {
            chunks[c].position = (Vector2i){ .x = INT_MAX, .y = INT_MAX };
            chunk_free(&chunks[c]);
        }

        chunk_view_width = new_view_width;
        chunk_view_height = new_view_height;
        chunk_count = chunk_view_width * chunk_view_height;

        Chunk* tempChunksPtr = (Chunk*)realloc(chunks, sizeof(Chunk) * chunk_count);
        if (!tempChunksPtr) {
            fprintf(stderr, "[ERROR] Failed to reallocate the Chunks.\n");
            return;
        }
        chunks = tempChunksPtr;

        for (int c = 0; c < chunk_count; c++) {
            chunk_init(&chunks[c]);
            chunks[c].position = (Vector2i){ .x = INT_MAX, .y = INT_MAX };
        }
    }

    chunk_manager_relocate(currentChunkPos);
}

void chunk_manager_reload_chunks()
{
    for (int c = 0; c < chunk_count; c++) {
        chunks[c].position = (Vector2i){ .x = INT_MAX, .y = INT_MAX };
        chunk_free(&chunks[c]);
    }

    chunk_manager_relocate(currentChunkPos);
}

void chunk_manager_relocate(Vector2i newCenter) {
    currentChunkPos = newCenter;

    int startChunkX = currentChunkPos.x - chunk_view_width / 2;
    int startChunkY = currentChunkPos.y - chunk_view_height / 2;

    Chunk* tempChunks = calloc(chunk_count, sizeof(Chunk));
    if (!tempChunks) {
        printf("[ERROR] Could not allocate memory for tempChunks.\n");
        return;
    };

    bool* oldUsed = calloc(chunk_count, sizeof(bool));
    for (int i = 0; i < chunk_count; ++i) oldUsed[i] = false;

    bool* isNew = calloc(chunk_count, sizeof(bool));
    for (int i = 0; i < chunk_count; ++i) isNew[i] = false;

    for (int i = 0; i < chunk_count; ++i) {
        int x = i % chunk_view_width;
        int y = i / chunk_view_width;
        Vector2i newPos = { startChunkX + x, startChunkY + y };

        bool found = false;
        for (int j = 0; j < chunk_count; ++j) {
            if (!oldUsed[j] && chunks[j].position.x == newPos.x && chunks[j].position.y == newPos.y) {
                tempChunks[i] = chunks[j];
                oldUsed[j] = true;
                found = true;
                isNew[i] = false;
                break;
            }
        }

        if (!found) {
            memset(&tempChunks[i], 0, sizeof(Chunk));
            tempChunks[i].position = newPos;
            isNew[i] = true;
        }

        tempChunks[i].neighbors.up = NULL;
        tempChunks[i].neighbors.right = NULL;
        tempChunks[i].neighbors.down = NULL;
        tempChunks[i].neighbors.left = NULL;
        tempChunks[i].neighbors.upLeft = NULL;
        tempChunks[i].neighbors.upRight = NULL;
        tempChunks[i].neighbors.downLeft = NULL;
        tempChunks[i].neighbors.downRight = NULL;
    }

    for (int j = 0; j < chunk_count; ++j) {
        if (!oldUsed[j]) {
            chunk_free(&chunks[j]);
        }
    }

    for (int i = 0; i < chunk_count; ++i) {
        chunks[i] = tempChunks[i];
    }

    for (int i = 0; i < chunk_count; ++i) {
        int x = i % chunk_view_width;
        int y = i / chunk_view_width;

        chunks[i].neighbors.up = (y > 0) ? &chunks[(y - 1) * chunk_view_width + x] : NULL;
        chunks[i].neighbors.right = (x < chunk_view_width - 1) ? &chunks[y * chunk_view_width + (x + 1)] : NULL;
        chunks[i].neighbors.down = (y < chunk_view_height - 1) ? &chunks[(y + 1) * chunk_view_width + x] : NULL;
        chunks[i].neighbors.left = (x > 0) ? &chunks[y * chunk_view_width + (x - 1)] : NULL;

        chunks[i].neighbors.upLeft = (x > 0 && y > 0) ? &chunks[(y - 1) * chunk_view_width + (x - 1)] : NULL;
        chunks[i].neighbors.upRight = (x < chunk_view_width - 1 && y > 0) ? &chunks[(y - 1) * chunk_view_width + (x + 1)] : NULL;
        chunks[i].neighbors.downLeft = (x > 0 && y < chunk_view_height - 1) ? &chunks[(y + 1) * chunk_view_width + (x - 1)] : NULL;
        chunks[i].neighbors.downRight = (x < chunk_view_width - 1 && y < chunk_view_height - 1) ? &chunks[(y + 1) * chunk_view_width + (x + 1)] : NULL;
    }

    for (int i = 0; i < chunk_count; i++) {
        if (isNew[i]) {
            chunk_init(&chunks[i]);
            chunk_regenerate(&chunks[i]);
        }
    }

    chunk_manager_update_lighting();

    free(tempChunks);
    free(oldUsed);
    free(isNew);
}

void chunk_manager_update_lighting() {
    for (int c = 0; c < chunk_count; c++) {
        for (int i = 0; i < CHUNK_AREA; i++) chunks[c].light[i] = 0;
    }

    for (int c = 0; c < chunk_count; c++) {
        for (int i = 0; i < CHUNK_AREA; i++) {
            BlockInstance b = chunks[c].blocks[i];
            BlockInstance w = chunks[c].walls[i];

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

    for (int c = 0; c < chunk_count; c++) chunk_genmesh(&chunks[c]);
}

size_t chunk_manager_get_view_width()
{
    return chunk_view_width;
}

size_t chunk_manager_get_view_height()
{
    return chunk_view_height;
}

bool chunk_manager_interact(Vector2i position, bool isWall) {
    Vector2i chunkPos = {
        (int)floorf((float)position.x / (float)CHUNK_WIDTH),
        (int)floorf((float)position.y / (float)CHUNK_WIDTH)
    };
    Chunk* chunk = chunk_manager_get_chunk(chunkPos);

    Vector2u relPos = {
        .x = ((position.x % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH,
        .y = ((position.y % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH
    };

    BlockInstance* inst = chunk_get_block_ptr(chunk, relPos, isWall);
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
            }
        );
        if (val) {
            chunk_manager_update_lighting();
            return true;
        }
        else { return false; }
    }
    return false;
}

void chunk_manager_set_block_safe(Vector2i position, BlockInstance blockValue, bool isWall) {
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
		BlockRegistry* current_br = br_get_block_registry(chunk_get_block(chunk, relPos, isWall).id);
        if (current_br->flags & BLOCK_FLAG_REPLACEABLE) {
            bool canPlace = false;

            BlockInstance neighbors[4];

            chunk_get_block_neighbors(chunk, relPos, isWall, neighbors);

            for (int i = 0; i < 4; i++) {
                if (neighbors[i].id > 0) {
                    canPlace = true;
                    break;
                }
            }

            if (!isWall) {
                BlockRegistry* br = br_get_block_registry(chunk_get_block(chunk, relPos, true).id);
                if (br->flags & BLOCK_FLAG_SOLID) canPlace = true;
            } else {
                BlockRegistry* br = br_get_block_registry(chunk_get_block(chunk, relPos, false).id);
                if (br->flags & BLOCK_FLAG_SOLID) canPlace = true;
            }

            if (canPlace) {
                chunk_set_block(chunk, relPos, blockValue, isWall, true);
            }
        }
    }
    else {
        BlockInstance current = chunk_get_block(chunk, relPos, isWall);
        if (current.id > 0) {
            chunk_set_block(chunk, relPos, blockValue, isWall, true);
        }
    }
}

Chunk* chunk_manager_get_chunk(Vector2i position) {
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

void chunk_manager_set_block(Vector2i position, BlockInstance blockValue, bool isWall) {
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
            isWall,
            true
        );
    }
}

BlockInstance chunk_manager_get_block(Vector2i position, bool isWall) {
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
            isWall
        );
    }
    return (BlockInstance) { 0, 0 };
}

uint8_t chunk_manager_get_light(Vector2i position) {
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