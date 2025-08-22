#ifndef ITEM_CONTAINER_H
#define ITEM_CONTAINER_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t item_id;
	uint8_t amount;
} ItemSlot;

typedef struct {
	ItemSlot* items;
	uint8_t rows;
	uint8_t columns;
} ItemContainer;

void item_container_create(ItemContainer* ic, uint8_t rows, uint8_t columns);
ItemSlot* item_container_get_item(ItemContainer* ic, uint8_t row, uint8_t column);
void item_container_set_item(ItemContainer* ic, uint8_t row, uint8_t column, ItemSlot item);
void item_container_open(ItemContainer* ic);
void item_container_close();
bool item_container_is_open();
void item_container_draw();
void item_container_free(ItemContainer* ic);

#endif