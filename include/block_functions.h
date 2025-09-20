#ifndef BLOCK_FUNCTIONS_H
#define BLOCK_FUNCTIONS_H

#include <stdbool.h>
#include <stdint.h>

#include "chunk.h"

// Function pointer to a function that will be called when the block is interacted with.
// returns a bool value to indicate if the interaction was successful or not.
typedef bool (*BlockInteractionCallback)(BlockExtraResult result);
// Function pointer to a function that will resolve the state of the block.
// returns a bool value to indicate if the block can be placed or not.
// the "other" member variable is either a block or wall, depending on the value of isWall.
typedef bool (*BlockStateResolver)(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], bool isWall);
// Function pointer to a function that will be called when the block is destroyed.
typedef void (*BlockDestroyCallback)(BlockExtraResult result);
// Function pointer to a function that will define the behavior for a block when it ticks.
// the returned value determines if the tick result has changed anything or not. This is to prevent
// unnecessary light and mesh updates.
typedef bool (*BlockTickCallback)(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall);

bool grounded_block_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], bool isWall);
bool plant_block_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], bool isWall);
bool torch_state_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], bool isWall);
bool fence_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], bool isWall);
bool chest_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], bool isWall);

bool on_chest_interact(BlockExtraResult result);

void on_chest_destroy(BlockExtraResult result);

bool falling_block_tick(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall);
bool water_source_tick(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall);
bool water_flowing_tick(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall);

#endif