#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"

#include "chunk.h"

void chunk_manager_init(size_t chunk_view_width, size_t chunk_view_height);
void chunk_manager_draw(bool draw_lines);
void chunk_manager_draw_liquids();
void chunk_manager_tick();
void chunk_manager_free();
void chunk_manager_set_view(size_t new_view_width, size_t new_view_height);
void chunk_manager_reload_chunks();
void chunk_manager_relocate(Vector2i newCenter);
void chunk_manager_update_lighting();

size_t chunk_manager_get_view_width();
size_t chunk_manager_get_view_height();

// Returns true when a interaction occurred, false when not.
bool chunk_manager_interact(Vector2i position, bool isWall);

// This version of set block checks for surroundings before placing anything,
// and does not set a blockId to zero when its already zero.
// Also resolves the states and calls block callbacks.
void chunk_manager_set_block_safe(Vector2i position, BlockInstance blockValue, bool isWall);

Chunk* chunk_manager_get_chunk(Vector2i position);
// Position is in global block coordinates
void chunk_manager_set_block(Vector2i position, BlockInstance blockValue, bool isWall);
// Position is in global block coordinates
BlockInstance chunk_manager_get_block(Vector2i position, bool isWall);
// Position is in global block coordinates
void chunk_manager_set_light(Vector2i position, uint8_t value);
// Position is in global block coordinates
uint8_t chunk_manager_get_light(Vector2i position);

#endif