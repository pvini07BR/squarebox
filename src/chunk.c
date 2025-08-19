#include "chunk.h"
#include "chunk_manager.h"
#include "block_registry.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

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
        int x = j % CHUNK_WIDTH * TILE_SIZE;
        int y = (j / CHUNK_WIDTH) % CHUNK_WIDTH * TILE_SIZE;

        Rectangle blockRect = {
            .x = x,
            .y = y,
            .width = TILE_SIZE,
            .height = TILE_SIZE
        };

        if (chunk->walls[j] > 0) {
            unsigned int seed = chunk_get_block_seed(chunk, (Vector2u) { x, y }, true);
            BlockRegistry* brg = block_registry_get_block_registry(chunk->walls[j]);
            Rectangle blockTextRect = block_registry_get_block_texture_rect(
                chunk->blocks[j],
                (seed & 1) ? true : false,
                (seed & 2) ? true : false
            );

            DrawTexturePro(
                *block_registry_get_block_atlas(),
                blockTextRect,
                blockRect,
                Vector2Zero(),
                0.0f,
                GRAY
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