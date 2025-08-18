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

/*
void chunk_update_lightmap(Chunk* chunk) {
    Image img = GenImageColor(CHUNK_WIDTH, CHUNK_WIDTH, (Color) { 0, 0, 0, 0 });
    for (int y = 0; y < img.height; y++) {
        for (int x = 0; x < img.width; x++) {
            int index = x + (y * CHUNK_WIDTH);
            unsigned char value = (unsigned char)(((float)chunk->light[index] / 15.0f) * 255.0f);
            Color pixelColor = {
                .r = 0,
                .g = 0,
                .b = 0,
                .a = 255 - value
            };
            ImageDrawPixel(&img, x, y, pixelColor);
        }
    }
    UnloadTexture(chunk->lightMap);
    chunk->lightMap = LoadTextureFromImage(img);
    SetTextureFilter(chunk->lightMap, TEXTURE_FILTER_BILINEAR);
}
*/

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

        if (chunk->walls[j] > 0) {
            textureRect.x = (float)(chunk->walls[j] - 1) * TILE_SIZE;

            if (blocksAtlas) {
                DrawTextureRec(
                    *blocksAtlas,
                    textureRect,
                    pos,
                    GRAY
                );
            } else {
                DrawRectangle(
                    x,
                    y,
                    TILE_SIZE,
                    TILE_SIZE,
                    GRAY
                );
            }
        }

        if (chunk->blocks[j] > 0) {
            if (blocksAtlas) {
                DrawTextureRec(
                    *blocksAtlas,
                    textureRect,
                    pos,
                    WHITE
                );
            } else {
                DrawRectangle(
                    x,
                    y,
                    TILE_SIZE,
                    TILE_SIZE,
                    WHITE
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