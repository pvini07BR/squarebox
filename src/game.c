#include "game.h"

#include "world_manager.h"
#include "game_settings.h"
#include "entity/entity.h"
#include "registries/block_colliders.h"
#include "registries/block_models.h"
#include "registries/block_registry.h"
#include "registries/item_registry.h"
#include "entity/item_entity.h"
#include "entity/player.h"
#include "lists/entity_list.h"
#include "chunk_manager.h"
#include "item_container.h"
#include "sign_editor.h"
#include "texture_atlas.h"
#include "types.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <limits.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

Player* player = NULL;
Camera2D camera;
Texture2D place_mode_icon;

Vector2 mouseWorldPos;
Vector2i mouseBlockPos;
Vector2i currentChunkPos;
Rectangle blockPlacerRect;

ChunkLayerEnum sel_layer = CHUNK_LAYER_FOREGROUND;
uint8_t blockStateIdx = 0;

bool loadedGhostMesh = false;
Mesh ghostBlockMesh = { 0 };

ItemContainer creativeMenu;
int8_t last_hotbarIdx = -1;
size_t last_blockState = SIZE_MAX;
uint8_t lastItemId = UCHAR_MAX;
int8_t hotbarIdx = 0;

bool demo_mode = true;
bool draw_ui = true;
bool debug_info = false;
char debug_text[1024];

void game_init() {
    place_mode_icon = LoadTexture(ASSETS_PATH "place_modes.png");

    texture_atlas_load(ASSETS_PATH "atlas.png", 4, 11);

    block_models_init();
    block_colliders_init();

    item_registry_init();
    block_registry_init();

	char* str = calloc(14, sizeof(char));
	strcpy(str, "Creative Menu");
    item_container_create(&creativeMenu, str, 5, 10, true);
    for (int i = 1; i < ITEM_COUNT; i++) {
        item_container_set_item(&creativeMenu, (i - 1) / creativeMenu.columns, (i - 1) % creativeMenu.columns, (ItemSlot){ i, 1 });
    }

    init_inventory();

    camera = (Camera2D){
		.target = { 0, 0 },
        .offset =  { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f },
        .rotation = 0.0f,
        .zoom = 1.0f
    };

    mouseWorldPos = Vector2Zero();
    mouseBlockPos = (Vector2i){ 0, 0 };
    currentChunkPos = (Vector2i){ 0, 0 };

    blockPlacerRect = (Rectangle){
        0, 0, TILE_SIZE, TILE_SIZE
    };

    currentChunkPos = (Vector2i) {
        (int)floorf(camera.target.x / (CHUNK_WIDTH * TILE_SIZE)),
        (int)floorf(camera.target.y / (CHUNK_WIDTH * TILE_SIZE))
    };

    chunk_manager_init((Vector2i) { currentChunkPos.x, currentChunkPos.y }, 5, 3);
}

void game_tick() {
    chunk_manager_tick();
}

void game_update(float deltaTime) {
    Vector2i cameraChunkPos = {
        (int)floorf(camera.target.x / (CHUNK_WIDTH * TILE_SIZE)),
        (int)floorf(camera.target.y / (CHUNK_WIDTH * TILE_SIZE))
    };

    if (cameraChunkPos.x != currentChunkPos.x || cameraChunkPos.y != currentChunkPos.y) {
        chunk_manager_relocate(cameraChunkPos);
        currentChunkPos = cameraChunkPos;
    }

    if (demo_mode) {
        camera.target.x += 300.0f * deltaTime;
        return;
    }

    mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    mouseBlockPos = (Vector2i){
        (int)floorf((float)mouseWorldPos.x / (float)TILE_SIZE),
        (int)floorf((float)mouseWorldPos.y / (float)TILE_SIZE)
    };
    blockPlacerRect.x = mouseBlockPos.x * TILE_SIZE;
    blockPlacerRect.y = mouseBlockPos.y * TILE_SIZE;

    if (!game_is_ui_open()) {
        if (IsKeyPressed(KEY_TAB)) sel_layer = sel_layer == CHUNK_LAYER_FOREGROUND ? CHUNK_LAYER_BACKGROUND : CHUNK_LAYER_FOREGROUND;

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            chunk_manager_set_block_safe(mouseBlockPos, (BlockInstance) { 0, 0 }, sel_layer);
        else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            if (!chunk_manager_interact(mouseBlockPos, sel_layer)) {
                ItemRegistry* itr = ir_get_item_registry(inventory_get_item(0, hotbarIdx).item_id);
                if (itr->blockId > 0 && !(itr->placingFlags & (sel_layer == CHUNK_LAYER_BACKGROUND ? ITEM_PLACE_FLAG_NOT_WALL : ITEM_PLACE_FLAG_NOT_BLOCK))) {
                    BlockRegistry* br = br_get_block_registry(itr->blockId);

					bool blockIsSolid = (br->flags & BLOCK_FLAG_SOLID) != 0;
                    bool playerAllowsPlacement = true;
                    if (blockIsSolid && player) {
						playerAllowsPlacement = !CheckCollisionRecs(blockPlacerRect, player->entity.rect);
                    }

                    bool canPlace =
                        (sel_layer == CHUNK_LAYER_BACKGROUND) ||
                        (sel_layer == CHUNK_LAYER_FOREGROUND && (!blockIsSolid || playerAllowsPlacement));

                    if (canPlace) {
                        uint8_t state = blockStateIdx;
                        if (br->state_selector) {
                            state = br->state_selector(blockStateIdx);
                        }
                        BlockInstance inst = {
                            .id = itr->blockId,
                            .state = state
                        };
                        chunk_manager_set_block_safe(mouseBlockPos, inst, sel_layer);
                    }
                }
            }
        }

        if (IsKeyPressed(KEY_F)) player->entity.gravity_affected = !player->entity.gravity_affected;

        if (IsKeyPressed(KEY_F1)) draw_ui = !draw_ui;
		if (IsKeyPressed(KEY_F2)) TakeScreenshot("screenshot.png");
        if (IsKeyPressed(KEY_F3)) debug_info = !debug_info;

        if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

        // When pressing Q, the holding item will be dropped and launched at the direction of the mouse.
        // the force of throwing is determined by how far the mouse is from the player (in screen coordinates)
        ItemSlot item = inventory_get_item(0, hotbarIdx);
        if (IsKeyPressed(KEY_Q) && item.item_id > 0) {
            Vector2 playerToScreen = GetWorldToScreen2D(entity_get_center(&player->entity), camera);
            Vector2 mouse_dir = Vector2Subtract(GetMousePosition(), playerToScreen);
            mouse_dir = Vector2Scale(mouse_dir, 2.0f);

            Vector2 item_pos = Vector2SubtractValue(entity_get_center(&player->entity), TILE_SIZE * 0.25f);
            ItemEntity* ie = item_entity_create(item_pos, mouse_dir, (ItemSlot) { item.item_id, 1 });
            if (ie) {
                if (entity_list_add(&ie->entity)) {
                    if (item.amount > 1) {
                        inventory_set_item(0, hotbarIdx, (ItemSlot) { item.item_id, item.amount - 1 });
                    }
                    else {
                        inventory_set_item(0, hotbarIdx, (ItemSlot) { ITEM_NONE, 0 });
                    }
                }
            }
        }

        if (debug_info && IsKeyPressed(KEY_C)) {
            Vector2i chunkPos = {
                (int)floorf((float)mouseBlockPos.x / (float)CHUNK_WIDTH),
                (int)floorf((float)mouseBlockPos.y / (float)CHUNK_WIDTH)
            };
            Chunk* chunk = chunk_manager_get_chunk(chunkPos);
            if (chunk) {
                printf(
                    "========\n"
                    "%d|%d|%d\n"
                    "%d| |%d\n"
                    "%d|%d|%d\n"
                    "========\n",

                    chunk->neighbors.upLeft != NULL,
                    chunk->neighbors.up != NULL,
                    chunk->neighbors.upRight != NULL,
                    chunk->neighbors.left != NULL,
                    chunk->neighbors.right != NULL,
                    chunk->neighbors.downLeft != NULL,
                    chunk->neighbors.down != NULL,
                    chunk->neighbors.downRight != NULL
                );
            }
        }

        int scroll = GetMouseWheelMoveV().y;
        if (IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_LEFT_SHIFT)) {
            if (scroll > 0) camera.zoom *= 1.1f;
            if (scroll < 0) camera.zoom /= 1.1f;

            camera.zoom = Clamp(camera.zoom, 0.1f, 10.0f);
        }
        else {
            if (scroll > 0) hotbarIdx--;
            if (scroll < 0) hotbarIdx++;

            if (hotbarIdx < 0) hotbarIdx = 9;
            if (hotbarIdx > 9) hotbarIdx = 0;
        }

        if (IsKeyPressed(KEY_ONE)) hotbarIdx = 0;
        if (IsKeyPressed(KEY_TWO)) hotbarIdx = 1;
        if (IsKeyPressed(KEY_THREE)) hotbarIdx = 2;
        if (IsKeyPressed(KEY_FOUR)) hotbarIdx = 3;
        if (IsKeyPressed(KEY_FIVE)) hotbarIdx = 4;
        if (IsKeyPressed(KEY_SIX)) hotbarIdx = 5;
        if (IsKeyPressed(KEY_SEVEN)) hotbarIdx = 6;
        if (IsKeyPressed(KEY_EIGHT)) hotbarIdx = 7;
        if (IsKeyPressed(KEY_NINE)) hotbarIdx = 8;
        if (IsKeyPressed(KEY_ZERO)) hotbarIdx = 9;
    }

    if (player) player->disable_input = game_is_ui_open();
    entity_list_update(GetFrameTime());
        
    if (player) {
        Vector2 newTarget = Vector2Add(player_get_position(player), Vector2Scale(player_get_size(player), 0.5f));
        camera.target = Vector2Lerp(camera.target, newTarget, 25.0f * GetFrameTime());
    }

    if (IsKeyPressed(KEY_E) && !game_is_ui_open())
        item_container_open(&creativeMenu);
    else if ((IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ESCAPE) && item_container_is_open()))
        item_container_close();
    
    if (IsKeyPressed(KEY_ESCAPE) && sign_editor_is_open()) {
        sign_editor_close();
    }

    ItemSlot heldItem = inventory_get_item(0, hotbarIdx);
    ItemRegistry* heldItemReg = ir_get_item_registry(heldItem.item_id);
    BlockRegistry* heldBlockReg = br_get_block_registry(heldItemReg->blockId);
    if (heldItemReg->blockId > 0 && heldBlockReg->flags & BLOCK_FLAG_STATE_SELECTABLE) {
        bool reload = false;

        if (lastItemId != heldItem.item_id) reload = true;

        if (IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_X)) {
            if (IsKeyPressed(KEY_Z)) blockStateIdx--;
            if (IsKeyPressed(KEY_X)) blockStateIdx++;

            if (heldBlockReg->state_selector) {
                if (blockStateIdx < 0) blockStateIdx = heldBlockReg->state_count - 1;
                if (blockStateIdx >= heldBlockReg->state_count) blockStateIdx = 0;
            } else {
                if (blockStateIdx < 0) blockStateIdx = heldBlockReg->variant_count - 1;
                if (blockStateIdx >= heldBlockReg->variant_count) blockStateIdx = 0;
            }

            reload = true;
        }

        if (reload) {
            if (loadedGhostMesh == true) {
                UnloadMesh(ghostBlockMesh);
                ghostBlockMesh = (Mesh){ 0 };
                loadedGhostMesh = false;
            }
        }
        
        if (loadedGhostMesh == false) {
            uint8_t variant_idx = blockStateIdx;
            if (heldBlockReg->state_selector && heldBlockReg->variant_selector) {
                uint8_t state = heldBlockReg->state_selector(blockStateIdx);
                variant_idx = heldBlockReg->variant_selector(state);
            }
            BlockVariant bvar = br_get_block_variant(heldItemReg->blockId, variant_idx);
            block_models_build_mesh(&ghostBlockMesh, bvar.model_idx, bvar.atlas_idx, false, false, bvar.flipH, bvar.flipV, bvar.rotation);
            loadedGhostMesh = true;
        }
    } else {
        blockStateIdx = 0;
        if (loadedGhostMesh == true) {
            UnloadMesh(ghostBlockMesh);
            ghostBlockMesh = (Mesh){ 0 };
            loadedGhostMesh = false;
        }
    }
}

void game_draw(bool draw_overlay) {
    camera.offset = (Vector2){
        .x = GetScreenWidth() / 2.0f, 
        .y = GetScreenHeight() / 2.0f
    };

    //Draw sky gradient
    typedef struct {
        Color color;
        float height;
    } SkyBand;

    int band_count = 5;
    SkyBand bands[] = {
        { BLACK, 6400 },
        { (Color) { 122, 122, 170, 255 }, 2560 },
        { (Color) { 122, 122, 170, 255 }, 50 },
        { BROWN, 1280 },
        { BLACK, 1280 }
    };

    rlPushMatrix();

    rlTranslatef(
        0.0f,
        camera.offset.y,
        0.0f
    );

    rlScalef(
        1.0f,
        camera.zoom,
        0.0f
    );

    rlTranslatef(
        0.0f,
        -camera.target.y,
        0.0f
    );

    float origin = -(bands[0].height);
    for (int i = 0; i < band_count; i++) {
        int next = i + 1;
        if (next >= band_count) next = band_count - 1;
        DrawRectangleGradientV(0, origin, GetScreenWidth(), bands[i].height, bands[i].color, bands[next].color);
        origin += bands[i].height;
    }

    rlPopMatrix();

    BeginMode2D(camera);

    chunk_manager_draw(debug_info);

    entity_list_draw(debug_info);

    chunk_manager_draw_liquids();

    if (demo_mode) {
        EndMode2D();
        return;
    }
    
    if (!game_is_ui_open() && draw_ui) {
        // Draw block model if it is rotatable
        if (loadedGhostMesh == true) {
            DrawMesh(
                ghostBlockMesh,
                texture_atlas_get_material(),
                MatrixTranslate(mouseBlockPos.x * TILE_SIZE, mouseBlockPos.y * TILE_SIZE, 0.0f)
            );
        }

        float cos = 47.5f * cosf(GetTime() * 4.0f) + 79.5f;

        DrawRectangleRec(blockPlacerRect, (Color) { 255, 255, 255, cos });
    }

    EndMode2D();

    if (!game_is_ui_open() && draw_ui) {
        if (inventory_get_item(0, hotbarIdx).item_id > 0) {
            draw_item(inventory_get_item(0, hotbarIdx), GetMouseX(), GetMouseY(), 0, 0.8f, false);
        }

        DrawTexturePro(
            place_mode_icon,
            (Rectangle) {
                .x = sel_layer == CHUNK_LAYER_BACKGROUND ? 0 : 8,
                .y = 0,
                .width = 8,
                .height = 8
            },
            (Rectangle) {
                .x = GetMouseX() + (TILE_SIZE * 0.8f) - 8,
                .y = GetMouseY() + (TILE_SIZE * 0.8f) - 8,
                .width = 16,
                .height = 16
            },
            Vector2Zero(),
            0.0f,
            WHITE
        );
    }

    if (draw_ui) {
        Vector2 center = { GetScreenWidth() / 2.0f, GetScreenHeight()};

        int width = (get_inventory()->columns * (ITEM_SLOT_SIZE + 4)) + 4;
        int height = (get_inventory()->rows * (ITEM_SLOT_SIZE + 4)) + 4;

        const Color hotbarBg = { 0, 0, 0, 128 };
        const Color selHotbarSlot = { 128, 128, 128, 128 };

        DrawRectangle(
            center.x - (width / 2.0f),
            center.y - height,
            width,
            height,
            hotbarBg
        );

        for (int r = 0; r < get_inventory()->rows; r++) {
            for (int c = 0; c < get_inventory()->columns; c++) {
                int i = c + (r * get_inventory()->columns);

                Rectangle slotRect = {
                    .x = ((center.x - (width / 2.0f)) + 4) + (c * (ITEM_SLOT_SIZE + 4)),
                    .y = ((center.y - height) + 4) + (r * (ITEM_SLOT_SIZE + 4)),
                    .width = ITEM_SLOT_SIZE,
                    .height = ITEM_SLOT_SIZE
                };

                DrawRectangleRec(slotRect, i == hotbarIdx ? selHotbarSlot : hotbarBg);

                draw_item(get_inventory()->items[i], slotRect.x, slotRect.y, ((ITEM_SLOT_SIZE - TILE_SIZE) / 2.0f), 1.0f, true);
            }
        }
    }

    if (debug_info && draw_ui) {
        sprintf(debug_text,
            "FPS: %d\n"
            "Loaded chunk area: %ux%u\n"
            "Camera chunk position: (%d, %d)\n"
            "Camera Zoom: %f\n"
            "Player position: (%f, %f)\n"
            "Holding item: %s\n",

            GetFPS(),
            chunk_manager_get_view_width(),
            chunk_manager_get_view_height(),
			currentChunkPos.x, currentChunkPos.y,
            camera.zoom,
            player->entity.rect.x, player->entity.rect.y,
            ir_get_item_registry(inventory_get_item(0, hotbarIdx).item_id)->name
        );

        DrawText(debug_text, 0, 0, 24, WHITE);
    }

    sign_editor_draw();
    item_container_draw();
}

void game_free() {
    entity_list_clear();
    free_inventory();
    item_container_free(&creativeMenu);
    item_registry_free();
    chunk_manager_free();
    block_registry_free();
    block_models_free();
    texture_atlas_free();
}

bool game_is_ui_open() {
	return item_container_is_open() || sign_editor_is_open();
}

void game_set_demo_mode(bool demo) {
    if (demo_mode && !demo) {
        // Switching from demo to normal mode
        chunk_manager_clear(false);

        Vector2 playerPosition = get_world_info()->player_position;
        Vector2i playerChunkPos = {
            (int)floorf(playerPosition.x / (CHUNK_WIDTH * TILE_SIZE)),
            (int)floorf(playerPosition.y / (CHUNK_WIDTH * TILE_SIZE))
		};

        chunk_manager_set_view(
            get_game_settings()->chunk_view_width,
            get_game_settings()->chunk_view_height
		);
        chunk_manager_relocate(playerChunkPos);

        if (player == NULL) {
            player = player_create(playerPosition);
            if (player) {
				player->entity.gravity_affected = get_world_info()->player_flying;
                entity_list_add(&player->entity);
                camera.target = Vector2Add(player_get_position(player), Vector2Scale(player_get_size(player), 0.5f));
            }
		}
    }
    else if (!demo_mode && demo) {
        // Switching from normal to demo mode
        player = NULL;
        entity_list_clear();
        chunk_manager_set_view(5, 3);
        chunk_manager_clear(true);
        camera.target = (Vector2){ 0, 0 };
        camera.zoom = 1.0f;
        debug_info = false;
		inventory_clear();
	}

    demo_mode = demo;
}

bool game_is_demo_mode() {
    return demo_mode;
}

void game_set_draw_ui(bool draw) {
	draw_ui = draw;
}

Player* game_get_player() {
    return player;
}

Vector2 game_get_camera_pos() {
    return camera.target;
}