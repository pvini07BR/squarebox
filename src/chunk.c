#include "chunk.h"
#include "chunk_manager.h"
#include "block_registry.h"
#include "light_queue.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

unsigned int posmod(int v, int m) {
    int r = v % m;
    return (unsigned int)(r < 0 ? r + m : r);
}

void chunk_set_block(Chunk* chunk, Vector2u position, uint8_t blockValue, bool isWall) {
    if (!chunk) return;
    if (position.x > CHUNK_WIDTH || position.y > CHUNK_WIDTH) return;

    if (!isWall)
        chunk->blocks[position.x + (position.y * CHUNK_WIDTH)] = blockValue;
    else
        chunk->walls[position.x + (position.y * CHUNK_WIDTH)] = blockValue;
}

uint8_t chunk_get_block(Chunk* chunk, Vector2u position, bool isWall) {
    if (!chunk) return 0;
    if (position.x > CHUNK_WIDTH || position.y > CHUNK_WIDTH) return 0;
    if (!isWall)
        return chunk->blocks[position.x + (position.y * CHUNK_WIDTH)];
    else
        return chunk->walls[position.x + (position.y * CHUNK_WIDTH)];
}


uint8_t chunk_get_light(Chunk* chunk, Vector2u position) {
    if (!chunk) return 0;
    if (position.x > CHUNK_WIDTH || position.y > CHUNK_WIDTH) return 0;
    return chunk->light[position.x + (position.y * CHUNK_WIDTH)];
}

unsigned int chunk_get_block_seed(Chunk* chunk, Vector2u position, bool isWall) {
    unsigned int h = chunk->seed;
    h ^= position.x * 374761393u;
    h ^= position.y * 668265263u;
    h ^= (unsigned int)isWall * 1442695040888963407ull;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h;
}


void chunk_set_light(Chunk* chunk, Vector2u position, uint8_t value) {
    if (!chunk) return;
    if (position.x > CHUNK_WIDTH || position.y > CHUNK_WIDTH) return;

    chunk->light[position.x + (position.y * CHUNK_WIDTH)] = value;
}

void chunk_regenerate(Chunk* chunk) {
    if (!chunk) return;

    chunk->seed = (unsigned int)(chunk->position.x * 73856093 ^ chunk->position.y * 19349663);
    srand(chunk->seed);
    for (int i = 0; i < CHUNK_AREA; i++) {
        // World generation
        Vector2i globalBlockPos = {
            chunk->position.x * CHUNK_WIDTH + (i % CHUNK_WIDTH),
            chunk->position.y * CHUNK_WIDTH + (i / CHUNK_WIDTH)
        };

        int sin = (int)floorf(sinf((float)globalBlockPos.x / CHUNK_WIDTH) * CHUNK_WIDTH);

        if (globalBlockPos.y == sin) {
            chunk->blocks[i] = 1;
        } else if (globalBlockPos.y > sin && globalBlockPos.y <= 50) {
            chunk->blocks[i] = 2;
        } else if (globalBlockPos.y > 50) {
            chunk->blocks[i] = 3;
        }
        else {
            chunk->blocks[i] = 0;
        }
        chunk->walls[i] = 0;
        chunk->light[i] = 0;
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

    for (int j = 0; j < CHUNK_AREA; j++) {
        const int x = j % CHUNK_WIDTH;
        const int y = (j / CHUNK_WIDTH) % CHUNK_WIDTH;

        const Rectangle blockRect = {
            .x = x * TILE_SIZE,
            .y = y * TILE_SIZE,
            .width = TILE_SIZE,
            .height = TILE_SIZE
        };

        if (chunk->walls[j] > 0) {
            const unsigned int seed = chunk_get_block_seed(chunk, (Vector2u) { x, y }, true);
            BlockRegistry* brg = block_registry_get_block_registry(chunk->walls[j]);
            Rectangle blockTextRect = block_registry_get_block_texture_rect(
                chunk->walls[j],
                brg->flipH && (seed & 1) ? true : false,
                brg->flipV && (seed & 2) ? true : false
            );

            DrawTexturePro(
                *block_registry_get_block_atlas(),
                blockTextRect,
                blockRect,
                Vector2Zero(),
                0.0f,
                GRAY
            );

            // Wall "ambient occlusion"
            uint8_t neighbors[8] = {
                chunk_get_block_extrapolating(chunk, (Vector2i) { x,     y - 1 }, false),   // Up
                chunk_get_block_extrapolating(chunk, (Vector2i) { x + 1, y     }, false),   // Right
                chunk_get_block_extrapolating(chunk, (Vector2i) { x,     y + 1 }, false),   // Down
                chunk_get_block_extrapolating(chunk, (Vector2i) { x - 1, y     }, false),   // Left

                chunk_get_block_extrapolating(chunk, (Vector2i) { x - 1,  y - 1 }, false),  // Up left
                chunk_get_block_extrapolating(chunk, (Vector2i) { x + 1,  y - 1 }, false),  // Up right
                chunk_get_block_extrapolating(chunk, (Vector2i) { x - 1,  y + 1 }, false),  // Down left
                chunk_get_block_extrapolating(chunk, (Vector2i) { x + 1,  y + 1 }, false),  // Down right
            };

            BlockRegistry* registries[8];
            for (int i = 0; i < 8; i++)
                registries[i] = block_registry_get_block_registry(neighbors[i]);

            const Color fadeColor = { 0, 0, 0, 128 };

            Color topLeft = { 0, 0, 0, 0 };
            Color bottomLeft = { 0, 0, 0, 0 };
            Color topRight = { 0, 0, 0, 0 };
            Color bottomRight = { 0, 0, 0, 0 };

            if (!registries[0]->transparent) topLeft = topRight = fadeColor;
            if (!registries[1]->transparent) topRight = bottomRight = fadeColor;
            if (!registries[2]->transparent) bottomLeft = bottomRight = fadeColor;
            if (!registries[3]->transparent) topLeft = bottomLeft = fadeColor;
                
            if (!registries[4]->transparent) topLeft = fadeColor;
            if (!registries[5]->transparent) topRight = fadeColor;
            if (!registries[6]->transparent) bottomLeft = fadeColor;
            if (!registries[7]->transparent) bottomRight = fadeColor;

            DrawRectangleGradientEx(
                blockRect,
                topLeft,
                bottomLeft,
                bottomRight,
                topRight
            );
        }

        if (chunk->blocks[j] > 0) {
            unsigned int seed = chunk_get_block_seed(chunk, (Vector2u) { x, y }, false);
            BlockRegistry* brg = block_registry_get_block_registry(chunk->blocks[j]);
            Rectangle blockTextRect = block_registry_get_block_texture_rect(
                chunk->blocks[j],
                brg->flipH && (seed & 1) ? true : false,
                brg->flipV && (seed & 2) ? true : false
            );

            DrawTexturePro(
                *block_registry_get_block_atlas(),
                blockTextRect,
                blockRect,
                Vector2Zero(),
                0.0f,
                WHITE
            );
        }
    }

    rlPopMatrix();
}

uint8_t chunk_get_block_extrapolating(Chunk* chunk, Vector2i position, bool isWall) {
    if (!chunk) return 0;

    if (position.x >= 0 && position.y >= 0 && position.x < CHUNK_WIDTH && position.y < CHUNK_WIDTH) {
        if (!isWall)
            return chunk->blocks[position.x + (position.y * CHUNK_WIDTH)];
        else
            return chunk->walls[position.x + (position.y * CHUNK_WIDTH)];
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

        if (neighbor == NULL) return 0;

        Vector2u relPos = {
            .x = posmod(position.x, CHUNK_WIDTH),
            .y = posmod(position.y, CHUNK_WIDTH)
        };

        if (!isWall)
            return neighbor->blocks[relPos.x + (relPos.y * CHUNK_WIDTH)];
        else
            return neighbor->walls[relPos.x + (relPos.y * CHUNK_WIDTH)];
    }
}

uint8_t chunk_get_light_extrapolating(Chunk* chunk, Vector2i position) {
    if (!chunk) return 0;

    if (position.x >= 0 && position.y >= 0 && position.x < CHUNK_WIDTH && position.y < CHUNK_WIDTH) {
        return chunk->light[position.x + (position.y * CHUNK_WIDTH)];
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

        if (neighbor == NULL) return 0;

        Vector2u relPos = {
            .x = posmod(position.x, CHUNK_WIDTH),
            .y = posmod(position.y, CHUNK_WIDTH)
        };

        return neighbor->light[relPos.x + (relPos.y * CHUNK_WIDTH)];
    }
}

void chunk_set_light_extrapolating(Chunk* chunk, Vector2i position, uint8_t lightValue,
    Chunk** affectedChunk, Vector2i* affectedPos) {
    if (!chunk) return;

    if (position.x >= 0 && position.y >= 0 && position.x < CHUNK_WIDTH && position.y < CHUNK_WIDTH) {
        chunk->light[position.x + (position.y * CHUNK_WIDTH)] = lightValue;
        *affectedChunk = chunk;
        *affectedPos = position;
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

        if (neighbor == NULL) {
            *affectedChunk = NULL;
            return;
        }

        Vector2i relPos = {
            .x = posmod(position.x, CHUNK_WIDTH),
            .y = posmod(position.y, CHUNK_WIDTH)
        };

        neighbor->light[relPos.x + (relPos.y * CHUNK_WIDTH)] = lightValue;
        *affectedChunk = neighbor;
        *affectedPos = relPos;
    }
}

bool chunk_is_transparent_extrapolating(Chunk* chunk, Vector2i position) {
    uint8_t block = chunk_get_block_extrapolating(chunk, position, false);
    uint8_t wall = chunk_get_block_extrapolating(chunk, position, true);

    BlockRegistry* block_reg = block_registry_get_block_registry(block);
    BlockRegistry* wall_reg = block_registry_get_block_registry(wall);

    return (block_reg->transparent && wall_reg->transparent);
}

void chunk_propagate_light_flood_fill(Chunk* startChunk, Vector2i startPos, uint8_t startLight) {
    if (!startChunk) return;
    if (!get_light_queue()) return;

    light_queue_push(startChunk, startPos, startLight);

    Vector2i directions[4] = {
        {-1, 0}, {1, 0}, {0, -1}, {0, 1}
    };

    while (get_light_queue()->size > 0) {
        LightNode current;
        if (!light_queue_pop(&current)) break;

        for (int i = 0; i < 4; i++) {
            Vector2i neighborPos = {
                current.localPosition.x + directions[i].x,
                current.localPosition.y + directions[i].y
            };

            if (!chunk_is_transparent_extrapolating(current.chunk, neighborPos)) {
                continue;
            }

            uint8_t newLightLevel = (current.lightLevel > 1) ? current.lightLevel - 1 : 0;
            if (newLightLevel == 0) continue;

            uint8_t existingLight = chunk_get_light_extrapolating(current.chunk, neighborPos);

            if (newLightLevel > existingLight) {
                Chunk* affectedChunk = NULL;
                Vector2i affectedPos;
                chunk_set_light_extrapolating(current.chunk, neighborPos, newLightLevel,
                    &affectedChunk, &affectedPos);

                if (affectedChunk) {
                    light_queue_push(affectedChunk, affectedPos, newLightLevel);
                }
            }
        }
    }
}