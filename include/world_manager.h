#ifndef WORLD_MANAGER_H
#define WORLD_MANAGER_H

#include "item_container.h"
#include "chunk.h"

#include <raylib.h>

#include "thirdparty/microui.h"
#include "types.h"

#define WORLD_NAME_LENGTH 32
#define WORLD_VERSION 0

typedef enum {
    WORLD_GEN_PRESET_DEFAULT,
    WORLD_GEN_PRESET_FLAT,
    WORLD_GEN_PRESET_EMPTY,
} WorldGenPreset;

typedef struct {
    char name[WORLD_NAME_LENGTH];
    ItemSlot hotbar_items[10];
    Vector2 player_position;
    WorldGenPreset preset;
    int seed;
    uint8_t version;
    bool player_flying;
} WorldInfo;

typedef struct {
    unsigned int worldDirLen;
    char* worldDir;
    WorldInfo info;
    bool selected;
} WorldListEntry;

typedef enum {
    CHUNK_LOAD_SUCCESS,
    CHUNK_LOAD_ERROR_NOT_FOUND,
    CHUNK_LOAD_ERROR_FATAL
} ChunkLoadStatus;

typedef enum {
    WORLD_RETURN_NONE,
    WORLD_RETURN_CLOSE,
    WORLD_RETURN_OPEN_WORLD
} WorldListReturnType;

void world_manager_init();

bool world_manager_create_world(WorldInfo info);

bool world_manager_load_world_info(const char* worldDirName);
bool world_manager_save_world_info();
bool world_manager_save_world_info_and_unload();
void world_manager_free();

WorldInfo* get_world_info();
bool world_manager_is_world_loaded();

bool world_manager_save_chunk(Vector2i position, ChunkLayer layers[CHUNK_LAYER_COUNT]);
ChunkLoadStatus world_manager_load_chunk(Vector2i position, ChunkLayer layers[CHUNK_LAYER_COUNT]);

bool world_manager_load_world_list();
WorldListReturnType world_manager_draw_list(mu_Context* ctx);

#endif