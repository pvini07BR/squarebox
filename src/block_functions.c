#include "block_functions.h"

#include "block_registry.h"
#include "block_state_bitfields.h"
#include "chunk.h"

#include <stdio.h>

bool grounded_block_resolver(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall) {
    BlockRegistry* reg = neighbors[NEIGHBOR_BOTTOM].reg;
    if (reg) {
        return reg->flags & BLOCK_FLAG_SOLID && reg->flags & BLOCK_FLAG_FULL_BLOCK;
    } else {
		return false;
	}
}

bool plant_block_resolver(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall) {
    BlockRegistry* reg = neighbors[NEIGHBOR_BOTTOM].reg;
    if (reg) {
        return reg->flags & BLOCK_FLAG_PLANTABLE;
    }
    else {
        return false;
    }
}

bool torch_state_resolver(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall) {
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

    return false;
}

bool fence_resolver(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall) {
    bool right = neighbors[NEIGHBOR_RIGHT].block->id == result.block->id;
	bool left = neighbors[NEIGHBOR_LEFT].block->id == result.block->id;

    if (right && left) result.block->state = 3;
    else if (right) result.block->state = 1;
    else if (left) result.block->state = 2;
    else result.block->state = 0;

    return true;
}

bool chest_solver(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall) {
    if (result.block->state <= 0) {
        result.block->state = container_vector_add(&result.chunk->containerVec, "Chest", 3, 10, false);
        if (result.block->state > 0) return true;
        else { return false; }
    }
    else {
        return true;
    }
}

bool on_chest_interact(BlockExtraResult result) {
    if (result.block->state > 0) {
        item_container_open(container_vector_get(&result.chunk->containerVec, result.block->state - 1));
        return true;
    }
    return false;
}


void on_chest_destroy(BlockExtraResult result) {
    if (result.block->state > 0) {
        container_vector_remove(&result.chunk->containerVec, result.block->state - 1);
    }
}

bool falling_block_tick(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall) {
    BlockRegistry* brg = neighbors[NEIGHBOR_BOTTOM].reg;
    if (!brg) return false;
    if (brg->flags & BLOCK_FLAG_REPLACEABLE) {
        BlockInstance temp = *result.block;
        chunk_set_block(neighbors[NEIGHBOR_BOTTOM].chunk, neighbors[NEIGHBOR_BOTTOM].position, temp, isWall, false);
        chunk_set_block(result.chunk, result.position, (BlockInstance) { 0, 0 }, isWall, false);
        return true;
    }
    return false;
}

bool flow_to_bottom(BlockExtraResult bottom) {
    BlockRegistry* brg = bottom.reg;
    if (!brg) return false;
    if (brg->flags & BLOCK_FLAG_REPLACEABLE && !(brg->flags & BLOCK_FLAG_LIQUID)) {
        chunk_set_block(bottom.chunk, bottom.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(7, true) }, false, false);
        return true;
    }
    else if (brg->flags & BLOCK_FLAG_LIQUID && bottom.block->id == BLOCK_WATER_FLOWING) {
        FlowingLiquidState* bottomState = &bottom.block->state;
        if (bottomState->level < 7) {
            chunk_set_block(bottom.chunk, bottom.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(7, true) }, false, false);
            return true;
        }
    }

    return false;
}

bool flow_to_sides(BlockExtraResult neighbors[4]) {
    NeighborDirection dirs[] = { NEIGHBOR_LEFT, NEIGHBOR_RIGHT };
    bool placed = false;

    for (int i = 0; i < 2; i++) {
        BlockExtraResult neighbor = neighbors[dirs[i]];
        BlockRegistry* neighrg = neighbor.reg;

        if (!neighrg) continue;

        if (neighrg->flags & BLOCK_FLAG_REPLACEABLE && !(neighrg->flags & BLOCK_FLAG_LIQUID)) {
            chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(6, false) }, false, false);
            placed = true;
        }
        else if (neighrg->flags & BLOCK_FLAG_LIQUID && neighbor.block->id == BLOCK_WATER_FLOWING) {
            FlowingLiquidState* neighState = &neighbor.block->state;
            if (neighState->level < 6) {
                chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(6, false) }, false, false);
                placed = true;
            }
        }
    }

    return placed;
}

bool water_source_tick(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall) {
    if (isWall) return false;

    // Flowing to the bottom
    BlockExtraResult bottom = neighbors[NEIGHBOR_BOTTOM];
    BlockRegistry* brg = bottom.reg;
    if (!brg) return false;
    if (flow_to_bottom(bottom)) return true;

    // Flowing to the sides
    if (flow_to_sides(neighbors)) return true;
    return false;
}

bool water_flowing_tick(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall) {
    if (isWall) return false;

    FlowingLiquidState* curState = &result.block->state;
    if (curState->falling == true) {
        BlockExtraResult bottom = neighbors[NEIGHBOR_BOTTOM];
        BlockRegistry* brg = bottom.reg;
        if (!brg) return false;
        if (flow_to_bottom(bottom)) return true;

        BlockExtraResult top = neighbors[NEIGHBOR_TOP];
        BlockRegistry* trg = top.reg;
        if (!trg) return false;
        if (!(trg->flags & BLOCK_FLAG_LIQUID)) {
            chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_AIR, 0 }, false, false);
            return true;
        }
    }
    else {
        // Flowing to the bottom
        BlockExtraResult bottom = neighbors[NEIGHBOR_BOTTOM];
        BlockRegistry* brg = bottom.reg;
        if (!brg) return false;
        if (flow_to_bottom(bottom)) return true;

        // Flowing to the sides
        if (!(brg->flags & BLOCK_FLAG_REPLACEABLE)) {
            if (flow_to_sides(neighbors)) return true;
        }
        return false;
    }

    /*if (neighbors[NEIGHBOR_LEFT].block && neighbors[NEIGHBOR_RIGHT].block) {
        if (neighbors[NEIGHBOR_LEFT].block->id == BLOCK_WATER_SOURCE && neighbors[NEIGHBOR_RIGHT].block->id == BLOCK_WATER_SOURCE) {
            chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_WATER_SOURCE, 0 }, false, false);
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
                FlowingLiquidState* neighState = &neighbor.block->state;
                if (neighState->level > max_neighbor_level) {
                    max_neighbor_level = neighState->level;
                }
            }
        }
    }

    int target_level = max_neighbor_level > 0 ? max_neighbor_level - 1 : 0;

    bool changed = false;

    if (target_level == 0) {
        chunk_set_block(result.chunk, result.position, (BlockInstance) { 0, 0 }, false, false);
        changed = true;
    }
    else if (curState->level != target_level) {
        size_t new_state = get_flowing_liquid_state(target_level, curState->falling);
        chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_WATER_FLOWING, new_state }, false, false);
        changed = true;
    }

    if (target_level > 0) {
        for (int i = 0; i < 2; i++) {
            BlockExtraResult neighbor = neighbors[dirs[i]];
            BlockRegistry* neighrg = neighbor.reg;

            if (!neighrg) continue;

            int flow_level = target_level - 1;
            if (flow_level <= 0) continue;

            if (neighrg->flags & BLOCK_FLAG_REPLACEABLE && !(neighrg->flags & BLOCK_FLAG_LIQUID)) {
                chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(flow_level, false) }, false, false);
                changed = true;
            }
            else if (neighrg->flags & BLOCK_FLAG_LIQUID && neighbor.block->id == BLOCK_WATER_FLOWING) {
                FlowingLiquidState* neighState = &neighbor.block->state;
                if (neighState->level < flow_level) {
                    chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(flow_level, false) }, false, false);
                    changed = true;
                }
            }
        }
    }

    return changed;*/
}