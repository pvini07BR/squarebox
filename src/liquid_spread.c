#include "liquid_spread.h"
#include "types.h"
#include <stdlib.h>

void liquid_spread_list_init(LiquidSpreadList* list) {
    list->capacity = CHUNK_AREA;
    list->indices = malloc(sizeof(uint8_t) * list->capacity);
    list->count = 0;
}

void liquid_spread_list_free(LiquidSpreadList* list) {
    if (list->indices) {
        free(list->indices);
        list->indices = NULL;
    }
    list->count = 0;
    list->capacity = 0;
}

void liquid_spread_list_add(LiquidSpreadList* list, uint8_t block_index) {
    for (int i = 0; i < list->count; i++) {
        if (list->indices[i] == block_index) {
            return;
        }
    }

    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->indices = realloc(list->indices, sizeof(uint8_t) * list->capacity);
    }

    list->indices[list->count++] = block_index;
}

void liquid_spread_list_remove(LiquidSpreadList* list, uint8_t block_index) {
    for (int i = 0; i < list->count; i++) {
        if (list->indices[i] == block_index) {
            list->indices[i] = list->indices[list->count - 1];
            list->count--;
            return;
        }
    }
}

void liquid_spread_list_clear(LiquidSpreadList* list) {
    list->count = 0;
}

bool liquid_spread_list_contains(LiquidSpreadList* list, uint8_t block_index) {
    for (int i = 0; i < list->count; i++) {
        if (list->indices[i] == block_index) {
            return true;
        }
    }
    return false;
}