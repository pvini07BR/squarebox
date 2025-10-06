#include "block_state_bitfields.h"

uint8_t get_flowing_liquid_state(uint8_t level, bool falling)
{
	uint8_t value = 0;
	FlowingLiquidState* s = (FlowingLiquidState*)&value;
	s->level = level;
	s->falling = falling;
	return value;
}

uint8_t get_trapdoor_state(uint8_t rotation, bool open) {
	uint8_t value = 0;
	TrapdoorState* s = (TrapdoorState*)&value;
	s->rotation = rotation;
	s->open = open;
	return value;
}

uint8_t get_frame_block_state(uint8_t rotation, uint8_t blockIdx) {
	uint8_t value = 0;
	FrameBlockState* s = (FrameBlockState*)&value;
	s->rotation = rotation;
	s->blockIdx = blockIdx;
	return value;
}