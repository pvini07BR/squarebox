#ifndef LIQUID_SPREAD_H
#define LIQUID_SPREAD_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t* indices;
    int count;
    int capacity;
} LiquidSpreadList;

void liquid_spread_list_init(LiquidSpreadList* list);
void liquid_spread_list_free(LiquidSpreadList* list);
void liquid_spread_list_add(LiquidSpreadList* list, uint8_t block_index);
void liquid_spread_list_remove(LiquidSpreadList* list, uint8_t block_index);
void liquid_spread_list_clear(LiquidSpreadList* list);
bool liquid_spread_list_contains(LiquidSpreadList* list, uint8_t block_index);

#endif