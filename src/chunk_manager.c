#include "chunk_manager.h"
#include "chunk.h"
#include "block_registry.h"
#include "light_queue.h"

#include <stdint.h>
#include <stdlib.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

static Chunk* chunks = NULL;
static Material chunkMaterial;
static Texture2D lightMap;
static Vector2i currentChunkPos;

void chunk_manager_init() {
    chunks = (Chunk*)malloc(CHUNK_COUNT * sizeof(Chunk));
    chunkMaterial = LoadMaterialDefault();
    SetMaterialTexture(&chunkMaterial, MATERIAL_MAP_ALBEDO, *br_get_block_atlas());

    for (int c = 0; c < CHUNK_COUNT; c++) chunks[c].initializedMeshes = false;
    chunk_manager_relocate((Vector2i) { 0, 0 });
}

void chunk_manager_draw() {
    rlDisableBackfaceCulling();

    for (int i = 0; i < CHUNK_COUNT; i++) {
        chunk_draw(&chunks[i], &chunkMaterial);
    }

    DrawTextureEx(
        lightMap,
        (Vector2) {
            (currentChunkPos.x - (CHUNK_VIEW_WIDTH / 2)) * CHUNK_WIDTH * TILE_SIZE,
            (currentChunkPos.y - (CHUNK_VIEW_HEIGHT / 2)) * CHUNK_WIDTH * TILE_SIZE
        },
        0.0f,
        TILE_SIZE,
        WHITE
    );

    for (int i = 0; i < CHUNK_COUNT; i++) {
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

void chunk_manager_free() {
    UnloadTexture(lightMap);
    if (chunks) {
        for (int c = 0; c < CHUNK_COUNT; c++) chunk_free_meshes(&chunks[c]);
        free(chunks);
        chunks = NULL;
    }
}


void chunk_manager_relocate(Vector2i newCenter) {
    currentChunkPos = newCenter;

    int startChunkX = currentChunkPos.x - CHUNK_VIEW_WIDTH / 2;
    int startChunkY = currentChunkPos.y - CHUNK_VIEW_HEIGHT / 2;

    Chunk tempChunks[CHUNK_COUNT] = { 0 };

    bool chunkUsed[CHUNK_COUNT] = { false };

    for (int i = 0; i < CHUNK_COUNT; i++) {
        int x = i % CHUNK_VIEW_WIDTH;
        int y = i / CHUNK_VIEW_WIDTH;

        Vector2i newPos = { startChunkX + x, startChunkY + y };

        bool found = false;
        for (int j = 0; j < CHUNK_COUNT; j++) {
            if (chunks[j].position.x == newPos.x && chunks[j].position.y == newPos.y) {
                tempChunks[i] = chunks[j];
                chunkUsed[j] = true;
                found = true;
                break;
            }
        }

        tempChunks[i].neighbors.up = NULL;
        tempChunks[i].neighbors.right = NULL;
        tempChunks[i].neighbors.down = NULL;
        tempChunks[i].neighbors.left = NULL;
        tempChunks[i].neighbors.upLeft = NULL;
        tempChunks[i].neighbors.upRight = NULL;
        tempChunks[i].neighbors.downLeft = NULL;
        tempChunks[i].neighbors.downRight = NULL;

        if (!found) {
            tempChunks[i].initializedMeshes = false;
            tempChunks[i].position = newPos;
            chunk_init_meshes(&tempChunks[i]);
            chunk_regenerate(&tempChunks[i]);
        }
    }

    for (int i = 0; i < CHUNK_COUNT; i++) {
        if (!chunkUsed[i] && chunks[i].initializedMeshes) {
            chunk_free_meshes(&chunks[i]);
        }
    }

    for (int i = 0; i < CHUNK_COUNT; i++) {
        chunks[i] = tempChunks[i];
    }

    for (int i = 0; i < CHUNK_COUNT; i++) {
        int x = i % CHUNK_VIEW_WIDTH;
        int y = i / CHUNK_VIEW_WIDTH;

        if (y > 0) {
            int upIndex = (y - 1) * CHUNK_VIEW_WIDTH + x;
            chunks[i].neighbors.up = &chunks[upIndex];
        }

        if (x < CHUNK_VIEW_WIDTH - 1) {
            int rightIndex = y * CHUNK_VIEW_WIDTH + (x + 1);
            chunks[i].neighbors.right = &chunks[rightIndex];
        }

        if (y < CHUNK_VIEW_HEIGHT - 1) {
            int downIndex = (y + 1) * CHUNK_VIEW_WIDTH + x;
            chunks[i].neighbors.down = &chunks[downIndex];
        }

        if (x > 0) {
            int leftIndex = y * CHUNK_VIEW_WIDTH + (x - 1);
            chunks[i].neighbors.left = &chunks[leftIndex];
        }

        if (x > 0 && y > 0) {
            int upLeftIndex = (y - 1) * CHUNK_VIEW_WIDTH + (x - 1);
            chunks[i].neighbors.upLeft = &chunks[upLeftIndex];
        }

        if (x < CHUNK_VIEW_WIDTH - 1 && y > 0) {
            int upRightIndex = (y - 1) * CHUNK_VIEW_WIDTH + (x + 1);
            chunks[i].neighbors.upRight = &chunks[upRightIndex];
        }

        if (x > 0 && y < CHUNK_VIEW_HEIGHT - 1) {
            int downLeftIndex = (y + 1) * CHUNK_VIEW_WIDTH + (x - 1);
            chunks[i].neighbors.downLeft = &chunks[downLeftIndex];
        }

        if (x < CHUNK_VIEW_WIDTH - 1 && y < CHUNK_VIEW_HEIGHT - 1) {
            int downRightIndex = (y + 1) * CHUNK_VIEW_WIDTH + (x + 1);
            chunks[i].neighbors.downRight = &chunks[downRightIndex];
        }
    }

    chunk_manager_update_lighting();
}

void chunk_manager_update_lighting() {
    for (int c = 0; c < CHUNK_COUNT; c++) {
        for (int i = 0; i < CHUNK_AREA; i++) chunks[c].light[i] = 0;
    }

    for (int c = 0; c < CHUNK_COUNT; c++) {
        for (int i = 0; i < CHUNK_AREA; i++) {
            uint8_t b = chunks[c].blocks[i];
            uint8_t w = chunks[c].walls[i];

            BlockRegistry* bbr = br_get_block_registry(b);
            BlockRegistry* wbr = br_get_block_registry(w);

            int x = i % CHUNK_WIDTH;
            int y = i / CHUNK_WIDTH;

            if (bbr->transparent && wbr->transparent) {
                chunk_fill_light(&chunks[c], (Vector2u) { x, y }, 15);
            }
            else if (bbr->lightLevel > 0 || wbr->lightLevel > 0) {
                uint8_t maxLight = fmax(bbr->lightLevel, wbr->lightLevel);
                chunk_fill_light(&chunks[c], (Vector2u) { x, y }, maxLight);
            }
        }
    }

    for (int c = 0; c < CHUNK_COUNT; c++) chunk_genmesh(&chunks[c]);
}

Chunk* chunk_manager_get_chunk(Vector2i position) {
    Vector2i localChunkPos = {
        position.x - (currentChunkPos.x - (CHUNK_VIEW_WIDTH / 2)),
        position.y - (currentChunkPos.y - (CHUNK_VIEW_HEIGHT / 2))
    };

    if (localChunkPos.x >= 0 && localChunkPos.x < CHUNK_VIEW_WIDTH && localChunkPos.y >= 0 && localChunkPos.y < CHUNK_VIEW_HEIGHT) {
        int chunkIndex = localChunkPos.y * CHUNK_VIEW_WIDTH + localChunkPos.x;
        return &chunks[chunkIndex];
    }
    else {
        return NULL;
    }
}

void chunk_manager_set_block(Vector2i position, int blockValue, bool isWall) {
    Vector2i chunkPos = {
        (int)floorf((float)position.x / (float)CHUNK_WIDTH),
        (int)floorf((float)position.y / (float)CHUNK_WIDTH)
    };

    Vector2i localChunkPos = {
        chunkPos.x - (currentChunkPos.x - CHUNK_VIEW_WIDTH / 2),
        chunkPos.y - (currentChunkPos.y - CHUNK_VIEW_HEIGHT / 2)
    };

    if (localChunkPos.x >= 0 && localChunkPos.x < CHUNK_VIEW_WIDTH && localChunkPos.y >= 0 && localChunkPos.y < CHUNK_VIEW_HEIGHT) {
        int chunkIndex = localChunkPos.y * CHUNK_VIEW_WIDTH + localChunkPos.x;

        chunk_set_block(
            &chunks[chunkIndex],
            (Vector2u){
                .x = ((position.x % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH,
                .y = ((position.y % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH
            },
            blockValue,
            isWall
        );
    }
    chunk_manager_update_lighting();
}

int chunk_manager_get_block(Vector2i position, bool isWall) {
    Vector2i chunkPos = {
        (int)floorf((float)position.x / (float)CHUNK_WIDTH),
        (int)floorf((float)position.y / (float)CHUNK_WIDTH)
    };

    Vector2i localChunkPos = {
        chunkPos.x - (currentChunkPos.x - CHUNK_VIEW_WIDTH / 2),
        chunkPos.y - (currentChunkPos.y - CHUNK_VIEW_HEIGHT / 2)
    };

    if (localChunkPos.x >= 0 && localChunkPos.x < CHUNK_VIEW_WIDTH && localChunkPos.y >= 0 && localChunkPos.y < CHUNK_VIEW_HEIGHT) {
        int chunkIndex = localChunkPos.y * CHUNK_VIEW_WIDTH + localChunkPos.x;

        return chunk_get_block(
            &chunks[chunkIndex],
            (Vector2u){
                .x = ((position.x % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH,
                .y = ((position.y % CHUNK_WIDTH) + CHUNK_WIDTH) % CHUNK_WIDTH
            },
            isWall
        );
    }
    return 0;
}

uint8_t chunk_manager_get_light(Vector2i position) {
    Vector2i chunkPos = {
        (int)floorf((float)position.x / (float)CHUNK_WIDTH),
        (int)floorf((float)position.y / (float)CHUNK_WIDTH)
    };

    Vector2i localChunkPos = {
        chunkPos.x - (currentChunkPos.x - CHUNK_VIEW_WIDTH / 2),
        chunkPos.y - (currentChunkPos.y - CHUNK_VIEW_HEIGHT / 2)
    };

    if (localChunkPos.x >= 0 && localChunkPos.x < CHUNK_VIEW_WIDTH && localChunkPos.y >= 0 && localChunkPos.y < CHUNK_VIEW_HEIGHT) {
        int chunkIndex = localChunkPos.y * CHUNK_VIEW_WIDTH + localChunkPos.x;

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
        chunkPos.x - (currentChunkPos.x - CHUNK_VIEW_WIDTH / 2),
        chunkPos.y - (currentChunkPos.y - CHUNK_VIEW_HEIGHT / 2)
    };

    if (localChunkPos.x >= 0 && localChunkPos.x < CHUNK_VIEW_WIDTH && localChunkPos.y >= 0 && localChunkPos.y < CHUNK_VIEW_HEIGHT) {
        int chunkIndex = localChunkPos.y * CHUNK_VIEW_WIDTH + localChunkPos.x;

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