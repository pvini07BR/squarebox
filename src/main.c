#include "item_container.h"
#include "game_settings.h"
#include "sign_editor.h"
#include "game.h"

#include <limits.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define RAYGUI_IMPLEMENTATION
#include "thirdparty/raygui.h"

#define TICK_DELTA (1.0f / 20.0f)

// extern int seed;
// extern bool wallAmbientOcclusion;
// extern bool smoothLighting;
// extern unsigned int wallBrightness;
// extern unsigned int wallAOvalue;

// extern bool drawChunkLines;

// unsigned int temp_chunk_view_width = 5;
// unsigned int temp_chunk_view_height = 3;

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "squaredbox");
    SetExitKey(KEY_NULL);
    SetTraceLogLevel(LOG_WARNING);
    rlDisableBackfaceCulling();
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24.0f);

    load_game_settings();

    bool paused = false;

    game_init();

    float accumulator = 0.0f;
    while (!WindowShouldClose()) {
        if (!item_container_is_open() && !sign_editor_is_open() && IsKeyPressed(KEY_ESCAPE)) {
            paused = !paused;
        }

        if (!paused) {
            // Tick chunks
            accumulator += GetFrameTime();
            while (accumulator >= TICK_DELTA) {
                game_tick();
                accumulator -= TICK_DELTA;
            }
    
            game_update(GetFrameTime());
        }

        BeginDrawing();

        ClearBackground(BLACK);

        game_draw(!paused);

        if (paused) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, 128 });

            const char* pausedTextStr = "PAUSED";
            const float pausedTextSize = 48.0f;
            const float pausedTextSpacing = pausedTextSize / 8.0f;

            Vector2 pausedTextMeasure = MeasureTextEx(GetFontDefault(), pausedTextStr, pausedTextSize, pausedTextSpacing);
            const int numButtons = 2;
            const float buttonW = 200.0f;
            const float buttonH = 40.0f;
            const float gapBetweenButtons = 16.0f;
            const float gapTitleToButtons = 24.0f;

            float totalHeight = pausedTextMeasure.y
                                + gapTitleToButtons
                                + (numButtons * buttonH)
                                + ((numButtons - 1) * gapBetweenButtons);

            float centerX = GetScreenWidth() * 0.5f;
            float startY = GetScreenHeight() * 0.5f - totalHeight * 0.5f;

            DrawTextPro(
                GetFontDefault(),
                pausedTextStr,
                (Vector2){ centerX, startY + pausedTextMeasure.y / 2.0f },
                (Vector2){ pausedTextMeasure.x / 2.0f, pausedTextMeasure.y / 2.0f },
                0.0f,
                pausedTextSize,
                pausedTextSpacing,
                WHITE
            );

            float y = startY + pausedTextMeasure.y + gapTitleToButtons;

            float buttonX = centerX - buttonW * 0.5f;

            if (GuiButton((Rectangle){ buttonX, y, buttonW, buttonH }, "Resume")) {
                paused = false;
            }

            y += buttonH + gapBetweenButtons;

            if (GuiButton((Rectangle){ buttonX, y, buttonW, buttonH }, "Options")) {
                // TODO
            }

            y += buttonH + gapBetweenButtons;

            if (GuiButton((Rectangle){ buttonX, y, buttonW, buttonH }, "Quit")) {
                break;
            }
        }

        EndDrawing();
    }

    //     Vector2 textSize = MeasureTextEx(GetFontDefault(), buffer, 24, 0);
    //     DrawText(buffer, 0, 0, 24, WHITE);

    //     if (!item_container_is_open()) {
    //         interPanel.y = textSize.y;

    //         mouseIsInUI = CheckCollisionPointRec(GetMousePosition(), interPanel);

    //         const int padding = 8;
    //         const int elementHeight = 16;
    //         const int sum = elementHeight + padding;
    //         int height = (-elementHeight) + (padding/2);

    //         GuiPanel(interPanel, NULL);
    //         if (GuiValueBox((Rectangle) { MeasureText("Seed ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Seed ", & seed, INT_MIN, INT_MAX, seedEdit)) seedEdit = !seedEdit;
    //         if (GuiValueBox((Rectangle) { MeasureText("Chunk View Width ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Chunk View Width ", &temp_chunk_view_width, 0, INT_MAX, chunkViewWidthEdit)) chunkViewWidthEdit = !chunkViewWidthEdit;
    //         if (GuiValueBox((Rectangle) { MeasureText("Chunk View Height ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Chunk View Height ", &temp_chunk_view_height, 0, INT_MAX, chunkViewHeightEdit)) chunkViewHeightEdit = !chunkViewHeightEdit;
    //         GuiCheckBox((Rectangle) { padding, textSize.y + (height += sum), elementHeight, elementHeight }, "Toggle Wall AO", & wallAmbientOcclusion);
    //         GuiCheckBox((Rectangle) { padding, textSize.y + (height += sum), elementHeight, elementHeight }, "Toggle Smooth Lighting", &smoothLighting);
    //         GuiCheckBox((Rectangle) { padding, textSize.y + (height += sum), elementHeight, elementHeight }, "Draw Chunk Lines", &drawChunkLines);
    //         if (GuiValueBox((Rectangle) { MeasureText("Wall Brightness  ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Wall Brightness ", &wallBrightness, 0, 255, wallBrightEdit)) wallBrightEdit = !wallBrightEdit;
    //         if (GuiValueBox((Rectangle) { MeasureText("Wall AO Value  ", 8) + padding, textSize.y + (height += sum), 64, elementHeight }, "Wall AO Value ", &wallAOvalue, 0, 255, wallAOEdit)) wallAOEdit = !wallAOEdit;
    //         if (GuiButton((Rectangle) { padding, textSize.y + (height += sum), interPanel.width - (padding * 2), 32 }, "Apply Settings")) {
    //             chunk_manager_set_view(temp_chunk_view_width, temp_chunk_view_height);
    //         }

    //         interPanel.height = height + sum + 16;
    //     }

    game_free();

    CloseWindow();

    return 0;
}