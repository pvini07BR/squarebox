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

#define BLOCK_IS_SOLID_DARK(i) \
    (!(registries[i]->transparent) && (registries[i]->lightLevel <= 0))

unsigned int posmod(int v, int m) {
    int r = v % m;
    return (unsigned int)(r < 0 ? r + m : r);
}

void chunk_fill_light(Chunk* chunk, Vector2u startPoint, uint8_t newLightValue) {
    if (!chunk) return;
    if (newLightValue < 1 || newLightValue > 15) return;
    if (startPoint.x < 0 || startPoint.x >= CHUNK_WIDTH) return;
    if (startPoint.y < 0 || startPoint.y >= CHUNK_WIDTH) return;

    uint8_t current = chunk_get_light(chunk, startPoint);
    if (current >= newLightValue) return;

    chunk_set_light(chunk, startPoint, newLightValue);

    uint8_t decayAmount = 1;

    BlockRegistry* br = block_registry_get_block_registry(chunk_get_block(chunk, startPoint, false));
    if (!br->transparent) decayAmount = 4;

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

void chunk_calc_lighting(Chunk* chunk) {
    for (int i = 0; i < CHUNK_AREA; i++) chunk->light[i] = 0;
    
    for (int i = 0; i < CHUNK_AREA; i++) {
        if (chunk->blocks[i] == 0 && chunk->walls[i] == 0) {
            chunk_fill_light(chunk, (Vector2u) { (i % CHUNK_WIDTH), (i / CHUNK_WIDTH) }, 15);
        }
    }
}

void chunk_set_block(Chunk* chunk, Vector2u position, uint8_t blockValue, bool isWall) {
    if (!chunk) return;
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return;

    if (!isWall)
        chunk->blocks[position.x + (position.y * CHUNK_WIDTH)] = blockValue;
    else
        chunk->walls[position.x + (position.y * CHUNK_WIDTH)] = blockValue;
}

uint8_t chunk_get_block(Chunk* chunk, Vector2u position, bool isWall) {
    if (!chunk) return 0;
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return 0;
    if (!isWall)
        return chunk->blocks[position.x + (position.y * CHUNK_WIDTH)];
    else
        return chunk->walls[position.x + (position.y * CHUNK_WIDTH)];
}

void chunk_set_light(Chunk* chunk, Vector2u position, uint8_t value) {
    if (!chunk) return;
    if (position.x < 0 || position.x >= CHUNK_WIDTH || position.y < 0 || position.y >= CHUNK_WIDTH) return;

    chunk->light[position.x + (position.y * CHUNK_WIDTH)] = value;
}

uint8_t chunk_get_light(Chunk* chunk, Vector2u position) {
    if (!chunk) return 0;
    if (position.x >= CHUNK_WIDTH || position.y >= CHUNK_WIDTH) return 0;
    return chunk->light[position.x + (position.y * CHUNK_WIDTH)];
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
            chunk->walls[i] = 1;
        } else if (globalBlockPos.y > sin && globalBlockPos.y <= 50) {
            chunk->blocks[i] = 2;
            chunk->walls[i] = 2;
        } else if (globalBlockPos.y > 50) {
            chunk->blocks[i] = 3;
            chunk->walls[i] = 3;
        }
        else {
            chunk->blocks[i] = 0;
            chunk->walls[i] = 0;
        }
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

            if (brg->lightLevel <= 0) {
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

                if (BLOCK_IS_SOLID_DARK(0)) topLeft = topRight = fadeColor;
                if (BLOCK_IS_SOLID_DARK(1)) topRight = bottomRight = fadeColor;
                if (BLOCK_IS_SOLID_DARK(2)) bottomLeft = bottomRight = fadeColor;
                if (BLOCK_IS_SOLID_DARK(3)) topLeft = bottomLeft = fadeColor;

                if (BLOCK_IS_SOLID_DARK(4)) topLeft = fadeColor;
                if (BLOCK_IS_SOLID_DARK(5)) topRight = fadeColor;
                if (BLOCK_IS_SOLID_DARK(6)) bottomLeft = fadeColor;
                if (BLOCK_IS_SOLID_DARK(7)) bottomRight = fadeColor;

                DrawRectangleGradientEx(
                    blockRect,
                    topLeft,
                    bottomLeft,
                    bottomRight,
                    topRight
                );
            }
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

        if (chunk->blocks[j] > 0 || chunk->walls[j] > 0) {
            unsigned char value = (unsigned char)(((float)chunk->light[j] / 15.0f) * 255.0f);
            Color lightColor = {
                .r = 0,
                .g = 0,
                .b = 0,
                .a = 255 - value
            };
            DrawRectangle(
                x * TILE_SIZE,
                y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE,
                lightColor
            );
        }
    }

    rlPopMatrix();
}

unsigned int chunk_get_block_seed(Chunk* chunk, Vector2u position, bool isWall) {
    unsigned int h = chunk->seed;
    h ^= position.x * 374761393u;
    h ^= position.y * 668265263u;
    h ^= (unsigned int)isWall * 1442695040888963407ull;
    h = (h ^ (h >> 13)) * 1274126177u;
    return h;
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