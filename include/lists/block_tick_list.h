#ifndef BLOCK_TICK_LIST_H
#define BLOCK_TICK_LIST_H

#include "types.h"

#include <stdbool.h>

#define TICKS_CAPACITY CHUNK_AREA * 2

typedef struct {
    Vector2u position;
    bool isWall;
} BlockTickListEntry;

typedef struct {
    BlockTickListEntry entries[TICKS_CAPACITY];
    unsigned int count;
} BlockTickList;

void block_tick_list_clear(BlockTickList* list);
bool block_tick_list_add(BlockTickList* list, BlockTickListEntry entry);
bool block_tick_list_remove(BlockTickList* list, BlockTickListEntry entry);
bool block_tick_list_remove_by_index(BlockTickList* list, int index);
bool block_tick_list_contains(BlockTickList* list, BlockTickListEntry entry);
int block_tick_list_count(const BlockTickList* list);
bool block_tick_list_get(const BlockTickList* list, int index, BlockTickListEntry* out);

#endif