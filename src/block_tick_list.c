#include "block_tick_list.h"
#include "types.h"
#include <stdlib.h>

static bool is_equal(BlockTickListEntry* a, BlockTickListEntry* b) {
    return a->instance == b->instance && a->position.x == b->position.x && a->position.y == b->position.y && a->isWall == b->isWall;
}

void block_tick_list_init(BlockTickList* list) {
    list->capacity = CHUNK_AREA;
    list->instances = malloc(sizeof(BlockInstance*) * list->capacity);
    list->count = 0;
}

void block_tick_list_free(BlockTickList* list) {
    if (list->instances) {
        free(list->instances);
        list->instances = NULL;
    }
    list->count = 0;
    list->capacity = 0;
}

void block_tick_list_add(BlockTickList* list, BlockTickListEntry entry) {
    for (int i = 0; i < list->count; i++) {
        if (is_equal(&list->instances[i], &entry)) {
            return;
        }
    }

    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->instances = realloc(list->instances, sizeof(uint8_t) * list->capacity);
    }

    list->instances[list->count++] = entry;
}

void block_tick_list_remove(BlockTickList* list, BlockTickListEntry entry) {
    for (int i = 0; i < list->count; i++) {
        if (is_equal(&list->instances[i], &entry)) {
            list->instances[i] = list->instances[list->count - 1];
            list->count--;
            return;
        }
    }
}

void block_tick_list_clear(BlockTickList* list) {
    list->count = 0;
}

bool block_tick_list_contains(BlockTickList* list, BlockTickListEntry entry) {
    for (int i = 0; i < list->count; i++) {
        if (is_equal(&list->instances[i], &entry)) {
            return true;
        }
    }
    return false;
}