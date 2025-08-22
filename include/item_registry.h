#ifndef ITEM_REGISTRY_H
#define ITEM_REGISTRY_H

#include <stdint.h>

#define ITEM_COUNT 3

typedef enum {
	ITEM_TYPE_NORMAL,
	ITEM_TYPE_BLOCK
} ItemType;

typedef struct {
	const char* name;
	ItemType type;
	union {
		uint8_t atlas_idx;
		uint8_t blockId;
	};
} ItemRegistry;

void item_registry_init();
ItemRegistry* ir_get_item_registry(size_t idx);
void item_registry_free();

#endif