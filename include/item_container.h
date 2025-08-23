#ifndef ITEM_CONTAINER_H
#define ITEM_CONTAINER_H

#include <stdbool.h>
#include <stdint.h>

#include <raylib.h>

#define ITEM_SLOT_SIZE 42
#define ITEM_SLOT_GAP 8

typedef struct {
	uint8_t item_id;
	uint8_t amount;
} ItemSlot;

typedef struct {
	const char* name;
	ItemSlot* items;
	uint8_t rows;
	uint8_t columns;
} ItemContainer;

void item_container_create(ItemContainer* ic, const char* name, uint8_t rows, uint8_t columns);
ItemSlot item_container_get_item(ItemContainer* ic, uint8_t row, uint8_t column);
void item_container_set_item(ItemContainer* ic, uint8_t row, uint8_t column, ItemSlot item);
Vector2 item_container_get_size(ItemContainer* ic);
void item_container_open(ItemContainer* ic);
void item_container_close();
bool item_container_is_open();
void item_container_draw();
void item_container_free(ItemContainer* ic);

void draw_item(ItemSlot* is, int x, int y);

void init_inventory();
ItemContainer* get_inventory();
void inventory_set_item(uint8_t row, uint8_t column, ItemSlot item);
ItemSlot inventory_get_item(uint8_t row, uint8_t column);
void free_inventory();

#endif