#ifndef BLOCK_FUNCTIONS_H
#define BLOCK_FUNCTIONS_H

#include <stdbool.h>
#include <stdint.h>

#include "item_container.h"
#include "chunk.h"
#include "types.h"

// Function that will return a block variant calculated from a state by this function.
typedef BlockVariant (*BlockVariantGenerator)(uint8_t state);
// Function pointer to a function that will be called when the block is interacted with.
// returns a bool value to indicate if the interaction was successful or not.
typedef bool (*BlockInteractionCallback)(BlockExtraResult result, ItemSlot holdingItem);
// Function pointer to a function that will resolve the state of the block.
// returns a bool value to indicate if the block can be placed or not.
// the "other" member variable is either a block or wall, depending on the value of the layer argument.
typedef bool (*BlockStateResolver)(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
// Function pointer to a function that will be called when the block is destroyed.
typedef void (*BlockDestroyCallback)(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
// Function pointer to a function that will define the behavior for a block when it ticks.
// the returned value determines if the tick result has changed anything or not. This is to prevent
// unnecessary light and mesh updates.
// the "other" member variable is either a block or wall, depending on the value of the layer argument.
typedef bool (*BlockTickCallback)(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
// Function that draws anything on the top of a block based on the block's external data.
typedef void (*BlockOverlayRender)(void* data, Vector2 position, uint8_t state);
// Function for freeing up any block data.
typedef void (*BlockFreeData)(void* data);

bool grounded_block_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool plant_block_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool torch_state_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool fence_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool chest_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool sign_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool power_wire_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool power_source_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);

bool on_chest_interact(BlockExtraResult result, ItemSlot holdingItem);
bool trapdoor_interact(BlockExtraResult result, ItemSlot holdingItem);
bool sign_interact(BlockExtraResult result, ItemSlot holdingItem);
bool frame_block_interact(BlockExtraResult result, ItemSlot holdingItem);

void on_power_wire_destroy(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
void on_power_source_destroy(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);

bool falling_block_tick(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool water_source_tick(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);
bool water_flowing_tick(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer);

void sign_text_draw(void* data, Vector2 position, uint8_t state);

void chest_free_data(void* data);
void sign_free_data(void* data);

#endif