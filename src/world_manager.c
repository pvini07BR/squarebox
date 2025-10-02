#include "world_manager.h"
#include "registries/block_registry.h"
#include "item_container.h"
#include "sign_editor.h"
#include "raylib.h"

#include <errno.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

typedef enum {
    POPUP_TYPE_NONE,
    POPUP_TYPE_DELETING,
    POPUP_TYPE_CREATING
} PopupType;

WorldInfo worldInfo;
char* currentWorldDir = NULL;

WorldListEntry* worldList = NULL;
size_t worldListCount = 0;

WorldListEntry* selectedEntry = NULL;
PopupType popupType = POPUP_TYPE_NONE;

WorldInfo tempWorldInfo;

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

int gen_random_int() {
    const int BITS_PER_RAND = (int)(log2(RAND_MAX / 2 + 1) + 1.0);
    int ret = 0;
    for (int i = 0; i < sizeof(int) * CHAR_BIT; i += BITS_PER_RAND) {
        ret <<= BITS_PER_RAND;
        ret |= rand();
    }
    return ret;
}

void world_manager_init() {
    if (MakeDirectory("worlds") != 0) {
        if (errno != EEXIST) {
            TraceLog(LOG_ERROR, "Could not create the worlds directory: %s", strerror(errno));
            return;
        }
    }

	memset(&worldInfo, 0, sizeof(WorldInfo));
    memset(&tempWorldInfo, 0, sizeof(WorldInfo));

    world_manager_load_world_list();
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

bool world_manager_load_world_info(const char* worldDir) {
    if (currentWorldDir) free(currentWorldDir);

	char* tmp = formatted_string("%s", worldDir);

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

    if (worldInfo.version != WORLD_VERSION) {
        TraceLog(LOG_WARNING, "The world (%s) has been created with a different version.\nThere is no world version conversion, so be careful with crashes and data corruption and loss!\nWorld version: %d\nCurrent version: %d", tmp, worldInfo.version, WORLD_VERSION);
    }

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

	uint32_t baseOffset = sizeof(uint8_t) + ((sizeof(uint8_t) * 2 + sizeof(uint32_t)) * CHUNK_AREA * CHUNK_LAYER_COUNT);
    uint32_t dataOffset = baseOffset;
    uint32_t zero = 0;

    uint8_t version = WORLD_VERSION;
    fwrite(&version, sizeof(uint8_t), 1, fptr);

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

    uint8_t version;
    fread(&version, sizeof(uint8_t), 1, fptr);
    if (version != WORLD_VERSION) {
        TraceLog(LOG_WARNING, "The chunk at (%d, %d) is being loaded with a different version. Be careful with data corruption and loss!\nChunk version: %d\nCurrent version: %d", chunk->position.x, chunk->position.y, version, WORLD_VERSION);
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

void world_manager_free() {
    if (worldList) {
        for (size_t i = 0; i < worldListCount; i++) {
            if (worldList[i].worldDir) free(worldList[i].worldDir);
        }
        free(worldList);
        worldList = NULL;
        worldListCount = 0;
    }
    if (currentWorldDir) free(currentWorldDir);
}

WorldInfo* get_world_info() {
    return &worldInfo;
}

bool world_manager_is_world_loaded() {
	return currentWorldDir != NULL;
}

bool world_manager_load_world_list() {
    if (selectedEntry) selectedEntry->selected = false;
    selectedEntry = NULL;

    if (worldList) {
        for (size_t i = 0; i < worldListCount; i++) {
            if (worldList[i].worldDir) free(worldList[i].worldDir);
        }
        free(worldList);
        worldList = NULL;
        worldListCount = 0;
    }

    FilePathList list = LoadDirectoryFiles("worlds");
    if (list.count <= 0) {
        UnloadDirectoryFiles(list);
        return true;
    }

    size_t valid_count = 0;
    for (size_t i = 0; i < list.count; i++) {
        if (FileExists(TextFormat("%s/worldinfo.bin", list.paths[i]))) valid_count++;
    }

    if (valid_count == 0) {
        UnloadDirectoryFiles(list);
        return true;
    }

    worldList = (WorldListEntry*)calloc(valid_count, sizeof(WorldListEntry));
    if (!worldList) {
        TraceLog(LOG_ERROR, "Could not allocate memory for the world list.");
        UnloadDirectoryFiles(list);
        return false;
    }

    size_t idx = 0;
    for (size_t i = 0; i < list.count && idx < valid_count; i++) {
        const char* path = TextFormat("%s/worldinfo.bin", list.paths[i]);
        if (!FileExists(path)) continue;

        FILE* fptr = fopen(path, "rb");
        if (!fptr) {
            TraceLog(LOG_ERROR, "Could not open world info (%s): %s", path, strerror(errno));
            free(worldList);
            worldList = NULL;
            worldListCount = 0;
            UnloadDirectoryFiles(list);
            return false;
        }

        WorldInfo winfo;
        size_t r = fread(&winfo, sizeof(WorldInfo), 1, fptr);
        fclose(fptr);
        if (r != 1) {
            TraceLog(LOG_ERROR, "Could not read world info (%s): read=%zu", path, r);
            free(worldList);
            worldList = NULL;
            worldListCount = 0;
            UnloadDirectoryFiles(list);
            return false;
        }

        worldList[idx].info = winfo;
        worldList[idx].selected = false;

        unsigned int len = strlen(list.paths[i]);
        char* dirCpy = (char*)malloc(len + 1);
        if (!dirCpy) {
            TraceLog(LOG_ERROR, "Could not allocate memory for the world directory");
            worldList[idx].worldDir = NULL;
            worldList[idx].worldDirLen = 0;
        }
        else {
            strcpy(dirCpy, list.paths[i]);
            worldList[idx].worldDir = dirCpy;
            worldList[idx].worldDirLen = len;
        }

        idx++;
    }

    worldListCount = idx;

    if (worldListCount < valid_count) {
        WorldListEntry* tmp = (WorldListEntry*)realloc(worldList, worldListCount * sizeof(WorldListEntry));
        if (tmp) worldList = tmp;
    }

    UnloadDirectoryFiles(list);

    return true;
}

WorldListReturnType world_manager_draw_list(struct nk_context* ctx) {
    WorldListReturnType returnType = WORLD_RETURN_NONE;

    const Vector2 totalSize = { 620, 243 };

    const Vector2 screenCenter = {
        (GetScreenWidth() / 2.0f) - (totalSize.x / 2.0f),
        (GetScreenHeight() / 2.0f) - (totalSize.y / 2.0f),
    };

    const Vector2 popupSize = { totalSize.x, 150 };
    const Vector2 popupCenter = {
        (totalSize.x / 2.0f) - (popupSize.x / 2.0f),
        (totalSize.y / 2.0f) - (popupSize.y / 2.0f),
    };

    if (nk_begin(ctx, "World List", nk_rect(screenCenter.x, screenCenter.y, totalSize.x, totalSize.y - 70), 0)) {
        if (popupType == POPUP_TYPE_CREATING) {
            if (nk_popup_begin(ctx, NK_POPUP_STATIC, "World Creation", 0, nk_rect(popupCenter.x, popupCenter.y, popupSize.x, popupSize.y))) {
                nk_layout_row_dynamic(ctx, 40, 3);
                nk_label(ctx, "World Name", NK_TEXT_LEFT);

                nk_spacing(ctx, 1);

                nk_edit_string_zero_terminated(
                    ctx,
                    NK_EDIT_BOX | NK_EDIT_AUTO_SELECT,
                    tempWorldInfo.name,
                    sizeof(tempWorldInfo.name),
                    nk_filter_ascii
                );

                nk_layout_row_dynamic(ctx, 40, 4);

                nk_label(ctx, "Seed", NK_TEXT_LEFT);

                nk_spacing(ctx, 1);

                if (nk_button_label(ctx, "Random")) {
                    tempWorldInfo.seed = gen_random_int();
                }
                tempWorldInfo.seed = nk_propertyi(ctx, "", INT_MIN, tempWorldInfo.seed, INT_MAX, 1, 10.0f);

                nk_layout_row_dynamic(ctx, 40, 2);
                if (nk_button_label(ctx, "Back")) {
                    memset(&tempWorldInfo, 0, sizeof(WorldInfo));
                    popupType = POPUP_TYPE_NONE;
                    nk_popup_close(ctx);
                }
                if (nk_button_label(ctx, "Create")) {
                    world_manager_create_world(tempWorldInfo);
                    world_manager_load_world_list();
                    memset(&tempWorldInfo, 0, sizeof(WorldInfo));
                    popupType = POPUP_TYPE_NONE;
                    nk_popup_close(ctx);
                }
            }
            nk_popup_end(ctx);
        }
        
        nk_layout_row_dynamic(ctx, 30, 1);
        if (worldList) {
            for (size_t i = 0; i < worldListCount; i++) {
                WorldListEntry* entry = &worldList[i];

                if(nk_select_text(ctx, entry->info.name, WORLD_NAME_LENGTH, NK_TEXT_LEFT, entry->selected)) {
                    if (selectedEntry) selectedEntry->selected = false;
                    selectedEntry = entry;
                    selectedEntry->selected = true;
                }
            }
        }
    }
    nk_end(ctx);

    if (nk_begin(ctx, "World List Buttons", nk_rect(screenCenter.x, screenCenter.y + totalSize.y - 70, totalSize.x, 70), NK_WINDOW_NO_SCROLLBAR)) {
        nk_layout_row_dynamic(ctx, 30, 2);

        if (selectedEntry) {
            if (nk_button_label(ctx, "Play")) {
                if (world_manager_load_world_info(selectedEntry->worldDir)) {
                    returnType = WORLD_RETURN_OPEN_WORLD;
                }
            }
        } else {
            nk_spacing(ctx, 1);
        }
        if (nk_button_label(ctx, "Create New World")) {
            tempWorldInfo.seed = gen_random_int();
            popupType = POPUP_TYPE_CREATING;
        }
        
        nk_layout_row_dynamic(ctx, 30, 2);

        if (nk_button_label(ctx, "Back")) {
            returnType = WORLD_RETURN_CLOSE;
        }

        if (nk_button_label(ctx, "Refresh")) {
            if (selectedEntry) selectedEntry->selected = false;
            selectedEntry = NULL;
            world_manager_load_world_list();
        }
    }
    nk_end(ctx);

    if (returnType != WORLD_RETURN_NONE) {
        if (selectedEntry) selectedEntry->selected = false;
        selectedEntry = NULL;
    }

    return returnType;
}