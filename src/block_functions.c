#include "block_functions.h"

#include "block_registry.h"

bool grounded_block_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
    BlockRegistry* bottom_brg = br_get_block_registry(neighbors[NEIGHBOR_BOTTOM].id);
    if (bottom_brg) {
        return bottom_brg->flags & BLOCK_FLAG_SOLID && bottom_brg->flags & BLOCK_FLAG_FULL_BLOCK;
    } else {
		return false;
	}
}

bool plant_block_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
    BlockRegistry* bottom_brg = br_get_block_registry(neighbors[NEIGHBOR_BOTTOM].id);
    if (bottom_brg) {
        return bottom_brg->flags & BLOCK_FLAG_SOLID && bottom_brg->flags & BLOCK_FLAG_FULL_BLOCK && bottom_brg->flags & BLOCK_FLAG_PLANTABLE;
    }
    else {
        return false;
    }
}

bool torch_state_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
    BlockRegistry* bottom_brg = br_get_block_registry(neighbors[NEIGHBOR_BOTTOM].id);
    if (bottom_brg->flags & BLOCK_FLAG_SOLID && bottom_brg->flags & BLOCK_FLAG_FULL_BLOCK) {
        inst->state = 0;
        return true;
    }
    
    BlockRegistry* right_brg = br_get_block_registry(neighbors[NEIGHBOR_RIGHT].id);
    if (right_brg->flags & BLOCK_FLAG_SOLID && right_brg->flags & BLOCK_FLAG_FULL_BLOCK) {
        inst->state = 1;
        return true;
    }

    BlockRegistry* left_brg = br_get_block_registry(neighbors[NEIGHBOR_LEFT].id);
    if (left_brg->flags & BLOCK_FLAG_SOLID && left_brg->flags & BLOCK_FLAG_FULL_BLOCK) {
        inst->state = 2;
        return true;
    }

    return false;
}

bool fence_resolver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
    bool right = neighbors[NEIGHBOR_RIGHT].id == inst->id;
	bool left = neighbors[NEIGHBOR_LEFT].id == inst->id;

    if (right && left) inst->state = 3;
    else if (right) inst->state = 1;
    else if (left) inst->state = 2;
    else inst->state = 0;

    return true;
}

bool on_chest_interact(BlockInstance* inst, Chunk* chunk) {
    if (inst->state >= 0) {
        item_container_open(container_vector_get(&chunk->containerVec, inst->state));
        return true;
    }
    return false;
}

bool chest_solver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
    inst->state = container_vector_add(&chunk->containerVec, "Chest", 3, 10, false);
    if (inst->state >= 0) return true;
    return false;
}

void on_chest_destroy(BlockInstance* inst, Chunk* chunk, uint8_t idx) {
    if (inst->state >= 0) {
        container_vector_remove(&chunk->containerVec, inst->state);
        inst->state = -1;
    }
}

bool liquid_solver(BlockInstance* inst, Chunk* chunk, uint8_t idx, BlockInstance neighbors[4], bool isWall) {
	liquid_spread_list_add(&chunk->liquidSpreadList, idx);
    return true;
}

void on_liquid_destroy(BlockInstance* inst, Chunk* chunk, uint8_t idx) {
    liquid_spread_list_remove(&chunk->liquidSpreadList, idx);
}