#include "world_manager.h"
#include "raylib.h"

#include <errno.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

void world_manager_init() {
    if (mkdir("worlds", 0755) != 0) {
        if (errno != EEXIST) {
            TraceLog(LOG_ERROR, "Could not create the worlds directory: %s", strerror(errno));
            return;
        }
    }
}

void world_manager_load_world(const char* name) {
    
}

void world_manager_save_chunk(Chunk* chunk) {
    if (!chunk) return;
    
    FILE* fptr = fopen(TextFormat("worlds/test_world/chunks/%d_%d.bin", chunk->position.x, chunk->position.y), "wb");
    if (!fptr) {
        TraceLog(LOG_ERROR, "Could not save chunk at position (%d, %d): %s", chunk->position.x, chunk->position.y, strerror(errno));
        return;
    }

    for (int l = 0; l < CHUNK_LAYER_COUNT; l++) {
        size_t written = fwrite(chunk->layers[l].blocks, sizeof(BlockInstance), CHUNK_AREA, fptr);
        if (written != CHUNK_AREA) {
            TraceLog(LOG_ERROR, "Could not write chunk layer %d data: %s", l, strerror(errno));
            fclose(fptr);
            return;
		}
    }

    fclose(fptr);
}

bool world_manager_load_chunk(Chunk* chunk) {
    if (!chunk) return false;

    FILE* fptr = fopen(TextFormat("worlds/test_world/chunks/%d_%d.bin", chunk->position.x, chunk->position.y), "rb");
    if (!fptr) {
        if (errno != ENOENT) {
            TraceLog(LOG_ERROR, "Could not read chunk at position (%d, %d): %s", chunk->position.x, chunk->position.y, strerror(errno));
        }
        return false;
    }

    for (int l = 0; l < CHUNK_LAYER_COUNT; l++) {
        size_t read = fread(chunk->layers[l].blocks, sizeof(BlockInstance), CHUNK_AREA, fptr);
        if (read != CHUNK_AREA) {
            TraceLog(LOG_ERROR, "Could not read chunk layer %d data: %s", l, strerror(errno));
            fclose(fptr);
            return false;
        }
    }

    fclose(fptr);

    return true;
}