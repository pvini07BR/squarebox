#include <stdio.h>
#include <limits.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define RAYGUI_IMPLEMENTATION
#include "thirdparty/raygui.h"

#include "defines.h"
#include "texture_atlas.h"
#include "chunk_manager.h"
#include "block_registry.h"
#include "item_registry.h"
#include "item_container.h"

extern int seed;
extern bool wallAmbientOcclusion;
extern bool smoothLighting;
extern unsigned int wallBrightness;
extern unsigned int wallAOvalue;

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "mijocraft");

    bool wall_mode = false;
    ItemContainer creativeMenu;
    uint8_t hotbarIdx = 0;

    bool mouseIsInUI = false;

    bool seedEdit = false;
    bool wallBrightEdit = false;
    bool wallAOEdit = false;

    Rectangle interPanel = {
        .x = 0,
        .y = 0,
        .width = 170,
        .height = 32 * 3
    };
    
    Texture2D place_mode_icon = LoadTexture(ASSETS_PATH "place_modes.png");

    texture_atlas_load(ASSETS_PATH "atlas.png");

    block_registry_init();
    item_registry_init();
    chunk_manager_init();

    item_container_create(&creativeMenu, 1, ITEM_COUNT-1);
    for (int i = 1; i < ITEM_COUNT; i++) {
        item_container_set_item(&creativeMenu, (i - 1) / creativeMenu.columns, (i - 1) % creativeMenu.columns, (ItemSlot){ i, 1 });
    }

    init_inventory();

    inventory_set_item(0, 0, (ItemSlot) { 1, 69 });
    inventory_set_item(0, 1, (ItemSlot) { 3, 2 });

    Camera2D camera = {
        .target =  { (CHUNK_WIDTH*TILE_SIZE)/2.0f, (CHUNK_WIDTH*TILE_SIZE)/2.0f },
        .offset =  { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f },
        .rotation = 0.0f,
        .zoom = 1.0f
    };

    Vector2 mouseWorldPos = Vector2Zero();
    Vector2i mouseBlockPos = { 0, 0 };
    Vector2i currentChunkPos = { 0, 0 };

    char buffer[1024];

    while (!WindowShouldClose()) {
        camera.offset = (Vector2){
            .x = GetScreenWidth() / 2.0f, 
            .y = GetScreenHeight() / 2.0f
        };


        mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        mouseBlockPos = (Vector2i){
            (int)floorf((float)mouseWorldPos.x / (float)TILE_SIZE),
            (int)floorf((float)mouseWorldPos.y / (float)TILE_SIZE)
        };

        if (!item_container_is_open()) {
            if (IsKeyPressed(KEY_TAB)) wall_mode = !wall_mode;

            if (!mouseIsInUI) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    chunk_manager_set_block(mouseBlockPos, (BlockInstance) { 0, 0 }, wall_mode);
                else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                    ItemRegistry* itr = ir_get_item_registry(inventory_get_item(0, hotbarIdx).item_id);
                    if (itr->blockId > 0) {
                        chunk_manager_set_block(mouseBlockPos, (BlockInstance) { itr->blockId, 0 }, wall_mode);
                    }
                }
            }

            Vector2 input = { 0 };

            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x = -1.0f;
            else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x = 1.0f;

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y = -1.0f;
            else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y = 1.0f;

            if (IsKeyPressed(KEY_EQUAL))camera.zoom *= 1.1f;
            else if (IsKeyPressed(KEY_MINUS)) camera.zoom /= 1.1f;

            int scroll = GetMouseWheelMoveV().y;
            if (hotbarIdx > 0 && scroll > 0) hotbarIdx--;
            if (hotbarIdx < 8 && scroll < 0) hotbarIdx++;

            input = Vector2Normalize(input);

            float speed = 500.0;
            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                speed *= 4.0;
            }

            camera.target.x += input.x * speed * GetFrameTime();
            camera.target.y += input.y * speed * GetFrameTime();
        }

        if (IsKeyPressed(KEY_E)) {
            if (!item_container_is_open()) {
                item_container_open(&creativeMenu);
            }
            else {
                item_container_close();
            }
        }

        Vector2i cameraChunkPos = {
            (int)floorf(camera.target.x / (CHUNK_WIDTH * TILE_SIZE)),
            (int)floorf(camera.target.y / (CHUNK_WIDTH * TILE_SIZE))
        };

        if (cameraChunkPos.x != currentChunkPos.x || cameraChunkPos.y != currentChunkPos.y) {
            chunk_manager_relocate(cameraChunkPos);
            currentChunkPos = cameraChunkPos;
        }

        BeginDrawing();
        ClearBackground((Color){ 122, 122, 170 });
        BeginMode2D(camera);
        
        chunk_manager_draw();

        if (!item_container_is_open()) {
            float cos = 47.5f * cosf(GetTime() * 4.0f) + 79.5f;

            DrawRectangle(
                mouseBlockPos.x * TILE_SIZE,
                mouseBlockPos.y * TILE_SIZE,
                TILE_SIZE,
                TILE_SIZE,
                (Color) {
                255, 255, 255, cos
            }
            );
        }

        EndMode2D();

        sprintf(buffer,
            "FPS: %d\n"
            "Loaded chunk area: %dx%d\n"
            "Mouse position: (%f, %f)\n"
            "Mouse block position: (%d, %d)\n"
            "Chunk position: (%d, %d)\n"
            "Holding item: %s\n"
            "Wall mode: %s",

            GetFPS(),
            CHUNK_VIEW_WIDTH,
            CHUNK_VIEW_HEIGHT,
            mouseWorldPos.x,
            mouseWorldPos.y,
            mouseBlockPos.x,
            mouseBlockPos.y,
            cameraChunkPos.x,
            cameraChunkPos.y,
            ir_get_item_registry(inventory_get_item(0, hotbarIdx).item_id)->name,
            wall_mode ? "true" : "false"
        );

        Vector2 textSize = MeasureTextEx(GetFontDefault(), buffer, 24, 0);
        DrawText(buffer, 0, 0, 24, WHITE);

        if (!item_container_is_open()) {
            if (inventory_get_item(0, hotbarIdx).item_id > 0) {
                DrawTexturePro(
                    texture_atlas_get(),
                    texture_atlas_get_rect(ir_get_item_registry(inventory_get_item(0, hotbarIdx).item_id)->atlas_idx, false, false),
                    (Rectangle) {
                    .x = GetMouseX(),
                        .y = GetMouseY(),
                        .width = TILE_SIZE * 0.8f,
                        .height = TILE_SIZE * 0.8f
                },
                    Vector2Zero(),
                    0.0f,
                    WHITE
                );
            }

            DrawTexturePro(
                place_mode_icon,
                (Rectangle) {
                    .x = wall_mode ? 0 : 8,
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

        Vector2 center = { GetScreenWidth() / 2.0f, GetScreenHeight()};

        int width = (get_inventory()->columns * (ITEM_SLOT_SIZE + 4)) + 4;
        int height = (get_inventory()->rows * (ITEM_SLOT_SIZE + 4)) + 4;

        const Color hotbarBg = { 0, 0, 0, 128 };
        const Color selHotbarSlot = { 128, 128, 128, 128 };

        DrawRectangle(
            center.x - (width / 2),
            center.y - height,
            width,
            height,
            hotbarBg
        );

        for (int r = 0; r < get_inventory()->rows; r++) {
            for (int c = 0; c < get_inventory()->columns; c++) {
                int i = c + (r * get_inventory()->columns);

                Rectangle slotRect = {
                    .x = ((center.x - (width / 2)) + 4) + (c * (ITEM_SLOT_SIZE + 4)),
                    .y = ((center.y - height) + 4) + (r * (ITEM_SLOT_SIZE + 4)),
                    .width = ITEM_SLOT_SIZE,
                    .height = ITEM_SLOT_SIZE
                };

                DrawRectangleRec(slotRect, i == hotbarIdx ? selHotbarSlot : hotbarBg);

                draw_item(&get_inventory()->items[i], slotRect.x, slotRect.y);
            }
        }

        if (!item_container_is_open()) {
            interPanel.y = textSize.y;

            mouseIsInUI = CheckCollisionPointRec(GetMousePosition(), interPanel);

            const int padding = 8;
            const int elementHeight = 16;
            const int sum = elementHeight + padding;
            int height = (-elementHeight) + (padding/2);

            GuiPanel(interPanel, NULL);
            if (GuiValueBox((Rectangle) { MeasureText("Seed ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Seed ", & seed, INT_MIN, INT_MAX, seedEdit)) seedEdit = !seedEdit;
            GuiCheckBox((Rectangle) { padding, textSize.y + (height += sum), elementHeight, elementHeight }, "Toggle Wall AO", & wallAmbientOcclusion);
            GuiCheckBox((Rectangle) { padding, textSize.y + (height += sum), elementHeight, elementHeight }, "Toggle Smooth Lighting", &smoothLighting);
            if (GuiValueBox((Rectangle) { MeasureText("Wall Brightness  ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Wall Brightness ", &wallBrightness, 0, 255, wallBrightEdit)) wallBrightEdit = !wallBrightEdit;
            if (GuiValueBox((Rectangle) { MeasureText("Wall AO Value  ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Wall AO Value ", &wallAOvalue, 0, 255, wallAOEdit)) wallAOEdit = !wallAOEdit;
            if (GuiButton((Rectangle) { padding, textSize.y + (height += sum), interPanel.width - (padding * 2), 32 }, "Reload chunks")) {
                chunk_manager_reload_chunks();
            }

            interPanel.height = height + sum + 16;
        }

        item_container_draw();

        EndDrawing();
    }
    
    free_inventory();
    item_container_free(&creativeMenu);
    item_registry_free();
    chunk_manager_free();
    block_registry_free();
    texture_atlas_free();
    CloseWindow();
    return 0;
}