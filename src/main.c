#include <stdio.h>
#include <limits.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define RAYGUI_IMPLEMENTATION
#include "thirdparty/raygui.h"

#include "types.h"
#include "texture_atlas.h"
#include "chunk_manager.h"
#include "block_registry.h"
#include "item_registry.h"
#include "item_container.h"
#include "block_models.h"
#include "block_colliders.h"
#include "player.h"

#define TICK_DELTA (1.0f / 20.0f)

extern int seed;
extern bool wallAmbientOcclusion;
extern bool smoothLighting;
extern unsigned int wallBrightness;
extern unsigned int wallAOvalue;

extern bool drawChunkLines;

unsigned int temp_chunk_view_width = 5;
unsigned int temp_chunk_view_height = 3;

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "mijocraft");
    SetExitKey(KEY_NULL);
    SetTraceLogLevel(LOG_WARNING);
    rlDisableBackfaceCulling();

    bool wall_mode = false;
    ItemContainer creativeMenu;
    int8_t hotbarIdx = 0;
    int blockState = 0;

    int8_t last_hotbarIdx = -1;
    size_t last_blockState = SIZE_MAX;
    uint8_t lastItemId = UCHAR_MAX;
    
    bool loadedGhostMesh = false;
    Mesh ghostBlockMesh = { 0 };

    bool mouseIsInUI = false;

    bool seedEdit = false;
    bool wallBrightEdit = false;
    bool wallAOEdit = false;

    bool chunkViewWidthEdit = false;
    bool chunkViewHeightEdit = false;

    Rectangle interPanel = {
        .x = 0,
        .y = 0,
        .width = 170,
        .height = 32 * 3
    };
    
    Texture2D place_mode_icon = LoadTexture(ASSETS_PATH "place_modes.png");

    texture_atlas_load(ASSETS_PATH "atlas.png", 2, 11);

    block_models_init();
    block_colliders_init();

    item_registry_init();
    block_registry_init();

    chunk_manager_init(5, 3);

    item_container_create(&creativeMenu, "Creative Menu", 3, 10, true);
    for (int i = 1; i < ITEM_COUNT; i++) {
        item_container_set_item(&creativeMenu, (i - 1) / creativeMenu.columns, (i - 1) % creativeMenu.columns, (ItemSlot){ i, 1 });
    }

    init_inventory();

    Camera2D camera = {
        .offset =  { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f },
        .rotation = 0.0f,
        .zoom = 1.0f
    };

    Vector2 mouseWorldPos = Vector2Zero();
    Vector2i mouseBlockPos = { 0, 0 };
    Vector2i currentChunkPos = { 0, 0 };

    Rectangle blockPlacerRect = {
        0, 0, TILE_SIZE, TILE_SIZE
    };

    char buffer[1024];
    float accumulator = 0.0f;

    Player player;
    player_init(&player, (Vector2){ 0, -TILE_SIZE });
    camera.target = Vector2Add(player_get_position(&player), Vector2Scale(player_get_size(&player), 0.5f));

    while (!WindowShouldClose()) {
        // Tick chunks
		accumulator += GetFrameTime();
        while (accumulator >= TICK_DELTA) {
            chunk_manager_tick();
            accumulator -= TICK_DELTA;
		}

        camera.offset = (Vector2){
            .x = GetScreenWidth() / 2.0f, 
            .y = GetScreenHeight() / 2.0f
        };

        mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        mouseBlockPos = (Vector2i){
            (int)floorf((float)mouseWorldPos.x / (float)TILE_SIZE),
            (int)floorf((float)mouseWorldPos.y / (float)TILE_SIZE)
        };
        blockPlacerRect.x = mouseBlockPos.x * TILE_SIZE;
        blockPlacerRect.y = mouseBlockPos.y * TILE_SIZE;

        if (!item_container_is_open()) {
            if (IsKeyPressed(KEY_TAB)) wall_mode = !wall_mode;

            if (!mouseIsInUI) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                    chunk_manager_set_block_safe(mouseBlockPos, (BlockInstance) { 0, 0 }, wall_mode);
                else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                    if (!chunk_manager_interact(mouseBlockPos, wall_mode)) {
                        ItemRegistry* itr = ir_get_item_registry(inventory_get_item(0, hotbarIdx).item_id);
                        if (itr->blockId > 0 && !(itr->placingFlags & (wall_mode ? ITEM_PLACE_FLAG_NOT_WALL : ITEM_PLACE_FLAG_NOT_BLOCK))) {
                            BlockRegistry* br = br_get_block_registry(itr->blockId);

                            if (wall_mode || (!wall_mode && ((br->flags & BLOCK_FLAG_SOLID && !CheckCollisionRecs(blockPlacerRect, player.entity.rect) || !(br->flags & BLOCK_FLAG_SOLID))))) {
                                BlockInstance inst = {
                                    .id = itr->blockId,
                                    .state = blockState
                                };
                                chunk_manager_set_block_safe(mouseBlockPos, inst, wall_mode);
                            }
                        }
                    }
                }
            }

            if (IsKeyPressed(KEY_F)) {
                player.flying = !player.flying;
            }

            if (IsKeyPressed(KEY_C)) {
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
            }
            else {
                if (scroll > 0) hotbarIdx--;
                if (scroll < 0) hotbarIdx++;

                if (hotbarIdx < 0) hotbarIdx = 9;
                if (hotbarIdx > 9) hotbarIdx = 0;
            }

            if (!seedEdit && !wallAOEdit && !wallBrightEdit && !chunkViewWidthEdit && !chunkViewHeightEdit) {
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
        }

        player_update(&player, GetFrameTime(), item_container_is_open());
            
        Vector2 newTarget = Vector2Add(player_get_position(&player), Vector2Scale(player_get_size(&player), 0.5f));
        camera.target = Vector2Lerp(camera.target, newTarget, 25.0f * GetFrameTime());

        if (IsKeyPressed(KEY_E) && !item_container_is_open())
            item_container_open(&creativeMenu);
        else if ((IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ESCAPE) && item_container_is_open()))
            item_container_close();

        ItemSlot heldItem = inventory_get_item(0, hotbarIdx);
        ItemRegistry* heldItemReg = ir_get_item_registry(heldItem.item_id);
        BlockRegistry* heldBlockReg = br_get_block_registry(heldItemReg->blockId);
        if (heldItemReg->blockId > 0 && heldBlockReg->flags & BLOCK_FLAG_STATE_MUTABLE) {
            bool reload = false;

            if (lastItemId != heldItem.item_id) reload = true;

            if (IsKeyPressed(KEY_Z) || IsKeyPressed(KEY_X)) {
                if (IsKeyPressed(KEY_Z)) blockState--;
                if (IsKeyPressed(KEY_X)) blockState++;

                if (blockState < 0) blockState = heldBlockReg->variant_count - 1;
                if (blockState >= heldBlockReg->variant_count) blockState = 0;

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
                BlockVariant bvar = br_get_block_variant(heldItemReg->blockId, blockState);
                block_models_build_mesh(&ghostBlockMesh, bvar.model_idx, bvar.atlas_idx, false, false, bvar.flipH, bvar.flipV, bvar.rotation);
                loadedGhostMesh = true;
            }
        } else {
            blockState = 0;
            if (loadedGhostMesh == true) {
                UnloadMesh(ghostBlockMesh);
                ghostBlockMesh = (Mesh){ 0 };
                loadedGhostMesh = false;
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

        ClearBackground(BLACK);

        // Draw sky gradient
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
        
        chunk_manager_draw();

        player_draw(&player);

        chunk_manager_draw_liquids();

        if (!item_container_is_open()) {
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

        sprintf(buffer,
            "FPS: %d\n"
            "Loaded chunk area: %dx%d\nCamera Zoom: %f\n"
            "Mouse position: (%f, %f)\n"
            "Mouse block position: (%d, %d)\n"
            "Chunk position: (%d, %d)\n"
            "Holding item: %s\n"
            "Wall mode: %s",

            GetFPS(),
            chunk_manager_get_view_width(),
            chunk_manager_get_view_height(),
            camera.zoom,
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
                draw_item(inventory_get_item(0, hotbarIdx), GetMouseX(), GetMouseY(), 0, 0.8f, false);
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

        if (!item_container_is_open()) {
            interPanel.y = textSize.y;

            mouseIsInUI = CheckCollisionPointRec(GetMousePosition(), interPanel);

            const int padding = 8;
            const int elementHeight = 16;
            const int sum = elementHeight + padding;
            int height = (-elementHeight) + (padding/2);

            GuiPanel(interPanel, NULL);
            if (GuiValueBox((Rectangle) { MeasureText("Seed ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Seed ", & seed, INT_MIN, INT_MAX, seedEdit)) seedEdit = !seedEdit;
            if (GuiValueBox((Rectangle) { MeasureText("Chunk View Width ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Chunk View Width ", &temp_chunk_view_width, 0, INT_MAX, chunkViewWidthEdit)) chunkViewWidthEdit = !chunkViewWidthEdit;
            if (GuiValueBox((Rectangle) { MeasureText("Chunk View Height ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Chunk View Height ", &temp_chunk_view_height, 0, INT_MAX, chunkViewHeightEdit)) chunkViewHeightEdit = !chunkViewHeightEdit;
            GuiCheckBox((Rectangle) { padding, textSize.y + (height += sum), elementHeight, elementHeight }, "Toggle Wall AO", & wallAmbientOcclusion);
            GuiCheckBox((Rectangle) { padding, textSize.y + (height += sum), elementHeight, elementHeight }, "Toggle Smooth Lighting", &smoothLighting);
            GuiCheckBox((Rectangle) { padding, textSize.y + (height += sum), elementHeight, elementHeight }, "Draw Chunk Lines", &drawChunkLines);
            if (GuiValueBox((Rectangle) { MeasureText("Wall Brightness  ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Wall Brightness ", &wallBrightness, 0, 255, wallBrightEdit)) wallBrightEdit = !wallBrightEdit;
            if (GuiValueBox((Rectangle) { MeasureText("Wall AO Value  ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Wall AO Value ", &wallAOvalue, 0, 255, wallAOEdit)) wallAOEdit = !wallAOEdit;
            if (GuiButton((Rectangle) { padding, textSize.y + (height += sum), interPanel.width - (padding * 2), 32 }, "Apply Settings")) {
                chunk_manager_set_view(temp_chunk_view_width, temp_chunk_view_height);
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
    block_models_free();
    texture_atlas_free();
    CloseWindow();
    return 0;
}