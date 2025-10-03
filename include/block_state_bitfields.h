#ifndef BLOCK_STATE_BITFIELDS_H
#define BLOCK_STATE_BITFIELDS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint8_t level : 3;
	bool falling : 1;
} FlowingLiquidState;

typedef struct {
	uint8_t rotation : 3;
	bool open : 1;
} TrapdoorState;

uint8_t get_flowing_liquid_state(uint8_t level, bool falling);
uint8_t get_trapdoor_state(uint8_t rotation, bool open);

#endif