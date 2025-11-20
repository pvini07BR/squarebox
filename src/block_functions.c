#include "block_functions.h"

#include "item_container.h"
#include "sign_editor.h"
#include "raylib.h"
#include "registries/block_registry.h"
#include "registries/item_registry.h"
#include "block_states.h"
#include "chunk.h"
#include "types.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raymath.h>

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
    }

    return false;
}

bool fence_resolver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    bool right = neighbors[NEIGHBOR_RIGHT].block->id == result.block->id;
	bool left = neighbors[NEIGHBOR_LEFT].block->id == result.block->id;
    bool up = neighbors[NEIGHBOR_TOP].block->id == result.block->id;

    result.block->state = (uint8_t)((up << 2) | (left << 1) | right);

    return true;
}

bool chest_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    if (result.block->data == NULL) {
        result.block->data = malloc(sizeof(ItemContainer));
        if (!result.block->data) return false;
		char* str = calloc(6, sizeof(char));
		strcpy(str, "Chest");
        item_container_create(result.block->data, str, 3, 10, false);
    }
    return true;
}

bool on_chest_interact(BlockExtraResult result, ItemSlot holdingItem) {
    if (result.block->data != NULL) {
        item_container_open(result.block->data);
        return true;
    }
    return false;
}

bool sign_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    bool valid = false;

    BlockRegistry* bottom_brg = neighbors[NEIGHBOR_BOTTOM].reg;
    if (bottom_brg->flags & BLOCK_FLAG_SOLID && bottom_brg->flags & BLOCK_FLAG_FULL_BLOCK) {
        result.block->state = 0;
        valid = true;
    } else if (layer == CHUNK_LAYER_FOREGROUND) {
        BlockRegistry* wall_rg = other.reg;
        if (wall_rg) {
            if (wall_rg->flags & BLOCK_FLAG_SOLID && wall_rg->flags & BLOCK_FLAG_FULL_BLOCK) {
                result.block->state = 1;
                valid = true;
            }
        }
    }

    if (valid && result.block->data == NULL) {
        result.block->data = malloc(sizeof(SignLines));
        if (result.block->data) {
            SignLines* lines = result.block->data;
            for (int i = 0; i < SIGN_LINE_COUNT; i++) {
                for (int j = 0; j < SIGN_LINE_LENGTH; j++) {
                    lines->lines[i][j] = '\0';
                }
            }
            sign_editor_open(lines);
        }
        else {
            valid = false;
        }
    }

    return valid;
}

bool trapdoor_interact(BlockExtraResult result, ItemSlot holdingItem) {
    TrapdoorState* state = (TrapdoorState*) & result.block->state;
    state->open = !state->open;
    return true;
}

void set_power_wire_dir(NeighborDirection dir, PowerWireState* state, bool value) {
    switch (dir) {
        case NEIGHBOR_TOP: state->up = value; break;
        case NEIGHBOR_RIGHT: state->right = value; break;
        case NEIGHBOR_BOTTOM: state->down = value; break;
        case NEIGHBOR_LEFT: state->left = value; break;
        default: break;
    }
}

bool power_wire_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    PowerWireState* s = (PowerWireState*)&result.block->state;

    int old_power = (int)s->power;
    int new_power = 0;

    // First calculate the directions the wire should be connected
    for (int i = 0; i < 4; i++) {
        uint8_t blockId = neighbors[i].block->id;

        if (blockId == BLOCK_POWER_WIRE || blockId == BLOCK_POWERED_LAMP) {
            set_power_wire_dir(i, s, true);
        } else if (blockId == BLOCK_BATTERY) {
            LogLikeBlockState batState = (LogLikeBlockState)neighbors[i].block->state;
            if (batState == LOGLIKE_BLOCK_STATE_VERTICAL) {
                if (i == NEIGHBOR_BOTTOM) set_power_wire_dir(i, s, true);
                if (i == NEIGHBOR_TOP) set_power_wire_dir(i, s, true);
            } else if (batState == LOGLIKE_BLOCK_STATE_HORIZONTAL) {
                if (i == NEIGHBOR_LEFT) set_power_wire_dir(i, s, true);
                if (i == NEIGHBOR_RIGHT) set_power_wire_dir(i, s, true);
            }
        } else if (blockId == BLOCK_POWER_REPEATER) {
            PowerRepeaterState* repState = (PowerRepeaterState*)&neighbors[i].block->state;
            if (repState->rotation % 2 == 0) {
                if (i == NEIGHBOR_LEFT) set_power_wire_dir(i, s, true);
                if (i == NEIGHBOR_RIGHT) set_power_wire_dir(i, s, true);
            } else {
                if (i == NEIGHBOR_TOP) set_power_wire_dir(i, s, true);
                if (i == NEIGHBOR_BOTTOM) set_power_wire_dir(i, s, true);
            }
        } else {
            set_power_wire_dir(i, s, false);
        }
    }

    // Now make it so only the connected neighbors will get checked

    BlockExtraResult* checks[5];

    if (s->up)
        checks[NEIGHBOR_TOP] = &neighbors[NEIGHBOR_TOP];
    else
        checks[NEIGHBOR_TOP] = NULL;

    if (s->right)
        checks[NEIGHBOR_RIGHT] = &neighbors[NEIGHBOR_RIGHT];    
    else
        checks[NEIGHBOR_RIGHT] = NULL;

    if (s->down)
        checks[NEIGHBOR_BOTTOM] = &neighbors[NEIGHBOR_BOTTOM];
    else
        checks[NEIGHBOR_BOTTOM] = NULL;

    if (s->left)
        checks[NEIGHBOR_LEFT] = &neighbors[NEIGHBOR_LEFT];
    else
        checks[NEIGHBOR_LEFT] = NULL;

    checks[4] = &other;

    for (int i = 0; i < 5; i++) {
        if (!checks[i]) continue;
        BlockExtraResult* nb = checks[i];
        BlockRegistry* nrg = (BlockRegistry*)nb->reg;
        if (!nrg) continue;

        if (nb->block->id == BLOCK_BATTERY) {
            // Check if the battery is from the other layer
            if (nb->block == other.block) {
                LogLikeBlockState batState = (LogLikeBlockState)nb->block->state;
                if (batState == LOGLIKE_BLOCK_STATE_FORWARD) {
                    new_power = 15;
                    break;
                }
            } else {
                new_power = 15;
                break;
            }
        } else if (nb->block->id == BLOCK_POWER_REPEATER && i < 4) {
            PowerRepeaterState* repState = (PowerRepeaterState*)&nb->block->state;
            if (repState->powered && 
                (
                    (repState->rotation == 0 && i == NEIGHBOR_LEFT) ||
                    (repState->rotation == 1 && i == NEIGHBOR_TOP) ||
                    (repState->rotation == 2 && i == NEIGHBOR_RIGHT) ||
                    (repState->rotation == 3 && i == NEIGHBOR_BOTTOM)
                )
            ) {
                new_power = 15;
                break;
            }
        }
       
        if (nb->block->id == BLOCK_POWER_WIRE) {
            PowerWireState* ns = (PowerWireState*)&nb->block->state;

            int neighbor_power = (int)ns->power;
            if (neighbor_power > 0) {
                int candidate = neighbor_power - 1;
                if (candidate > new_power) new_power = candidate;
            }
        }
    }

    new_power = Clamp(new_power, 0, 15);

    if (new_power != old_power) {
        s->power = new_power;
    }

    if (new_power != old_power) {
        s->power = (uint8_t)new_power;

        // Propagate power to neighbors
        for (int i = 0; i < 4; i++) {
            BlockExtraResult nb = neighbors[i];
            BlockRegistry* reg = (BlockRegistry*)nb.reg;
            if (!reg) continue;

            if (reg->flags & BLOCK_FLAG_POWER_TRIGGERED) {
                chunk_solve_block(nb.chunk, nb.position, layer);
            }
        }

        // Also propagate to the wire in the opposite layer
        BlockRegistry* other_reg = (BlockRegistry*)other.reg;
        if (other_reg) {
            if (other_reg->flags & BLOCK_FLAG_POWER_TRIGGERED) {
                ChunkLayerEnum otherLayer = layer == CHUNK_LAYER_FOREGROUND ? CHUNK_LAYER_BACKGROUND : CHUNK_LAYER_FOREGROUND;
                chunk_solve_block(result.chunk, result.position, otherLayer);
            }
        }
    }

    return true;
}

bool power_repeater_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    PowerRepeaterState* s = (PowerRepeaterState*)&result.block->state;
    s->powered = false;

    PowerWireState* input = NULL;
    PowerWireState* output = NULL;

    for (int i = 0; i < 4; i++) {
        if (neighbors[i].block->id != BLOCK_POWER_WIRE) continue;
        PowerWireState* wireState = (PowerWireState*)&neighbors[i].block->state;

        if (s->rotation == 0) {
            if (i == NEIGHBOR_LEFT) {
                input = wireState;
            } else if (i == NEIGHBOR_RIGHT) {
                output = wireState;
            }
        } else if (s->rotation == 1) {
            if (i == NEIGHBOR_TOP) {
                input = wireState;
            } else if (i == NEIGHBOR_BOTTOM) {
                output = wireState;
            }
        } else if (s->rotation == 2) {
            if (i == NEIGHBOR_LEFT) {
                output = wireState;
            } else if (i == NEIGHBOR_RIGHT) {
                input = wireState;
            }
        } else if (s->rotation == 3) {
            if (i == NEIGHBOR_TOP) {
                output = wireState;
            } else if (i == NEIGHBOR_BOTTOM) {
                input = wireState;
            }
        }
    }

    if (input) {
        if (input->power > 0) {
            s->powered = true;
        }
    }
    
    return true;
}

bool powered_lamp_solver(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    result.block->state = 0;
    
    for (int i = 0; i < 4; i++) {
        if (neighbors[i].block->id == BLOCK_POWER_WIRE) {
            PowerWireState* s = (PowerWireState*)&neighbors[i].block->state;
            if (s->power > 0) {
                result.block->state = 1;
                break;
            }
        }
    }

    if (other.block->id == BLOCK_POWER_WIRE) {
        PowerWireState* s = (PowerWireState*)&other.block->state;
        if (s->power > 0) {
            result.block->state = 1;
        }
    }
    
    return true;
}

bool sign_interact(BlockExtraResult result, ItemSlot holdingItem) {
    if (result.block->data != NULL) {
        SignLines* lines = result.block->data;
        sign_editor_open(lines);
        return true;
    }
    return false;
}

bool frame_block_interact(BlockExtraResult result, ItemSlot holdingItem) {
    FrameBlockState* s = (FrameBlockState*)&result.block->state;
    if (s->blockIdx <= 0) {
        ItemRegistry* irg = ir_get_item_registry(holdingItem.item_id);
        BlockRegistry* brg = br_get_block_registry(irg->blockId);

        if (irg->blockId > 0 && irg->blockId != BLOCK_SLAB_FRAME && (brg->flags & BLOCK_FLAG_SOLID) && (brg->flags & BLOCK_FLAG_FULL_BLOCK)) {
            s->blockIdx = irg->blockId;
            return true;
        }
    }

    return false;
}

bool falling_block_tick(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    BlockRegistry* brg = neighbors[NEIGHBOR_BOTTOM].reg;
    if (!brg) return false;
    if (brg->flags & BLOCK_FLAG_REPLACEABLE) {
        BlockInstance temp = *result.block;
        chunk_set_block(neighbors[NEIGHBOR_BOTTOM].chunk, neighbors[NEIGHBOR_BOTTOM].position, temp, layer, false);
        chunk_set_block(result.chunk, result.position, (BlockInstance) { 0, 0, NULL }, layer, false);
        return true;
    }
    return false;
}

bool flow_to_bottom(BlockExtraResult bottom) {
    BlockRegistry* brg = bottom.reg;
    if (!brg) return false;
    if (brg->flags & BLOCK_FLAG_REPLACEABLE && !(brg->flags & BLOCK_FLAG_LIQUID)) {
        chunk_set_block(bottom.chunk, bottom.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(7, true), NULL }, CHUNK_LAYER_FOREGROUND, false);
        return true;
    }
    else if (brg->flags & BLOCK_FLAG_LIQUID && bottom.block->id == BLOCK_WATER_FLOWING) {
        FlowingLiquidState* bottomState = (FlowingLiquidState*)&bottom.block->state;
        if (bottomState->level < 7) {
            chunk_set_block(bottom.chunk, bottom.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(7, true), NULL }, CHUNK_LAYER_FOREGROUND, false);
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
            chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(startLevel - 1, false), NULL }, CHUNK_LAYER_FOREGROUND, false);
            placed = true;
        }
        else if (neighrg->flags & BLOCK_FLAG_LIQUID && neighbor.block->id == BLOCK_WATER_FLOWING) {
            FlowingLiquidState* neighState = (FlowingLiquidState*)&neighbor.block->state;
            if (!neighState->falling && neighState->level < (startLevel - 1)) {
                chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(startLevel - 1, false), NULL }, CHUNK_LAYER_FOREGROUND, false);
                placed = true;
            }
        }
    }

    return placed;
}

bool water_source_tick(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    if (layer != CHUNK_LAYER_FOREGROUND) return false;

    // Flowing to the bottom
    BlockExtraResult bottom = neighbors[NEIGHBOR_BOTTOM];
    BlockRegistry* brg = bottom.reg;
    if (!brg) return false;
    if (flow_to_bottom(bottom)) return true;

    // Flowing to the sides
    return flow_to_sides(neighbors, 7);
}

bool water_flowing_tick(BlockExtraResult result, BlockExtraResult other, BlockExtraResult neighbors[4], ChunkLayerEnum layer) {
    if (layer != CHUNK_LAYER_FOREGROUND) return false;

    FlowingLiquidState* curState = (FlowingLiquidState*)&result.block->state;
    if (curState->falling == true) {
        BlockExtraResult top = neighbors[NEIGHBOR_TOP];
        BlockRegistry* trg = top.reg;
        if (!trg) return false;

        if (!(trg->flags & BLOCK_FLAG_LIQUID)) {
            chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_AIR, 0, NULL }, CHUNK_LAYER_FOREGROUND, false);
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
                chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_WATER_SOURCE, 0, NULL }, CHUNK_LAYER_FOREGROUND, false);
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
                chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_WATER_SOURCE, 0, NULL }, CHUNK_LAYER_FOREGROUND, false);
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
            chunk_set_block(result.chunk, result.position, (BlockInstance) { 0, 0, NULL }, CHUNK_LAYER_FOREGROUND, false);
            changed = true;
        }
        else if (curState->level != target_level) {
            size_t new_state = get_flowing_liquid_state(target_level, curState->falling);
            chunk_set_block(result.chunk, result.position, (BlockInstance) { BLOCK_WATER_FLOWING, new_state, NULL }, CHUNK_LAYER_FOREGROUND, false);
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
                    chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(flow_level, false), NULL }, CHUNK_LAYER_FOREGROUND, false);
                    changed = true;
                }
                else if (neighrg->flags & BLOCK_FLAG_LIQUID && neighbor.block->id == BLOCK_WATER_FLOWING) {
                    FlowingLiquidState* neighState = (FlowingLiquidState*)&neighbor.block->state;
                    if (neighState->level < flow_level) {
                        chunk_set_block(neighbor.chunk, neighbor.position, (BlockInstance) { BLOCK_WATER_FLOWING, get_flowing_liquid_state(flow_level, false), NULL }, CHUNK_LAYER_FOREGROUND, false);
                        changed = true;
                    }
                }
            }
        }

        return changed;
    }
}

void sign_text_draw(void* data, Vector2 position, uint8_t state) {
    if (data != NULL) {
        SignLines* lines = data;
        float fontSize = 4.0f;
        float disloc = state == 1 ? 5.0f : 0.0f;
        Vector2 textPos = (Vector2){
            position.x + (TILE_SIZE / 2.0f),
            position.y + TILE_SIZE / 8.0f + disloc
        };

        for (int i = 0; i < SIGN_LINE_COUNT; i++) {;
            Vector2 textMeasure = MeasureTextEx(GetFontDefault(), lines->lines[i], fontSize, fontSize / 8.0f);

            DrawTextPro(
                GetFontDefault(),
                lines->lines[i],
                textPos,
                (Vector2) { textMeasure.x / 2.0f, 0.0f },
                0.0f,
                fontSize,
                fontSize / 8.0f,
                BLACK
            );

            textPos.y += fontSize + 1.0f;
        }
    }
}

void chest_free_data(void* data) {
    if (data != NULL) {
        item_container_free(data);
        free(data);
        data = NULL;
    }
}

void sign_free_data(void* data) {
    if (data != NULL) {
        free(data);
        data = NULL;
    }
}