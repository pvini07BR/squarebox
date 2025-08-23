#include "item_container.h"

#include <stdlib.h>
#include <stdio.h>

#include <raylib.h>
#include <raymath.h>

#include "block_registry.h"
#include "item_registry.h"

#define ITEM_SLOT_SIZE 32
#define ITEM_SLOT_GAP 8

static ItemContainer* openedContainer = NULL;
static ItemSlot grabbedItem = { 0, 0 };
static int lastSlotIdx = -1;

void item_container_create(ItemContainer* ic, uint8_t rows, uint8_t columns)
{
	if (!ic) return;
	ic->rows = rows;
	ic->columns = columns;
	ic->items = calloc(ic->rows * ic->columns, sizeof(ItemSlot));
	if (ic->items) for (int i = 0; i < (ic->rows * ic->columns); i++)
		ic->items[i] = (ItemSlot){ 0, 0 };
}

ItemSlot* item_container_get_item(ItemContainer* ic, uint8_t row, uint8_t column)
{
	if (!ic) return;
	if (row < 0 || row >= ic->rows || column < 0 || column >= ic->columns) return NULL;
	int i = column + (row * ic->columns);
	return &ic->items[i];
}

void item_container_set_item(ItemContainer* ic, uint8_t row, uint8_t column, ItemSlot item)
{
	if (!ic) return;
	if (row < 0 || row >= ic->rows || column < 0 || column >= ic->columns) return;
	if (item.amount <= 0) item.amount = 1;
	int i = column + (row * ic->columns);
	ic->items[i] = item;
}

void item_container_open(ItemContainer* ic)
{
	if (!ic) return;
	openedContainer = ic;
}

void item_container_close() {
	// If closing the item container while having a grabbed item,
	// add it back to the slot it came from
	if (lastSlotIdx >= 0) {
		openedContainer->items[lastSlotIdx].item_id = grabbedItem.item_id;
		openedContainer->items[lastSlotIdx].amount += grabbedItem.amount;

		grabbedItem.item_id = 0;
		grabbedItem.amount = 0;

		lastSlotIdx = -1;
	}

	openedContainer = NULL; 
}
bool item_container_is_open() { return openedContainer != NULL; }

void draw_item(ItemSlot* is, int x, int y, Texture2D* textureAtlas) {
	ItemRegistry* ir = ir_get_item_registry(is->item_id);
	if (ir == NULL) return;

	if (ir->type == ITEM_TYPE_BLOCK) {
		DrawTexturePro(
			*textureAtlas,
			br_get_block_texture_rect(ir->blockId, false, false),
			(Rectangle) {
				.x = x + (ITEM_SLOT_SIZE * 0.1f),
				.y = y + (ITEM_SLOT_SIZE * 0.1f),
				.width = ITEM_SLOT_SIZE * 0.8f,
				.height = ITEM_SLOT_SIZE * 0.8f
			},
			Vector2Zero(),
			0.0f,
			WHITE
		);
	}

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

void item_container_draw(Texture2D* textureAtlas) {
	if (!openedContainer) return;

	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) { 0, 0, 0, 128 });

	Vector2 center = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

	int width = (openedContainer->columns * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP)) + ITEM_SLOT_GAP;
	int height = (openedContainer->rows * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP)) + ITEM_SLOT_GAP;

	DrawRectangle(
		center.x - (width / 2),
		center.y - (height / 2),
		width,
		height,
		LIGHTGRAY
	);

	for (int r = 0; r < openedContainer->rows; r++) {
		for (int c = 0; c < openedContainer->columns; c++) {
			int i = c + (r * openedContainer->columns);

			Rectangle slotRect = {
				.x = ((center.x - (width / 2)) + ITEM_SLOT_GAP) + (c * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP)),
				.y = ((center.y - (height / 2)) + ITEM_SLOT_GAP) + (r * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP)),
				.width = ITEM_SLOT_SIZE,
				.height = ITEM_SLOT_SIZE
			};

			bool isHovered = CheckCollisionPointRec(GetMousePosition(), slotRect);
			ItemRegistry* ir = ir_get_item_registry(openedContainer->items[i].item_id);
			if (isHovered) {
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
					// Swapping existing items
					if (ir && grabbedItem.item_id != openedContainer->items[i].item_id) {
						lastSlotIdx = i;

						uint8_t prev_slot_item_id = openedContainer->items[i].item_id;
						uint8_t prev_slot_amount = openedContainer->items[i].amount;

						uint8_t prev_grab_item_id = grabbedItem.item_id;
						uint8_t prev_grab_amount = grabbedItem.amount;

						grabbedItem.item_id = prev_slot_item_id;
						grabbedItem.amount = prev_slot_amount;

						openedContainer->items[i].item_id = prev_grab_item_id;
						openedContainer->items[i].amount = prev_grab_amount;
					}
					// Adding an amount of items to a existing slot if the item IDs match
					else if (ir && grabbedItem.item_id == openedContainer->items[i].item_id) {
						lastSlotIdx = -1;

						openedContainer->items[i].amount += grabbedItem.amount;

						grabbedItem.item_id = 0;
						grabbedItem.amount = 0;
					}
					// Putting the holding item on a empty slot
					else if (!ir && grabbedItem.item_id > 0) {
						lastSlotIdx = -1;

						openedContainer->items[i].item_id = grabbedItem.item_id;
						openedContainer->items[i].amount = grabbedItem.amount;

						grabbedItem.item_id = 0;
						grabbedItem.amount = 0;
					}
				}
				if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
					if (ir) {
						// Adding to existing item with the grabbed item
						if (grabbedItem.item_id == openedContainer->items[i].item_id) {
							if (grabbedItem.amount > 1) {
								lastSlotIdx = i;
								grabbedItem.amount--;
								openedContainer->items[i].amount++;
							}
							else {
								lastSlotIdx = -1;

								grabbedItem.item_id = 0;
								grabbedItem.amount = 0;

								openedContainer->items[i].amount++;
							}
						}
						// Dividing a stack of items into two
						else if (grabbedItem.item_id <= 0) {
							uint8_t amount = openedContainer->items[i].amount;
							lastSlotIdx = i;
							if (amount > 1) {
								grabbedItem.item_id = openedContainer->items[i].item_id;
								grabbedItem.amount = amount / 2;

								openedContainer->items[i].amount = amount - grabbedItem.amount;
							}
							else {
								grabbedItem.item_id = openedContainer->items[i].item_id;
								grabbedItem.amount = openedContainer->items[i].amount;

								openedContainer->items[i].item_id = 0;
								openedContainer->items[i].amount = 0;
							}
						}
					}
					else {
						// Adding to an empty slot with the grabbed item
						if (grabbedItem.item_id > 0) {
							if (grabbedItem.amount > 1) {
								lastSlotIdx = i;

								openedContainer->items[i].item_id = grabbedItem.item_id;
								openedContainer->items[i].amount++;

								grabbedItem.amount--;
							}
							else {
								lastSlotIdx = -1;

								openedContainer->items[i].item_id = grabbedItem.item_id;
								openedContainer->items[i].amount++;

								grabbedItem.item_id = 0;
								grabbedItem.amount = 0;
							}
						}
					}
				}
			}

			DrawRectangleRec(slotRect, GRAY);

			draw_item(&openedContainer->items[i], slotRect.x, slotRect.y, textureAtlas);

			if (isHovered) DrawRectangleRec(slotRect, (Color){ 255, 255, 255, 128 });
		}
	}

	for (int i = 0; i < (openedContainer->rows * openedContainer->columns); i++) {
		int r = i / openedContainer->columns;
		int c = i % openedContainer->columns;

		Rectangle slotRect = {
			.x = ((center.x - (width / 2)) + ITEM_SLOT_GAP) + (c * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP)),
			.y = ((center.y - (height / 2)) + ITEM_SLOT_GAP) + (r * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP)),
			.width = ITEM_SLOT_SIZE,
			.height = ITEM_SLOT_SIZE
		};

		bool isHovered = CheckCollisionPointRec(GetMousePosition(), slotRect);
		ItemRegistry* ir = ir_get_item_registry(openedContainer->items[i].item_id);

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

	draw_item(&grabbedItem, GetMouseX(), GetMouseY(), textureAtlas);
}

void item_container_free(ItemContainer* ic)
{
	if (!ic) return;
	if (ic->items) free(ic->items);
}
