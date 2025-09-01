#include "block_state_bitfields.h"

size_t get_flowing_liquid_state(uint8_t level, bool falling)
{
	size_t value = 0;
	FlowingLiquidState* s = (FlowingLiquidState*)&value;
	s->level = level;
	s->falling = falling;
	return value;
}
