#ifndef BLOCK_STATE_BITFIELDS_H
#define BLOCK_STATE_BITFIELDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TRAPDOOR_ROTATION_MASK 0x07
#define TRAPDOOR_OPEN_MASK 0x08

typedef struct {
	uint8_t level : 3;
	bool falling : 1;
} FlowingLiquidState;

size_t get_flowing_liquid_state(uint8_t level, bool falling);

#endif