#include "item_container.h"

#include <stdlib.h>
#include <stdio.h>

#include <rlgl.h>
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
static ItemSlot holdingItem = { 0, 0 };
static ItemContainer* lastContainer = NULL;

static ItemContainer inventory;

void swapItems(ItemSlot* a, ItemSlot* b) {
	ItemSlot temp = *a;
	*a = *b;
	*b = temp;
}

void clearItem(ItemSlot* item) {
	item->item_id = item->amount = 0;
}

void setLastAction(ItemContainer* container) {
	lastContainer = container;
}

void clearLastAction() {
	setLastAction(NULL);
}

bool tryStackItems(ItemSlot* target, ItemSlot* source) {
	unsigned int total = target->amount + source->amount;
	if (total <= MAX_STACK) {
		target->amount = total;
		clearItem(source);
		return true;
	}
	else {
		target->amount = MAX_STACK;
		source->amount = total - MAX_STACK;
		return false;
	}
}

ItemSlot* findStackableSlot(ItemContainer* container, uint8_t item_id) {
	if (!container) return NULL;
	for (int j = 0; j < (container->rows * container->columns); j++) {
		if (container->items[j].item_id == item_id && container->items[j].amount < MAX_STACK) {
			return &container->items[j];
		}
	}
	return NULL;
}

ItemSlot* findEmptySlot(ItemContainer* container) {
	for (int j = 0; j < (container->rows * container->columns); j++) {
		if (container->items[j].item_id <= 0) {
			return &container->items[j];
		}
	}
	return NULL;
}

void distribute_item(ItemSlot* item, ItemContainer* container) {
	while (item->amount > 0) {
		ItemSlot* stackable = findStackableSlot(container, item->item_id);
		if (stackable) {
			bool fullyStacked = tryStackItems(stackable, item);
			if (fullyStacked) break; // All items were moved
		}
		else {
			// No more stackable slots, try to place in empty slot
			ItemSlot* empty = findEmptySlot(container);
			if (empty) {
				*empty = *item;
				clearItem(item);
			}
			break;
		}
	}
}

void init_inventory() {
	item_container_create(&inventory, "Inventory", 1, 10, false);
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
	if (!ic) return (ItemSlot) { 0, 0 };
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
	// add it to the container before closing it.
	if (lastContainer) {
		if (!lastContainer->immutable) {
			distribute_item(&holdingItem, lastContainer);
		}
		else {
			clearItem(&holdingItem);
		}
	}

	lastContainer = NULL;
	openedContainer = NULL;
}

bool item_container_is_open() { return openedContainer != NULL; }

void handleNormalClick(ItemSlot* curItem, int i, ItemContainer* ic) {
	if (holdingItem.item_id != curItem->item_id) {
		// Different items - swap or copy
		if (!ic->immutable) {
			setLastAction(ic);
			swapItems(&holdingItem, curItem);
		}
		else {
			if (holdingItem.item_id <= 0) {
				setLastAction(ic);
				holdingItem = *curItem;
			}
			else {
				clearLastAction();
				clearItem(&holdingItem);
			}
		}
	}
	else {
		// Same items - stack
		if (!ic->immutable) {
			if (tryStackItems(curItem, &holdingItem)) {
				clearLastAction();
			}
			else {
				setLastAction(ic);
			}
		}
		else {
			// Immutable - just increment
			if (holdingItem.amount < MAX_STACK) holdingItem.amount++;
		}
	}
}

void draw_item(ItemSlot* is, int x, int y) {
	if (is->item_id <= 0) return;
	ItemRegistry* ir = ir_get_item_registry(is->item_id);

	rlDrawRenderBatchActive();

	DrawMesh(
		ir->mesh,
		texture_atlas_get_material(),
		MatrixTranslate(x + ((ITEM_SLOT_SIZE - TILE_SIZE) / 2.0f), y + ((ITEM_SLOT_SIZE - TILE_SIZE) / 2.0f), 0.0f)
	);

	/*
	DrawTexturePro(
		texture_atlas_get(),
		texture_atlas_get_rect(ir->atlas_idx, false, false),
		(Rectangle) {
			.x = x + ((ITEM_SLOT_SIZE - TILE_SIZE) / 2.0f),
			.y = y + ((ITEM_SLOT_SIZE - TILE_SIZE) / 2.0f),
			.width = TILE_SIZE,
			.height = TILE_SIZE
		},
		Vector2Zero(),
		0.0f,
		WHITE
	);
	*/

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
	if (!ic) return;

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
	for (int i = 0; i < (ic->rows * ic->columns); i++) {
		int r = i / ic->columns;
		int c = i % ic->columns;

		Rectangle slotRect = {
			.x = x + (c * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP) + ITEM_SLOT_GAP),
			.y = y + (r * (ITEM_SLOT_SIZE + ITEM_SLOT_GAP) + ITEM_SLOT_GAP) + titleSize.y,
			.width = ITEM_SLOT_SIZE,
			.height = ITEM_SLOT_SIZE
		};

		DrawRectangleRec(slotRect, SLOT_COLOR);
		draw_item(&ic->items[i], slotRect.x, slotRect.y);

		// Why continue if not being hovered?
		if (!CheckCollisionPointRec(GetMousePosition(), slotRect)) continue;

		DrawRectangleRec(slotRect, SLOT_HOVER_COLOR);

		ItemSlot* curItem = &ic->items[i];
		ItemRegistry* curItemReg = ir_get_item_registry(curItem->item_id);

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
			if (!IsKeyDown(KEY_LEFT_SHIFT)) {
				handleNormalClick(curItem, i, ic);
			}
			else {
				if (!ic->immutable) {
					ItemContainer* otherContainer = ic == openedContainer ? &inventory : openedContainer;

					if (!otherContainer->immutable) {
						// Try to stack with all existing items that have space
						distribute_item(curItem, otherContainer);
					}
					else clearItem(curItem);
				}
				else {
					// Immutable container with shift
					if (holdingItem.item_id <= 0) {
						setLastAction(ic);
						holdingItem.item_id = curItem->item_id;
						holdingItem.amount = MAX_STACK;
					}
					else {
						if (holdingItem.item_id <= 0) {
							setLastAction(ic);
							holdingItem = *curItem;
						}
						else {
							clearLastAction();
							clearItem(&holdingItem);
						}
					}
				}
			}
		}
		else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			if (!IsKeyDown(KEY_LEFT_SHIFT)) {
				if (!ic->immutable) {
					// Add holding item to slot if holding something
					if (holdingItem.item_id > 0) {
						if (holdingItem.item_id == curItem->item_id) {
							if (curItem->amount < MAX_STACK) {
								curItem->amount++;
								if (holdingItem.amount > 1) holdingItem.amount--;
								else {
									clearLastAction();
									clearItem(&holdingItem);
								}
							}
						}
						else {
							if (curItem->item_id <= 0) {
								curItem->item_id = holdingItem.item_id;
								curItem->amount = 1;
								if (holdingItem.amount > 1) holdingItem.amount--;
								else {
									clearLastAction();
									clearItem(&holdingItem);
								}
							}
							else swapItems(&holdingItem, curItem);
						}
					}
					// Split the stack in the current slot in half when not
					// holding anything
					else {
						setLastAction(ic);
						holdingItem.item_id = curItem->item_id;
						holdingItem.amount = curItem->amount / 2;
						curItem->amount = curItem->amount - holdingItem.amount;
					}
				}
			}
			else {
				// Shift + right click = same as normal left click
				handleNormalClick(curItem, i, ic);
			}
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

	draw_item(&holdingItem, GetMouseX(), GetMouseY());
}

void item_container_free(ItemContainer* ic)
{
	if (!ic) return;
	if (ic->items) {
		free(ic->items);
		ic->items = NULL;
	}
}
