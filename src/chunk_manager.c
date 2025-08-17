#include "chunk_manager.h"
#include "chunk.h"

#include <stdint.h>
#include <stdlib.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>

static Chunk* chunks = NULL;
static Texture2D blocksAtlas;
static Vector2i currentChunkPos;

void chunk_manager_init() {
    chunks = (Chunk*)malloc(sizeof(Chunk) * CHUNK_COUNT);
    blocksAtlas = LoadTexture(ASSETS_PATH "blocks.png");

    int startChunkX = -CHUNK_VIEW_WIDTH / 2;
	int startChunkY = -CHUNK_VIEW_HEIGHT / 2;

    for (int y = 0; y < CHUNK_VIEW_HEIGHT; y++) {
		for (int x = 0; x < CHUNK_VIEW_WIDTH; x++) {
			int i = y * CHUNK_VIEW_WIDTH + x;
			chunks[i].position.x = startChunkX + x;
			chunks[i].position.y = startChunkY + y;
			chunk_regenerate(&chunks[i]);
		}
	}

    chunk_manager_calculate_ligthing();
}

void chunk_manager_draw() {
    for (int i = 0; i < CHUNK_COUNT; i++) {
        chunk_draw(&chunks[i], &blocksAtlas);
    }
}

void chunk_manager_free() {
    UnloadTexture(blocksAtlas);
    if (chunks) {
        free(chunks);
        chunks = NULL;
    }
}

void chunk_manager_relocate(Vector2i newCenter) {
    currentChunkPos = newCenter;

    int startChunkX = currentChunkPos.x - CHUNK_VIEW_WIDTH / 2;
    int startChunkY = currentChunkPos.y - CHUNK_VIEW_HEIGHT / 2;

    Chunk tempChunks[CHUNK_COUNT];
    
    for (int i = 0; i < CHUNK_COUNT; i++) {
        int x = i % CHUNK_VIEW_WIDTH;
        int y = i / CHUNK_VIEW_WIDTH;

        Vector2i newPos = { startChunkX + x, startChunkY + y };

        bool found = false;
        for (int j = 0; j < CHUNK_COUNT; j++) {
            if (chunks[j].position.x == newPos.x && chunks[j].position.y == newPos.y) {
                tempChunks[i] = chunks[j];
                found = true;
                break;
            }
        }
        
        if (!found) {
            tempChunks[i].position = newPos;
            chunk_regenerate(&tempChunks[i]);
        }
    }
    
    for (int i = 0; i < CHUNK_COUNT; i++) {
        chunks[i] = tempChunks[i];
    }
    
    chunk_manager_calculate_ligthing();
}

void chunk_manager_calculate_ligthing() {
    
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
        chunk_manager_calculate_ligthing();
	}
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

/*
Rectangle rect = {
    (float)x,
    (float)y,
    TILE_SIZE,
    TILE_SIZE,
};

Color topLeft = (Color){ 0, 0, 0, 0 };
Color bottomLeft = (Color){ 0, 0, 0, 0 };
Color topRight = (Color){ 0, 0, 0, 0 };
Color bottomRight = (Color){ 0, 0, 0, 0 };

Vector2i globalBlockPos = {
    chunks[i].position.x * CHUNK_WIDTH + (j % CHUNK_WIDTH),
    chunks[i].position.y * CHUNK_WIDTH + (j / CHUNK_WIDTH)
};

Vector2i offsets[4] = {
    { 1, 0 },
    {-1, 0 },
    { 0, 1 },
    { 0,-1 }
};

for (int i = 0; i < 4; i++) {
    Vector2i neighborPos = { globalBlockPos.x + offsets[i].x, globalBlockPos.y + offsets[i].y };
    int neighborBlock = get_block(neighborPos, currentChunkPos);
    if (neighborBlock > 0) {
        if (offsets[i].x == 1 && offsets[i].y == 0) {
            topRight = BLACK;
            bottomRight = BLACK;
        }
        else if (offsets[i].x == -1 && offsets[i].y == 0) {
            topLeft = BLACK;
            bottomLeft = BLACK;
        }
        else if (offsets[i].y == 1 && offsets[i].x == 0) {
            bottomLeft = BLACK;
            bottomRight = BLACK;
        }
        else if (offsets[i].y == -1 && offsets[i].x == 0) {
            topLeft = BLACK;
            topRight = BLACK;
        }
    }
}

DrawRectangleGradientEx(
    rect,
    topLeft,
    bottomLeft,
    topRight,
    bottomRight
);
*/