#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"

#include "chunk.h"

void chunk_manager_init(Vector2i center, uint8_t cvw, uint8_t cvh);
void chunk_manager_relocate(Vector2i newCenter);
void chunk_manager_set_view(uint8_t new_view_width, uint8_t new_view_height);
// This function recalculates all lighting in all chunks, and regenerates their meshes.
void chunk_manager_update_lighting();
void chunk_manager_draw(bool draw_lines);
void chunk_manager_draw_liquids();
void chunk_manager_tick();
void chunk_manager_free();

uint8_t chunk_manager_get_view_width();
uint8_t chunk_manager_get_view_height();

// Returns true when a interaction occurred, false when not.
bool chunk_manager_interact(Vector2i position, ChunkLayerEnum layer);

// This version of set block checks for surroundings before placing anything,
// and does not set a blockId to zero when its already zero.
// Also resolves the states and calls block callbacks.
void chunk_manager_set_block_safe(Vector2i position, BlockInstance blockValue, ChunkLayerEnum layer);

Chunk* chunk_manager_get_chunk(Vector2i position);
// Position is in global block coordinates
void chunk_manager_set_block(Vector2i position, BlockInstance blockValue, ChunkLayerEnum layer);
// Position is in global block coordinates
BlockInstance chunk_manager_get_block(Vector2i position, ChunkLayerEnum layer);
// Position is in global block coordinates
void chunk_manager_set_light(Vector2i position, uint8_t value);
// Position is in global block coordinates
uint8_t chunk_manager_get_light(Vector2i position);

#endif