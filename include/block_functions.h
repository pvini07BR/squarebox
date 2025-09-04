#ifndef BLOCK_FUNCTIONS_H
#define BLOCK_FUNCTIONS_H

#include <stdbool.h>

#include "chunk.h"

// Function pointer to a function that will be called when the block is interacted with.
// returns a bool value to indicate if the interaction was successful or not.
typedef bool (*BlockInteractionCallback)(BlockInstance* inst, Chunk* chunk);
// Function pointer to a function that will resolve the state of the block.
// returns a bool value to indicate if the block can be placed or not.
typedef bool (*BlockStateResolver)(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
// Function pointer to a function that will be called when the block is destroyed.
typedef void (*BlockDestroyCallback)(BlockInstance* inst, Chunk* chunk, uint8_t idx);

bool grounded_block_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
bool plant_block_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
bool torch_state_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
bool liquid_solver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
bool fence_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);
bool chest_solver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall);

bool on_chest_interact(BlockInstance* inst, Chunk* chunk);

void on_chest_destroy(BlockInstance* inst, Chunk* chunk, uint8_t idx);
void on_liquid_destroy(BlockInstance* inst, Chunk* chunk, uint8_t idx);

#endif