#include "item_container.h"

#include <stdlib.h>
#include <stdio.h>

#include <raylib.h>
#include <raymath.h>

#include "block_registry.h"
#include "item_registry.h"
#include "texture_atlas.h"
#include "defines.h"

const float TITLE_SIZE = 18.0f;
const Color BG_COLOR = { 200, 200, 200, 255 };
const Color SLOT_COLOR = { 130, 130, 130, 255 };
const Color SLOT_HOVER_COLOR = { 255, 255, 255, 128 };
const Color TITLE_COLOR = { 80, 80, 80, 255 };

static ItemContainer* openedContainer = NULL;

// Variables for holding items
static ItemSlot grabbedItem = { 0, 0 };
static int lastSlotIdx = -1;
static ItemContainer* lastContainer = NULL;

static ItemContainer inventory;

void init_inventory() {
	item_container_create(&inventory, "Inventory", 1, 9, false);
}

ItemContainer* get_inventory()
{
	return &inventory;
}

void inventory_set_item(uint8_t row, uint8_t column, ItemSlot item)
{
	if (row < 0 || row >= inventory.rows || column < 0 || column >= inventory.columns) return;
	if (item.amount <= 0) item.amount = 1;
	int i = column + (row * inventory.columns);
	inventory.items[i] = item;
}

ItemSlot inventory_get_item(uint8_t row, uint8_t column)
{
	if (row < 0 || row >= inventory.rows || column < 0 || column >= inventory.columns) return (ItemSlot){ 0, 0 };
	int i = column + (row * inventory.columns);
	return inventory.items[i];
}

void free_inventory()
{
	item_container_free(&inventory);
}

void item_container_create(ItemContainer* ic, const char* name, uint8_t rows, uint8_t columns, bool immutable)
{
	if (!ic) return;
	ic->name = name;
	ic->rows = rows;
	ic->columns = columns;
	ic->immutable = immutable;
	ic->items = calloc(ic->rows * ic->columns, sizeof(ItemSlot));
	if (ic->items) for (int i = 0; i < (ic->rows * ic->columns); i++)
		ic->items[i] = (ItemSlot){ 0, 0 };
}

ItemSlot item_container_get_item(ItemContainer* ic, uint8_t row, uint8_t column)
{
	if (!ic) return;
	if (row < 0 || row >= ic->rows || column < 0 || column >= ic->columns) return (ItemSlot) { 0, 0 };
	int i = column + (row * ic->columns);
	return ic->items[i];
}

void item_container_set_item(ItemContainer* ic, uint8_t row, uint8_t column, ItemSlot item)
{
	if (!ic) return;
	if (row < 0 || row >= ic->rows || column < 0 || column >= ic->columns) return;
	if (item.amount <= 0) item.amount = 1;
	int i = column + (row * ic->columns);
	ic->items[i] = item;
}

Vector2 item_container_get_size(ItemContainer* ic)
{
	if (!ic) return Vector2Zero();

	Vector2 titleSize = MeasureTextEx(GetFontDefault(), ic->name, TITLE_SIZE, 0.0f);

	return (Vector2) {
		.x = (ic->columns * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP)) + ITEM_SLOT_GAP,
		.y = (ic->rows * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP)) + ITEM_SLOT_GAP + (titleSize.y + ITEM_SLOT_GAP)
	};
}

void item_container_open(ItemContainer* ic)
{
	if (!ic) return;
	openedContainer = ic;
}

void item_container_close() {
	// If closing the item container while having a grabbed item,
	// add it back to the slot it came from
	if (lastContainer && lastSlotIdx >= 0) {
		if (!lastContainer->immutable) {
			lastContainer->items[lastSlotIdx].item_id = grabbedItem.item_id;
			lastContainer->items[lastSlotIdx].amount += grabbedItem.amount;
		}

		grabbedItem.item_id = 0;
		grabbedItem.amount = 0;

		lastSlotIdx = -1;
		lastContainer = NULL;
	}
	openedContainer = NULL;
}

bool item_container_is_open() { return openedContainer != NULL; }

void draw_item(ItemSlot* is, int x, int y) {
	if (is->item_id <= 0) return;
	ItemRegistry* ir = ir_get_item_registry(is->item_id);

	DrawTexturePro(
		texture_atlas_get(),
		texture_atlas_get_rect(ir->atlas_idx, false, false),
		(Rectangle) {
			.x = x + ((ITEM_SLOT_SIZE - TILE_SIZE) / 2),
			.y = y + ((ITEM_SLOT_SIZE - TILE_SIZE) / 2),
			.width = TILE_SIZE,
			.height = TILE_SIZE
		},
		Vector2Zero(),
		0.0f,
		WHITE
	);

	if (is->amount > 1) {
		char amountStr[4];
		sprintf(amountStr, "%d", is->amount);
		Vector2 textSize = MeasureTextEx(GetFontDefault(), amountStr, 18.0f, 0.0f);

		DrawText(amountStr,
			(x + ITEM_SLOT_SIZE) - (textSize.x / 1.25f),
			(y + ITEM_SLOT_SIZE) - (textSize.y / 1.25f),
			18.0f,
			WHITE
		);
	}
}

void item_container_draw_specific(ItemContainer* ic, int x, int y) {
	Vector2 size = item_container_get_size(ic);
	Vector2 titleSize = MeasureTextEx(GetFontDefault(), ic->name, TITLE_SIZE, 0.0f);
	titleSize.y += ITEM_SLOT_GAP;

	DrawRectangle(
		x,
		y,
		size.x,
		size.y,
		BG_COLOR
	);

	DrawText(
		ic->name,
		x + ITEM_SLOT_GAP,
		y + ITEM_SLOT_GAP,
		TITLE_SIZE,
		TITLE_COLOR
	);

	// Drawing slots, items and registering input
	for (int r = 0; r < ic->rows; r++) {
		for (int c = 0; c < ic->columns; c++) {
			int i = c + (r * ic->columns);

			Rectangle slotRect = {
				.x = x + (c * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP) + ITEM_SLOT_GAP),
				.y = y + (r * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP) + ITEM_SLOT_GAP) + titleSize.y,
				.width = ITEM_SLOT_SIZE,
				.height = ITEM_SLOT_SIZE
			};

			bool isHovered = CheckCollisionPointRec(GetMousePosition(), slotRect);
			ItemRegistry* ir = ir_get_item_registry(ic->items[i].item_id);
			if (isHovered && !ic->immutable) {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					// Swapping existing items
					if (ic->items[i].item_id > 0 && grabbedItem.item_id != ic->items[i].item_id) {
						lastSlotIdx = i;
						lastContainer = ic;

						uint8_t prev_slot_item_id = ic->items[i].item_id;
						uint8_t prev_slot_amount = ic->items[i].amount;

						uint8_t prev_grab_item_id = grabbedItem.item_id;
						uint8_t prev_grab_amount = grabbedItem.amount;

						grabbedItem.item_id = prev_slot_item_id;
						grabbedItem.amount = prev_slot_amount;

						ic->items[i].item_id = prev_grab_item_id;
						ic->items[i].amount = prev_grab_amount;
					}
					// Adding an amount of items to a existing slot if the item IDs match
					else if (ic->items[i].item_id > 0 && grabbedItem.item_id == ic->items[i].item_id) {
						lastSlotIdx = -1;
						lastContainer = NULL;

						ic->items[i].amount += grabbedItem.amount;

						grabbedItem.item_id = 0;
						grabbedItem.amount = 0;
					}
					// Putting the holding item on a empty slot
					else if (ic->items[i].item_id <= 0 && grabbedItem.item_id > 0) {
						lastSlotIdx = -1;
						lastContainer = NULL;

						ic->items[i].item_id = grabbedItem.item_id;
						ic->items[i].amount = grabbedItem.amount;

						grabbedItem.item_id = 0;
						grabbedItem.amount = 0;
					}
				}
				if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
					if (ic->items[i].item_id > 0) {
						// Adding to existing item with the grabbed item
						if (grabbedItem.item_id == ic->items[i].item_id) {
							if (grabbedItem.amount > 1) {
								lastSlotIdx = i;
								lastContainer = ic;

								grabbedItem.amount--;
								ic->items[i].amount++;
							}
							else {
								lastSlotIdx = -1;
								lastContainer = NULL;

								grabbedItem.item_id = 0;
								grabbedItem.amount = 0;

								ic->items[i].amount++;
							}
						}
						// Dividing a stack of items into two
						else if (grabbedItem.item_id <= 0) {
							uint8_t amount = ic->items[i].amount;
							lastSlotIdx = i;
							lastContainer = ic;
							if (amount > 1) {
								grabbedItem.item_id = ic->items[i].item_id;
								grabbedItem.amount = amount / 2;

								ic->items[i].amount = amount - grabbedItem.amount;
							}
							else {
								grabbedItem.item_id = ic->items[i].item_id;
								grabbedItem.amount = ic->items[i].amount;

								ic->items[i].item_id = 0;
								ic->items[i].amount = 0;
							}
						}
					}
					else {
						// Adding to an empty slot with the grabbed item
						if (grabbedItem.item_id > 0) {
							if (grabbedItem.amount > 1) {
								lastSlotIdx = i;
								lastContainer = ic;

								ic->items[i].item_id = grabbedItem.item_id;
								ic->items[i].amount++;

								grabbedItem.amount--;
							}
							else {
								lastSlotIdx = -1;
								lastContainer = NULL;

								ic->items[i].item_id = grabbedItem.item_id;
								ic->items[i].amount++;

								grabbedItem.item_id = 0;
								grabbedItem.amount = 0;
							}
						}
					}
				}
			}
			// If the container is immutable, then just copy the item to the holding item
			else if (isHovered && ic->immutable) {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					if (ic->items[i].item_id > 0 && grabbedItem.item_id <= 0) {
						lastContainer = ic;
						lastSlotIdx = i;

						grabbedItem.item_id = ic->items[i].item_id;
						grabbedItem.amount = ic->items[i].amount;
					}
					else if (grabbedItem.item_id > 0) {
						grabbedItem.item_id = 0;
						grabbedItem.amount = 0;
					}
				}
			}

			DrawRectangleRec(slotRect, SLOT_COLOR);

			draw_item(&ic->items[i], slotRect.x, slotRect.y);

			if (isHovered) DrawRectangleRec(slotRect, SLOT_HOVER_COLOR);
		}
	}

	// Drawing text
	for (int i = 0; i < (ic->rows * ic->columns); i++) {
		if (ic->items[i].item_id > 0) {
			int r = i / ic->columns;
			int c = i % ic->columns;

			Rectangle slotRect = {
				.x = x + (c * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP) + ITEM_SLOT_GAP),
				.y = y + (r * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP) + ITEM_SLOT_GAP) + titleSize.y,
				.width = ITEM_SLOT_SIZE,
				.height = ITEM_SLOT_SIZE
			};

			bool isHovered = CheckCollisionPointRec(GetMousePosition(), slotRect);

			ItemRegistry* ir = ir_get_item_registry(ic->items[i].item_id);

			if (isHovered && ir) {
				const int padding = 4;
				Vector2 textSize = MeasureTextEx(GetFontDefault(), ir->name, 24.0f, 2.0f);

				DrawRectangle(
					GetMouseX(),
					GetMouseY() - textSize.y - padding,
					textSize.x + (padding * 2),
					textSize.y + (padding * 2),
					(Color) { 0, 0, 0, 128 }
				);

				DrawText(
					ir->name,
					GetMouseX() + padding,
					GetMouseY() - textSize.y,
					24.0f,
					WHITE
				);
			}
		}
	}
}

void item_container_draw() {
	if (!openedContainer) return;

	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) { 0, 0, 0, 128 });

	Vector2 center = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

	Vector2 openContSize = item_container_get_size(openedContainer);
	Vector2 invSize = item_container_get_size(&inventory);

	item_container_draw_specific(openedContainer, center.x - (openContSize.x / 2), center.y - openContSize.y);
	item_container_draw_specific(&inventory, center.x - (invSize.x / 2), center.y);

	draw_item(&grabbedItem, GetMouseX(), GetMouseY());
}

void item_container_free(ItemContainer* ic)
{
	if (!ic) return;
	if (ic->items) free(ic->items);
}
