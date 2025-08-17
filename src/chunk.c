#include "chunk.h"
#include "chunk_manager.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

void chunk_set_block(Chunk* chunk, Vector2u position, int blockValue, bool isWall) {
    if (!chunk) return;
    if (position.x > CHUNK_WIDTH || position.y > CHUNK_WIDTH) return;

    if (!isWall)
        chunk->blocks[position.x + (position.y * CHUNK_WIDTH)] = blockValue;
    else
        chunk->walls[position.x + (position.y * CHUNK_WIDTH)] = blockValue;
}

int chunk_get_block(Chunk* chunk, Vector2u position, bool isWall) {
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

void chunk_regenerate(Chunk* chunk) {
    if (!chunk) return;

    unsigned int seed = (unsigned int)(chunk->position.x * 73856093 ^ chunk->position.y * 19349663);
    srand(seed);
    for (int i = 0; i < CHUNK_AREA; i++) {
        // World generation
        Vector2i globalBlockPos = {
            chunk->position.x * CHUNK_WIDTH + (i % CHUNK_WIDTH),
            chunk->position.y * CHUNK_WIDTH + (i / CHUNK_WIDTH)
        };

        int sin = (int)floorf(sinf((float)globalBlockPos.x / CHUNK_WIDTH) * CHUNK_WIDTH);

        if (globalBlockPos.y == sin) {
            chunk->blocks[i] = 1;
        } else if (globalBlockPos.y > sin) {
            chunk->blocks[i] = 2;
        } else {
            chunk->blocks[i] = 0;
        }
        chunk->walls[i] = 0;
        chunk->light[i] = 15;
    }
}

void chunk_calculate_lighting(Chunk* chunk) {
    if (!chunk) return;

    for (int i = 0; i < CHUNK_AREA; i++) {
        chunk->light[i] = 15;
    }

    /*
    for (int i = 0; i < CHUNK_AREA; i++) {
        if (chunk->blocks[i] == 0 && chunk->walls[i] == 0) {
            chunk->light[i] = 255;
        } else {
            chunk->light[i] = 0;
        }
    }

    for (int i = 0; i < CHUNK_AREA; i++) {
        if (chunk->blocks[i] == 0 && chunk->walls[i] == 0) {
            chunk->light[i] = 255;
        } else {
            Vector2i globalBlockPos = {
                chunk->position.x * CHUNK_WIDTH + (i % CHUNK_WIDTH),
                chunk->position.y * CHUNK_WIDTH + (i / CHUNK_WIDTH)
            };

            uint8_t neighborLightValues[4] = {
                chunk_manager_get_light((Vector2i){ globalBlockPos.x + 1, globalBlockPos.y + 0 }),
                chunk_manager_get_light((Vector2i){ globalBlockPos.x - 1, globalBlockPos.y + 0 }),
                chunk_manager_get_light((Vector2i){ globalBlockPos.x + 0, globalBlockPos.y + 1 }),
                chunk_manager_get_light((Vector2i){ globalBlockPos.x + 0, globalBlockPos.y - 1 }),
            };

            uint8_t maxLightValue = fmax(
                fmax(neighborLightValues[0], neighborLightValues[1]),
                fmax(neighborLightValues[2], neighborLightValues[3])
            );

            if (maxLightValue > 0 && chunk->light[i] > 0) chunk->light[i]--;
        }
    }
    */
}

void chunk_draw(Chunk* chunk, Texture2D* blocksAtlas) {
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

        Vector2 pos = { (float)x, (float)y };

        Rectangle textureRect = {
            .x = (float)(chunk->blocks[j] - 1) * TILE_SIZE,
            .y = 0.0f,
            .width = (float)TILE_SIZE,
            .height = (float)TILE_SIZE
        };

        Color blockTint = {  
            .r = (unsigned char)(((float)chunk->light[j] / 15.0f) * 255.0f),
            .g = (unsigned char)(((float)chunk->light[j] / 15.0f) * 255.0f),
            .b = (unsigned char)(((float)chunk->light[j] / 15.0f) * 255.0f),
            .a = 255
        };

        if (chunk->walls[j] > 0) {
            textureRect.x = (float)(chunk->walls[j] - 1) * TILE_SIZE;
            Color wallTint = {
                blockTint.r / 2,
                blockTint.g / 2,
                blockTint.b / 2,
                blockTint.a
            };

            if (blocksAtlas) {
                DrawTextureRec(
                    *blocksAtlas,
                    textureRect,
                    pos,
                    wallTint
                );
            } else {
                DrawRectangle(
                    x,
                    y,
                    TILE_SIZE,
                    TILE_SIZE,
                    wallTint
                );
            }
        }

        if (chunk->blocks[j] > 0) {
            if (blocksAtlas) {
                DrawTextureRec(
                    *blocksAtlas,
                    textureRect,
                    pos,
                    blockTint
                );
            } else {
                DrawRectangle(
                    x,
                    y,
                    TILE_SIZE,
                    TILE_SIZE,
                    blockTint
                );
            }
        }
    }

    DrawRectangleLines(
        0,
        0,
        CHUNK_WIDTH * TILE_SIZE,
        CHUNK_WIDTH * TILE_SIZE,
        WHITE
    );

    rlPopMatrix();
}