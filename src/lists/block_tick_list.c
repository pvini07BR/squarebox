#include "lists/block_tick_list.h"
#include "types.h"

static inline bool is_equal(const BlockTickListEntry* a, const BlockTickListEntry* b) {
    return a->position.x == b->position.x
        && a->position.y == b->position.y
        && a->layer == b->layer;
}

void block_tick_list_clear(BlockTickList* list) {
    if (!list) return;
    list->count = 0;
}

bool block_tick_list_add(BlockTickList* list, BlockTickListEntry entry) {
    if (!list) return false;

    for (int i = 0; i < list->count; i++) {
        if (is_equal(&list->entries[i], &entry)) {
            return true;
        }
    }

    if (list->count >= TICKS_CAPACITY) {
        return false;
    }

    list->entries[list->count++] = entry;
    return true;
}

bool block_tick_list_remove(BlockTickList* list, BlockTickListEntry entry) {
    if (!list) return false;

    for (int i = 0; i < list->count; i++) {
        if (is_equal(&list->entries[i], &entry)) {
            list->entries[i] = list->entries[list->count - 1];
            list->count--;
            return true;
        }
    }
    return false;
}

bool block_tick_list_remove_by_index(BlockTickList* list, int index) {
    if (!list) return false;
    if (index < 0 || index >= list->count) return false;
    list->entries[index] = list->entries[list->count - 1];
    list->count--;
    return true;
}

bool block_tick_list_contains(BlockTickList* list, BlockTickListEntry entry) {
    if (!list) return false;
    for (int i = 0; i < list->count; i++) {
        if (is_equal(&list->entries[i], &entry)) {
            return true;
        }
    }
    return false;
}

int block_tick_list_count(const BlockTickList* list) {
    return list ? list->count : 0;
}

bool block_tick_list_get(const BlockTickList* list, int index, BlockTickListEntry* out) {
    if (!list || !out) return false;
    if (index < 0 || index >= list->count) return false;
    *out = list->entries[index];
    return true;
}