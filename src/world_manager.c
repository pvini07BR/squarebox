#include "world_manager.h"
#include "registries/block_registry.h"
#include "sign_editor.h"
#include "raylib.h"

#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
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

bool world_manager_save_chunk(Chunk* chunk) {
    if (!chunk) return false;
    
    FILE* fptr = fopen(TextFormat("worlds/test_world/chunks/%d_%d.bin", chunk->position.x, chunk->position.y), "wb");
    if (!fptr) {
        TraceLog(LOG_ERROR, "Could not save chunk at position (%d, %d): %s", chunk->position.x, chunk->position.y, strerror(errno));
        return false;
    }

	uint32_t baseOffset = (sizeof(uint8_t) * 2 + sizeof(uint32_t)) * CHUNK_AREA * CHUNK_LAYER_COUNT;
    uint32_t dataOffset = baseOffset;
    uint32_t zero = 0;

    for (int l = 0; l < CHUNK_LAYER_COUNT; l++) {
        for (int b = 0; b < CHUNK_AREA; b++) {
			fwrite(&chunk->layers[l].blocks[b].id, sizeof(uint8_t), 1, fptr);
            fwrite(&chunk->layers[l].blocks[b].state, sizeof(uint8_t), 1, fptr);

            if (chunk->layers[l].blocks[b].data) {
                if (chunk->layers[l].blocks[b].id == BLOCK_SIGN) {
                    fwrite(&dataOffset, sizeof(uint32_t), 1, fptr);
					dataOffset += sizeof(SignLines);
                }
                else {
                    fwrite(&zero, sizeof(uint32_t), 1, fptr);
                }
            }
            else {
				fwrite(&zero, sizeof(uint32_t), 1, fptr);
            }
        }
    }

    for (int l = 0; l < CHUNK_LAYER_COUNT; l++) {
        for (int b = 0; b < CHUNK_AREA; b++) {
            if (chunk->layers[l].blocks[b].data) {
                if (chunk->layers[l].blocks[b].id == BLOCK_SIGN) {
                    SignLines* lines = chunk->layers[l].blocks[b].data;
                    fwrite(lines, sizeof(SignLines), 1, fptr);
                }
            }
        }
	}

    fclose(fptr);
    
    return true;
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
        for (int b = 0; b < CHUNK_AREA; b++) {
			fread(&chunk->layers[l].blocks[b].id, sizeof(uint8_t), 1, fptr);
            fread(&chunk->layers[l].blocks[b].state, sizeof(uint8_t), 1, fptr);
            uint32_t dataOffset = 0;
            fread(&dataOffset, sizeof(uint32_t), 1, fptr);
            if (dataOffset != 0) {
                long currentPos = ftell(fptr);
                fseek(fptr, dataOffset, SEEK_SET);
                if (chunk->layers[l].blocks[b].id == BLOCK_SIGN) {
                    chunk->layers[l].blocks[b].data = malloc(sizeof(SignLines));
                    if (chunk->layers[l].blocks[b].data) {
                        fread(chunk->layers[l].blocks[b].data, sizeof(SignLines), 1, fptr);
                    } else {
                        TraceLog(LOG_ERROR, "Could not allocate memory for sign data in chunk at (%d, %d)", chunk->position.x, chunk->position.y);
                    }
                }
                fseek(fptr, currentPos, SEEK_SET);
            } else {
                chunk->layers[l].blocks[b].data = NULL;
			}
        }
    }

    fclose(fptr);

    return true;
}