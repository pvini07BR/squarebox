#ifndef WORLD_MANAGER_H
#define WORLD_MANAGER_H

#include "chunk.h"
#include <raylib.h>

#define WORLD_NAME_LENGTH 64

typedef struct {
    char name[WORLD_NAME_LENGTH];
    int seed;
    Vector2 player_position;
    bool player_flying;
} WorldInfo;

void world_manager_init();

bool world_manager_create_world(WorldInfo info);

bool world_manager_load_world_info(const char* worldDirName);
bool world_manager_save_world_info();
bool world_manager_save_world_info_and_unload();

WorldInfo* get_world_info();
bool world_manager_is_world_loaded();

bool world_manager_save_chunk(Chunk* chunk);
bool world_manager_load_chunk(Chunk* chunk);

#endif