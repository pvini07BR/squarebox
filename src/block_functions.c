#include "block_functions.h"

#include "item_container.h"
#include "raylib.h"
#include "registries/block_registry.h"
#include "block_state_bitfields.h"
#include "chunk.h"
#include "types.h"

#include <stdio.h>
#include <stdlib.h>

uint8_t trapdoor_state_selector(uint8_t idx) {
    uint8_t state = 0;

    state = (state & ~TRAPDOOR_ROTATION_MASK) | (idx & TRAPDOOR_ROTATION_MASK);
    state = (state & ~TRAPDOOR_OPEN_MASK) | (false ? TRAPDOOR_OPEN_MASK : 0);

    return state;
}

uint8_t trapdoor_variant_selector(uint8_t state) {
    int rotation = state & TRAPDOOR_ROTATION_MASK;
    bool open = (state & TRAPDOOR_OPEN_MASK) ? true : false;

    if (rotation == 0) {
        if (!open) {
            return 0;
        }
        else {
            return 1;
        }
    }
    else if (rotation == 1) {
        if (!open) {
            return 1;
        }
        else {
            return 2;
        }
    }
    else if (rotation == 2) {
        if (!open) {
            return 2;
        }
        else {
            return 3;
        }
    }
    else if (rotation == 3) {
        if (!open) {
            return 3;
        }
        else {
            return 2;
        }
    }
    else {
        return rotation;
    }
}

bool grounded_block_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    BlockRegistry* reg = neighbors[NEIGHBOR_BOTTOM].reg;
    if (reg) {
        return reg->flags & BLOCK_FLAG_SOLID && reg->flags & BLOCK_FLAG_FULL_BLOCK;
    } else {
		return false;
	}
}

bool plant_block_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    BlockRegistry* reg = neighbors[NEIGHBOR_BOTTOM].reg;
    if (reg) {
        return reg->flags & BLOCK_FLAG_PLANTABLE;
    }
    else {
        return false;
    }
}

bool torch_state_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    BlockRegistry* bottom_brg = neighbors[NEIGHBOR_BOTTOM].reg;
    if (bottom_brg->flags & BLOCK_FLAG_SOLID && bottom_brg->flags & BLOCK_FLAG_FULL_BLOCK) {
        result.block->state = 0;
        return true;
    }
    
    BlockRegistry* right_brg = neighbors[NEIGHBOR_RIGHT].reg;
    if (right_brg->flags & BLOCK_FLAG_SOLID && right_brg->flags & BLOCK_FLAG_FULL_BLOCK) {
        result.block->state = 1;
        return true;
    }

    BlockRegistry* left_brg = neighbors[NEIGHBOR_LEFT].reg;
    if (left_brg->flags & BLOCK_FLAG_SOLID && left_brg->flags & BLOCK_FLAG_FULL_BLOCK) {
        result.block->state = 2;
        return true;
    }

    if (layer == CHUNK_LAYER_FOREGROUND) {
        BlockRegistry* wall_rg = other.reg;
        if (wall_rg) {
            if (wall_rg->flags & BLOCK_FLAG_SOLID && wall_rg->flags & BLOCK_FLAG_FULL_BLOCK) {
                result.block->state = 0;
                return true;
            }
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }

    return false;
}

bool fence_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    bool right = neighbors[NEIGHBOR_RIGHT].block->id == result.block->id;
	bool left = neighbors[NEIGHBOR_LEFT].block->id == result.block->id;

    if (right && left) result.block->state = 3;
    else if (right) result.block->state = 1;
    else if (left) result.block->state = 2;
    else result.block->state = 0;

    return true;
}

bool chest_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    if (result.block->data == NULL) {
        result.block->data = malloc(sizeof(ItemContainer));
        if (!result.block->data) return false;
        item_container_create(result.block->data, "Chest", 3, 10, false);
    }
    return true;
}

bool on_chest_interact(BlockExtraResult result) {
    if (result.block->data != NULL) {
        item_container_open(result.block->data);
        return true;
    }
    return false;
}

void on_chest_destroy(BlockExtraResult result) {
    if (result.block->data != NULL) {
        item_container_free(result.block->data);
        free(result.block->data);
        result.block->data = NULL;
    }
}

bool trapdoor_interact(BlockExtraResult result) {
    uint8_t state = result.block->state;
    state ^= TRAPDOOR_OPEN_MASK;
    result.block->state = state;
    return true;
}


bool falling_block_tick(BlockExtraResult result, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    BlockRegistry* brg = neighbors[NEIGHBOR_BOTTOM].reg;
    if (!brg) return false;
    if (brg->flags & BLOCK_FLAG_REPLACEABLE) {
        BlockInstance temp = *result.block;
        chunk_set_block(neighbors[NEIGHBOR_BOTTOM].chunk, neighbors[NEIGHBOR_BOTTOM].position, temp, layer, false);
        chunk_set_block(result.chunk, result.position, (BlockInstance) { 0, 0 }, layer, false);
        return true;
    }
    return false;
}

bool flow_to_bottom(BlockExtraResult bottom) {
    BlockRegistry* brg = bottom.reg;
    if (!brg) return false;
    if (brg->flags & BLOCK_FLAG_REPLACEABLE && !(brg->flags & BLOCK_FLAG_LIQUID)) {
        chunk_set_block(bottom.chunk, bottom.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(7, true) }, CHUNK_LAYER_FOREGROUND, false);
        return true;
    }
    else if (brg->flags & BLOCK_FLAG_LIQUID && bottom.block->id == BLOCK_WATER_FLOWING) {
        FlowingLiquidState* bottomState = (FlowingLiquidState*)&bottom.block->state;
        if (bottomState->level < 7) {
            chunk_set_block(bottom.chunk, bottom.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(7, true) }, CHUNK_LAYER_FOREGROUND, false);
            return true;
        }
    }

    return false;
}

bool flow_to_sides(BlockExtraResult neighbors[4], int startLevel) {
    if (startLevel < 0) return false;

    NeighborDirection dirs[] = { NEIGHBOR_LEFT, NEIGHBOR_RIGHT };
    bool placed = false;

    for (int i = 0; i < 2; i++) {
        BlockExtraResult neighbor = neighbors[dirs[i]];
        BlockRegistry* neighrg = neighbor.reg;

        if (!neighrg) continue;

        if (neighrg->flags & BLOCK_FLAG_REPLACEABLE && !(neighrg->flags & BLOCK_FLAG_LIQUID)) {
            chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(startLevel - 1, false) }, CHUNK_LAYER_FOREGROUND, false);
            placed = true;
        }
        else if (neighrg->flags & BLOCK_FLAG_LIQUID && neighbor.block->id == BLOCK_WATER_FLOWING) {
            FlowingLiquidState* neighState = (FlowingLiquidState*)&neighbor.block->state;
            if (!neighState->falling && neighState->level < (startLevel - 1)) {
                chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(startLevel - 1, false) }, CHUNK_LAYER_FOREGROUND, false);
                placed = true;
            }
        }
    }

    return placed;
}

bool water_source_tick(BlockExtraResult result, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    if (layer != CHUNK_LAYER_FOREGROUND) return false;

    // Flowing to the bottom
    BlockExtraResult bottom = neighbors[NEIGHBOR_BOTTOM];
    BlockRegistry* brg = bottom.reg;
    if (!brg) return false;
    if (flow_to_bottom(bottom)) return true;

    // Flowing to the sides
    return flow_to_sides(neighbors, 7);
}

bool water_flowing_tick(BlockExtraResult result, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    if (layer != CHUNK_LAYER_FOREGROUND) return false;

    FlowingLiquidState* curState = (FlowingLiquidState*)&result.block->state;
    if (curState->falling == true) {
        BlockExtraResult top = neighbors[NEIGHBOR_TOP];
        BlockRegistry* trg = top.reg;
        if (!trg) return false;

        if (!(trg->flags & BLOCK_FLAG_LIQUID)) {
            chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_AIR, 0 }, CHUNK_LAYER_FOREGROUND, false);
            return true;
        }
        
        BlockExtraResult bottom = neighbors[NEIGHBOR_BOTTOM];
        BlockRegistry* brg = bottom.reg;
        if (!brg) return false;

        bool ret = flow_to_bottom(bottom);
        if (ret) return true;

        if (!(brg->flags & BLOCK_FLAG_REPLACEABLE)) {
            FlowingLiquidState* curState = (FlowingLiquidState*)&result.block->state;
            return flow_to_sides(neighbors, curState->level);
        }
        else {
            return false;
        }
    }
    else {
        if (neighbors[NEIGHBOR_LEFT].block && neighbors[NEIGHBOR_RIGHT].block) {
            if (neighbors[NEIGHBOR_LEFT].block->id == BLOCK_WATER_SOURCE && neighbors[NEIGHBOR_RIGHT].block->id == BLOCK_WATER_SOURCE) {
                chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_WATER_SOURCE, 0 }, CHUNK_LAYER_FOREGROUND, false);
                return true;
            }
        }

        // Flowing to the bottom
        BlockExtraResult bottom = neighbors[NEIGHBOR_BOTTOM];
        BlockRegistry* brg = bottom.reg;
        if (!brg) return false;
        if (flow_to_bottom(bottom)) return true;

        // Flowing to the sides
        if (neighbors[NEIGHBOR_LEFT].block && neighbors[NEIGHBOR_RIGHT].block) {
            if (neighbors[NEIGHBOR_LEFT].block->id == BLOCK_WATER_SOURCE && neighbors[NEIGHBOR_RIGHT].block->id == BLOCK_WATER_SOURCE) {
                chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_WATER_SOURCE, 0 }, CHUNK_LAYER_FOREGROUND, false);
                return true;
            }
        }

        NeighborDirection dirs[] = { NEIGHBOR_LEFT, NEIGHBOR_RIGHT };
        int max_neighbor_level = 0;
        for (int i = 0; i < 2; i++) {
            BlockExtraResult neighbor = neighbors[dirs[i]];
            if (neighbor.block) {
                if (neighbor.block->id == BLOCK_WATER_SOURCE) {
                    max_neighbor_level = 7;
                    break;
                }
                else if (neighbor.block->id == BLOCK_WATER_FLOWING) {
                    FlowingLiquidState* neighState = (FlowingLiquidState*)&neighbor.block->state;
                    if (neighState->level > max_neighbor_level) {
                        max_neighbor_level = neighState->level;
                    }
                }
            }
        }

        int target_level = max_neighbor_level >= 0 ? max_neighbor_level - 1 : 0;

        bool changed = false;

        if (target_level < 0) {
            chunk_set_block(result.chunk, result.position, (BlockInstance) { 0, 0 }, CHUNK_LAYER_FOREGROUND, false);
            changed = true;
        }
        else if (curState->level != target_level) {
            size_t new_state = get_flowing_liquid_state(target_level, curState->falling);
            chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_WATER_FLOWING, new_state }, CHUNK_LAYER_FOREGROUND, false);
            changed = true;
        }

        if (target_level > 0 && !(brg->flags & BLOCK_FLAG_REPLACEABLE)) {
            for (int i = 0; i < 2; i++) {
                BlockExtraResult neighbor = neighbors[dirs[i]];
                BlockRegistry* neighrg = neighbor.reg;

                if (!neighrg) continue;

                int flow_level = target_level - 1;
                if (flow_level < 0) continue;

                if (neighrg->flags & BLOCK_FLAG_REPLACEABLE && !(neighrg->flags & BLOCK_FLAG_LIQUID)) {
                    chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(flow_level, false) }, CHUNK_LAYER_FOREGROUND, false);
                    changed = true;
                }
                else if (neighrg->flags & BLOCK_FLAG_LIQUID && neighbor.block->id == BLOCK_WATER_FLOWING) {
                    FlowingLiquidState* neighState = (FlowingLiquidState*)&neighbor.block->state;
                    if (neighState->level < flow_level) {
                        chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(flow_level, false) }, CHUNK_LAYER_FOREGROUND, false);
                        changed = true;
                    }
                }
            }
        }

        return changed;
    }
}

void chest_overlay_draw(void* data, Vector2 position) {
    if (!data) return;
    ItemContainer* ic = (ItemContainer*)data;
    DrawText(ic->name, position.x, position.y, 32.0f, WHITE);
}