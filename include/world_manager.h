#ifndef WORLD_MANAGER_H
#define WORLD_MANAGER_H

#include "item_container.h"
#include "chunk.h"
#include <raylib.h>

#include "thirdparty/raylib-nuklear.h"

#define WORLD_NAME_LENGTH 32
#define WORLD_VERSION 0

typedef enum {
    WORLD_GEN_PRESET_DEFAULT,
    WORLD_GEN_PRESET_FLAT,
    WORLD_GEN_PRESET_EMPTY,
} WorldGenPreset;

typedef struct {
    Vector2 player_position;
    ItemSlot hotbar_items[10];
    char name[WORLD_NAME_LENGTH];
    int seed;
    WorldGenPreset preset;
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

bool world_manager_save_chunk(Chunk* chunk);
bool world_manager_load_chunk(Chunk* chunk);

bool world_manager_load_world_list();
WorldListReturnType world_manager_draw_list(struct nk_context* ctx);

#endif