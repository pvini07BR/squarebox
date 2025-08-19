#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "chunk.h"
#include <stdint.h>

#define CHUNK_VIEW_WIDTH 5
#define CHUNK_VIEW_HEIGHT 3

#define CHUNK_COUNT CHUNK_VIEW_WIDTH*CHUNK_VIEW_HEIGHT

void chunk_manager_init();
void chunk_manager_draw();
void chunk_manager_free();
void chunk_manager_relocate(Vector2i newCenter);

Chunk* chunk_manager_get_chunk(Vector2i position);
// Position is in global block coordinates
void chunk_manager_set_block(Vector2i position, int blockValue, bool isWall);
// Position is in global block coordinates
int chunk_manager_get_block(Vector2i position, bool isWall);
// Position is in global block coordinates
void chunk_manager_set_light(Vector2i position, uint8_t value);
// Position is in global block coordinates
uint8_t chunk_manager_get_light(Vector2i position);

#endif