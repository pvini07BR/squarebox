#include "block_functions.h"

#include "block_registry.h"
#include "chunk.h"

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

bool liquid_solver(BlockExtraResult result, BlockExtraResult neighbors[4], bool isWall) {
    //liquid_spread_list_add(&result.chunk->liquidSpreadList, result.idx);
    return true;
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

void on_liquid_destroy(BlockExtraResult result) {
    //liquid_spread_list_remove(&result.chunk->liquidSpreadList, result.idx);
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