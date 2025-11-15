#include "world_manager.h"
#include "registries/block_registry.h"
#include "item_container.h"
#include "sign_editor.h"
#include "raylib.h"

#include <errno.h>

#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

const char* presetComboboxItems[] = {
    "Default",
    "Flat",
    "Empty"
};

WorldInfo worldInfo;
char* currentWorldDir = NULL;

WorldListEntry* worldList = NULL;
size_t worldListCount = 0;

WorldListEntry* selectedEntry = NULL;
bool creatingWorld = false;

WorldInfo tempWorldInfo;

int combobox(mu_Context* ctx, int item_count, const char* items[], int* item_idx) {
    mu_Id id = mu_get_id(ctx, items, sizeof(const char*) * item_count);
    mu_Rect rect = mu_layout_next(ctx);
    mu_update_control(ctx, id, rect, 0);

    int biggest_width = 0;
    for (int i = 0; i < item_count; i++) {
        int width = ctx->text_width(ctx->style->font, items[i], strlen(items[i]));
        if (width > biggest_width) {
            biggest_width = width;
        }
    }

    int res = 0;
    if (ctx->mouse_pressed == MU_MOUSE_LEFT && ctx->focus == id) {
        mu_open_popup(ctx, "combobox");
    }

    if (mu_begin_popup(ctx, "combobox")) {
        mu_layout_row(ctx, 1, (int[1]) { biggest_width + 20 }, 30);

        for (int i = 0; i < item_count; i++) {
            if (mu_button(ctx, items[i])) {
                *item_idx = i;
                res |= MU_RES_CHANGE;
            }
        }
        mu_end_popup(ctx);
    }

    mu_draw_control_frame(ctx, id, rect, MU_COLOR_BASE, 0);
    mu_draw_control_text(ctx, items[*item_idx], rect, MU_COLOR_TEXT, 0);

    return res;
}

int valuebox(mu_Context* ctx, int* value) {
    mu_Id id = mu_get_id(ctx, value, sizeof(value));
    mu_Rect rect = mu_layout_next(ctx);
    mu_update_control(ctx, id, rect, 0);

    int res = 0;

    mu_draw_control_frame(ctx, id, rect, MU_COLOR_BASE, 0);
    char buf[MU_MAX_FMT + 1];
    sprintf(buf, "%d", *value);
    mu_draw_control_text(ctx, buf, rect, MU_COLOR_TEXT, 0);

    return res;
}

int world_entry(mu_Context* ctx, WorldListEntry* entry) {
    mu_Id id = mu_get_id(ctx, entry, sizeof(WorldListEntry));
    mu_Rect rect = mu_layout_next(ctx);
    mu_update_control(ctx, id, rect, 0);

    int res = 0;
    if (ctx->mouse_pressed == MU_MOUSE_LEFT && ctx->focus == id) {
        selectedEntry = entry;
        res |= MU_RES_CHANGE;
    }

    if (selectedEntry == entry) {
        mu_draw_control_frame(ctx, id, rect, MU_COLOR_BUTTON, 0);
    }
    else {
        mu_draw_control_frame(ctx, id, rect, MU_COLOR_BASE, 0);
    }
    mu_draw_control_text(ctx, entry->info.name, rect, MU_COLOR_TEXT, 0);

    return res;
}

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
    for (unsigned long i = 0; i < sizeof(int) * CHAR_BIT; i += BITS_PER_RAND) {
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
        TraceLog(LOG_ERROR, "Could not load world info (%s/worldinfo.bin): %s", tmp, strerror(errno));
        free(tmp);
        return false;
    }
    fread(&worldInfo, sizeof(WorldInfo), 1, fptr);
	fclose(fptr);

    if (worldInfo.version != WORLD_VERSION) {
        TraceLog(LOG_ERROR, "Refused to load the world info because it is saved in another version.\nWorld version: %d\nCurrent version: %d", tmp, worldInfo.version, WORLD_VERSION);
        free(tmp);
        return false;
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

ChunkLoadStatus world_manager_load_chunk(Chunk* chunk) {
    if (!chunk || !currentWorldDir) return false;

    FILE* fptr = fopen(TextFormat("%s/chunks/%d_%d.bin", currentWorldDir, chunk->position.x, chunk->position.y), "rb");
    if (!fptr) {
        if (errno != ENOENT) {
            TraceLog(LOG_ERROR, "Could not read chunk at position (%d, %d): %s", chunk->position.x, chunk->position.y, strerror(errno));
            return CHUNK_LOAD_ERROR_FATAL;
        }
        return CHUNK_LOAD_ERROR_NOT_FOUND;
    }

    uint8_t version;
    fread(&version, sizeof(uint8_t), 1, fptr);
    if (version != WORLD_VERSION) {
        TraceLog(LOG_ERROR, "Refused to load chunk (%d, %d) because its saved in a different version.\nChunk version: %d\nCurrent version: %d", chunk->position.x, chunk->position.y, version, WORLD_VERSION);
        return CHUNK_LOAD_ERROR_FATAL;
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

    return CHUNK_LOAD_SUCCESS;
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

WorldListReturnType world_manager_draw_list(mu_Context* ctx) {
    WorldListReturnType returnType = WORLD_RETURN_NONE;

    if (!creatingWorld) {
        if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) return WORLD_RETURN_CLOSE;

        if (mu_begin_window_ex(ctx, "Select a World", mu_rect(0, 0, 500, 500), MU_OPT_NOSCROLL | MU_OPT_NOCLOSE)) {
            mu_Container* win = mu_get_current_container(ctx);

            win->rect.x = (GetScreenWidth() / 2.0f) - (win->rect.w / 2.0f);
            win->rect.y = (GetScreenHeight() / 2.0f) - (win->rect.h / 2.0f);

            mu_layout_row(ctx, 1, (int[1]){ -1 }, -69);
            mu_begin_panel(ctx, "World List");
            mu_layout_row(ctx, 1, (int[]) { -1 }, 30);

            if (worldList) {
                for (size_t i = 0; i < worldListCount; i++) {
                    world_entry(ctx, &worldList[i]);
                }
            }
            
            mu_end_panel(ctx);

            mu_layout_height(ctx, ctx->style->padding);
            mu_layout_next(ctx);

            mu_layout_row(ctx, 2, (int[2]){ -250, -1 }, 30);
            if (selectedEntry) {
                if (mu_button(ctx, "Play")) {
                    if (world_manager_load_world_info(selectedEntry->worldDir)) {
                        returnType = WORLD_RETURN_OPEN_WORLD;
                    }
                }
            }
            else {
                mu_layout_next(ctx);
            }
            if (mu_button(ctx, "Create New World")) {
                tempWorldInfo.seed = gen_random_int();
                creatingWorld = true;
            }
            mu_layout_row(ctx, 2, (int[2]) { -250, -1 }, 30);
            if (mu_button(ctx, "Back")) returnType = WORLD_RETURN_CLOSE;
            if (mu_button(ctx, "Refresh")) world_manager_load_world_list();

            mu_end_window(ctx);
        }
    } else {
        if (IsKeyPressed(KEY_ESCAPE) || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) creatingWorld = false;

        if (mu_begin_window_ex(ctx, "Create New World", mu_rect(0, 0, 400, 0), MU_OPT_NOSCROLL | MU_OPT_NOCLOSE)) {
            mu_Container* win = mu_get_current_container(ctx);

            win->rect.h = win->content_size.y + (ctx->style->padding * 2) + 25;

            win->rect.x = (GetScreenWidth() / 2.0f) - (win->rect.w / 2.0f);
            win->rect.y = (GetScreenHeight() / 2.0f) - (win->rect.h / 2.0f);

            mu_layout_row(ctx, 2, (int[2]) { -(win->rect.w - 160), -1 }, 30);
            mu_label(ctx, "World Name");
            mu_textbox(ctx, tempWorldInfo.name, sizeof(tempWorldInfo.name));

            mu_label(ctx, "World Preset");
            combobox(ctx, 3, presetComboboxItems, (int*)& tempWorldInfo.preset);
            
            mu_layout_row(ctx, 3, (int[3]) { -(win->rect.w - 160), -(win->rect.w - 260), -1 }, 30);
            mu_label(ctx, "Seed");
            if (mu_button(ctx, "Random")) {
                tempWorldInfo.seed = gen_random_int();
            }
            valuebox(ctx, &tempWorldInfo.seed);

            mu_layout_height(ctx, ctx->style->padding);
            mu_layout_next(ctx);

            mu_layout_row(ctx, 2, (int[2]) { -(win->rect.w / 2), -1 }, 30);
            if (mu_button(ctx, "Back")) {
                memset(&tempWorldInfo, 0, sizeof(WorldInfo));
                creatingWorld = false;
            }
            if (mu_button(ctx, "Create")) {
                if (world_manager_create_world(tempWorldInfo)) {
                    world_manager_load_world_list();
                    memset(&tempWorldInfo, 0, sizeof(WorldInfo));
                    creatingWorld = false;
                }
            }

            mu_end_window(ctx);
        }
    }

    if (returnType != WORLD_RETURN_NONE) {
        if (selectedEntry) selectedEntry->selected = false;
        selectedEntry = NULL;
    }

    return returnType;
}