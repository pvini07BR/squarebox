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
    
    FILE* fptr = fopen(TextFormat("worlds/test_world/%d_%d.bin", chunk->position.x, chunk->position.y), "wb");
    if (!fptr) {
        TraceLog(LOG_ERROR, "Could not save chunk at position (%d, %d): %s", chunk->position.x, chunk->position.y, strerror(errno));
        return;
    }

    fclose(fptr);
}