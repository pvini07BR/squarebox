#ifndef BLOCK_FUNCTIONS_H
#define BLOCK_FUNCTIONS_H

#include <stdbool.h>
#include <stdint.h>

#include "chunk.h"

// Function that returns the state value depending on a defined list of possible states.
// This is used only when placing blocks that rotates but has more than one state field, like trapdoors.
typedef uint8_t (*BlockStateSelector)(uint8_t idx);
// Function that will get the index of a specific variant for a block depending on the bit values on a state.
typedef uint8_t (*BlockVariantSelector)(uint8_t state);
// Function pointer to a function that will be called when the block is interacted with.
// returns a bool value to indicate if the interaction was successful or not.
typedef bool (*BlockInteractionCallback)(BlockExtraResult result);
// Function pointer to a function that will resolve the state of the block.
// returns a bool value to indicate if the block can be placed or not.
// the "other" member variable is either a block or wall, depending on the value of isWall.
typedef bool (*BlockStateResolver)(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
// Function pointer to a function that will be called when the block is destroyed.
typedef void (*BlockDestroyCallback)(BlockExtraResult result);
// Function pointer to a function that will define the behavior for a block when it ticks.
// the returned value determines if the tick result has changed anything or not. This is to prevent
// unnecessary light and mesh updates.
typedef bool (*BlockTickCallback)(BlockExtraResult result, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
// Function that draws anything on the top of a block based on the block's external data.
typedef void (*BlockOverlayRender)(void* data, Vector2 position, uint8_t state);

uint8_t trapdoor_state_selector(uint8_t idx);
uint8_t trapdoor_variant_selector(uint8_t state);

bool grounded_block_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool plant_block_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool torch_state_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool fence_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool chest_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool sign_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);

bool on_chest_interact(BlockExtraResult result);
bool trapdoor_interact(BlockExtraResult result);
bool sign_interact(BlockExtraResult result);

void on_chest_destroy(BlockExtraResult result);
void on_sign_destroy(BlockExtraResult result);

bool falling_block_tick(BlockExtraResult result, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool water_source_tick(BlockExtraResult result, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool water_flowing_tick(BlockExtraResult result, BlockExtraResult neighbors[4], ChunkLayerEnum layer);

void sign_text_draw(void* data, Vector2 position, uint8_t state);

#endif