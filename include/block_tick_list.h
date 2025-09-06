#ifndef BLOCK_TICK_LIST_H
#define BLOCK_TICK_LIST_H

#include "types.h"

#include <stdbool.h>

typedef struct {
    BlockInstance* instance;
    Vector2u position;
    bool isWall;
} BlockTickListEntry;

typedef struct {
    BlockTickListEntry* instances;
    unsigned int count;
    unsigned int capacity;
} BlockTickList;

void block_tick_list_init(BlockTickList* list);
void block_tick_list_free(BlockTickList* list);
void block_tick_list_add(BlockTickList* list, BlockTickListEntry entry);
void block_tick_list_remove(BlockTickList* list, BlockTickListEntry entry);
void block_tick_list_clear(BlockTickList* list);
bool block_tick_list_contains(BlockTickList* list, BlockTickListEntry entry);

#endif