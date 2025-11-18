#ifndef BLOCK_STATES_H
#define BLOCK_STATES_H

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

typedef struct {
	uint8_t rotation : 3;
	uint8_t blockIdx : 5;
} FrameBlockState;

typedef struct {
	uint8_t power : 4;
	bool right : 1;
	bool up : 1;
	bool left : 1;
	bool down : 1;
} PowerWireState;

typedef enum {
	BATTERY_STATE_UP,
	BATTERY_STATE_RIGHT,
	BATTERY_STATE_DOWN,
	BATTERY_STATE_LEFT,
	BATTERY_STATE_FORWARD
} BatteryState;

uint8_t get_flowing_liquid_state(uint8_t level, bool falling);
uint8_t get_trapdoor_state(uint8_t rotation, bool open);
uint8_t get_frame_block_state(uint8_t rotation, uint8_t blockIdx);

#endif