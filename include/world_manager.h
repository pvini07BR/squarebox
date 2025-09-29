#ifndef WORLD_MANAGER_H
#define WORLD_MANAGER_H

#include "chunk.h"
#include <raylib.h>

typedef struct {
    char name[64];
    Vector2 player_position;
} WorldInfo;

void world_manager_init();
void world_manager_load_world(const char* name);

bool world_manager_save_chunk(Chunk* chunk);
bool world_manager_load_chunk(Chunk* chunk);

#endif