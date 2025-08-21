#include <stdio.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

#include "chunk_manager.h"
#include "block_registry.h"

extern int seed;
extern bool wallAmbientOcclusion;
extern bool smoothLighting;
extern unsigned int wallBrightness;
extern unsigned int wallAOvalue;

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

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "mijocraft");

    uint8_t selected_block = 1;
    bool wall_mode = false;
    
    Texture2D place_mode_icon = LoadTexture(ASSETS_PATH "place_modes.png");

    block_registry_init();
    chunk_manager_init();

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

        if (IsKeyPressed(KEY_TAB)) wall_mode = !wall_mode;

        mouseWorldPos = GetScreenToWorld2D(GetMousePosition(), camera);
        mouseBlockPos = (Vector2i){
            (int)floorf((float)mouseWorldPos.x / (float)TILE_SIZE),
            (int)floorf((float)mouseWorldPos.y / (float)TILE_SIZE)
        };

        if (mouseIsInUI) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
                chunk_manager_set_block(mouseBlockPos, 0, wall_mode);
            else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
                chunk_manager_set_block(mouseBlockPos, selected_block, wall_mode);
        }

        Vector2 input = { 0 };

        if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) input.x = -1.0f;
        else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) input.x = 1.0f;

        if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) input.y = -1.0f;
        else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) input.y = 1.0f;

        if (IsKeyPressed(KEY_EQUAL))camera.zoom *= 1.1f;
        else if (IsKeyPressed(KEY_MINUS)) camera.zoom /= 1.1f;

        int scroll = GetMouseWheelMoveV().y;
        if (selected_block > 1 && scroll < 0) selected_block--;
        if (selected_block < 8 && scroll > 0) selected_block++;

        input = Vector2Normalize(input);

        float speed = 500.0;
        if (IsKeyDown(KEY_LEFT_SHIFT)) {
            speed *= 4.0;
        }

        camera.target.x += input.x * speed * GetFrameTime();
        camera.target.y += input.y * speed * GetFrameTime();

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

        float cos = 47.5f * cosf(GetTime() * 4.0f) + 79.5f;

        DrawRectangle(
            mouseBlockPos.x * TILE_SIZE,
            mouseBlockPos.y * TILE_SIZE,
            TILE_SIZE,
            TILE_SIZE,
            (Color){ 255, 255, 255, cos }
        );

        EndMode2D();

        sprintf(buffer,
            "FPS: %d\n"
            "Loaded chunk area: %dx%d\n"
            "Mouse position: (%f, %f)\n"
            "Mouse block position: (%d, %d)\n"
            "Chunk position: (%d, %d)\n"
            "Selected block: %s\n"
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
            br_get_block_registry(selected_block)->name,
            wall_mode ? "true" : "false"
        );

        Vector2 textSize = MeasureTextEx(GetFontDefault(), buffer, 24, 0);
        DrawText(buffer, 0, 0, 24, WHITE);

        DrawTexturePro(
            *br_get_block_atlas(),
            br_get_block_texture_rect(selected_block, false, false),
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

        interPanel.y = textSize.y;

        mouseIsInUI = !CheckCollisionPointRec(GetMousePosition(), interPanel);

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
            chunk_manager_reset_chunks();
        }

        interPanel.height = height + sum + 16;

        EndDrawing();
    }
    
    chunk_manager_free();
    block_registry_free();
    CloseWindow();
    return 0;
}