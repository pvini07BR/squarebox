#include "world_manager.h"
#include "registries/block_registry.h"
#include "item_container.h"
#include "sign_editor.h"
#include "raylib.h"

#include <errno.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

WorldInfo worldInfo;
char* currentWorldDir = NULL;

char* formatted_string(const char* fmt, ...) {
    if (fmt == NULL) return NULL;

    va_list args;
    va_start(args, fmt);
    size_t size = vsnprintf(NULL, 0, fmt, args) + 1; // +1 for null terminator
    va_end(args);

    char* buffer = (char*)malloc(size);
    if (buffer == NULL) {
        TraceLog(LOG_ERROR, "Error allocating memory for formatted string.\n");
        return NULL;
    }

    va_start(args, fmt);
    vsnprintf(buffer, size, fmt, args);
    va_end(args);

    return buffer;
}

void world_manager_init() {
    if (MakeDirectory("worlds") != 0) {
        if (errno != EEXIST) {
            TraceLog(LOG_ERROR, "Could not create the worlds directory: %s", strerror(errno));
            return;
        }
    }

	memset(&worldInfo, 0, sizeof(WorldInfo));
}

bool world_manager_create_world(WorldInfo info) {
    char* dirName = TextReplace(TextToLower(info.name), " ", "_");

    if (DirectoryExists(TextFormat("worlds/%s", dirName))) {
        TraceLog(LOG_ERROR, "World directory already exists: worlds/%s", dirName);
        free(dirName);
        return false;
	}

	const char* worldDir = TextFormat("worlds/%s", dirName);
    if (MakeDirectory(worldDir) != 0) {
        TraceLog(LOG_ERROR, "Could not create world directory (%s): %s", worldDir, strerror(errno));
        return false;
	}

    if (MakeDirectory(TextFormat("%s/chunks", worldDir)) != 0) {
        TraceLog(LOG_ERROR, "Could not create chunks directory (%s/chunks): %s", worldDir, strerror(errno));
        return false;
	}

    FILE* fptr = fopen(TextFormat("%s/worldinfo.bin", worldDir), "wb");
    if (!fptr) {
        TraceLog(LOG_ERROR, "Could not create world info (%s/worldinfo.bin): %s", worldDir, strerror(errno));
        return false;
    }
    fwrite(&info, sizeof(WorldInfo), 1, fptr);
    fclose(fptr);

    free(dirName);
    return true;
}

bool world_manager_save_world_info() {
    if (!currentWorldDir) {
		TraceLog(LOG_ERROR, "Could not save world info: no world is currently loaded.");
        return false;
    }

    if (!DirectoryExists(currentWorldDir)) {
        TraceLog(LOG_ERROR, "World directory does not exist: %s", currentWorldDir);
        return false;
    }

	FILE* fptr = fopen(TextFormat("%s/worldinfo.bin", currentWorldDir), "wb");
    if (!fptr) {
        TraceLog(LOG_ERROR, "Could not save world info (%s/worldinfo.bin): %s", currentWorldDir, strerror(errno));
        return false;
    }
	fwrite(&worldInfo, sizeof(WorldInfo), 1, fptr);
	fclose(fptr);
    return true;
}

bool world_manager_load_world_info(const char* worldDirName) {
    if (currentWorldDir) free(currentWorldDir);

	char* tmp = formatted_string("worlds/%s", worldDirName);

    if (!DirectoryExists(tmp)) {
		TraceLog(LOG_ERROR, "World directory does not exist: %s", tmp);
        free(tmp);
        return false;
    }

    FILE* fptr = fopen(TextFormat("%s/worldinfo.bin", tmp), "rb");
    if (!fptr) {
        TraceLog(LOG_ERROR, "Could not read world info (%s/worldinfo.bin): %s", tmp, strerror(errno));
        free(tmp);
        return false;
    }
    fread(&worldInfo, sizeof(WorldInfo), 1, fptr);
	fclose(fptr);

	currentWorldDir = tmp;
    return true;
}

bool world_manager_save_chunk(Chunk* chunk) {
    if (!chunk || !currentWorldDir) return false;
    
    FILE* fptr = fopen(TextFormat("%s/chunks/%d_%d.bin", currentWorldDir, chunk->position.x, chunk->position.y), "wb");
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
                switch (chunk->layers[l].blocks[b].id) {
                case BLOCK_SIGN:
                    fwrite(&dataOffset, sizeof(uint32_t), 1, fptr);
                    dataOffset += sizeof(SignLines);
                    break;
                case BLOCK_CHEST:
					fwrite(&dataOffset, sizeof(uint32_t), 1, fptr);
					dataOffset += item_container_serialized_size(chunk->layers[l].blocks[b].data);
                    break;
                default:
                    fwrite(&zero, sizeof(uint32_t), 1, fptr);
                    break;
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
                switch (chunk->layers[l].blocks[b].id) {
                    case BLOCK_SIGN: {
                        SignLines* lines = chunk->layers[l].blocks[b].data;
                        fwrite(lines, sizeof(SignLines), 1, fptr);
                        break;
                    }
                    case BLOCK_CHEST: {
						item_container_serialize(chunk->layers[l].blocks[b].data, fptr);
                        break;
                    }
                }
            }
        }
	}

    fclose(fptr);
    
    return true;
}

bool world_manager_load_chunk(Chunk* chunk) {
    if (!chunk || !currentWorldDir) return false;

    FILE* fptr = fopen(TextFormat("%s/chunks/%d_%d.bin", currentWorldDir, chunk->position.x, chunk->position.y), "rb");
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
                switch (chunk->layers[l].blocks[b].id) {
                case BLOCK_SIGN:
                    chunk->layers[l].blocks[b].data = malloc(sizeof(SignLines));
                    if (chunk->layers[l].blocks[b].data) {
                        fread(chunk->layers[l].blocks[b].data, sizeof(SignLines), 1, fptr);
                    } else {
                        TraceLog(LOG_ERROR, "Could not allocate memory for sign data in chunk at (%d, %d)", chunk->position.x, chunk->position.y);
                    }
                    break;
				case BLOCK_CHEST:
					chunk->layers[l].blocks[b].data = malloc(sizeof(ItemContainer));
                    if (chunk->layers[l].blocks[b].data) {
                        item_container_deserialize(chunk->layers[l].blocks[b].data, fptr);
                    }
                    else {
                        TraceLog(LOG_ERROR, "Could not allocate memory for chest data in chunk at (%d, %d)", chunk->position.x, chunk->position.y);
                    }
                    break;
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

bool world_manager_save_world_info_and_unload() {
    if (!currentWorldDir) return false;
    bool saved = world_manager_save_world_info();
    if (currentWorldDir) {
        free(currentWorldDir);
        currentWorldDir = NULL;
    }
    memset(&worldInfo, 0, sizeof(WorldInfo));
	return saved;
}

WorldInfo* get_world_info() {
    return &worldInfo;
}

bool world_manager_is_world_loaded() {
	return currentWorldDir != NULL;
}