#ifndef BLOCK_FUNCTIONS_H
#define BLOCK_FUNCTIONS_H

#include <stdbool.h>

#include "chunk.h"

bool grounded_block_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
bool plant_block_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
bool torch_state_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
bool fence_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
bool on_chest_interact(BlockInstance* inst, Chunk* chunk);
bool chest_solver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
void on_chest_destroy(BlockInstance* inst, Chunk* chunk, uint8_t idx);
bool liquid_solver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
void on_liquid_destroy(BlockInstance* inst, Chunk* chunk, uint8_t idx);

#endif