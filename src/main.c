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

typedef enum {
    GAME_RUNNING,
    GAME_PAUSED,
    GAME_SETTINGS
} GameState;

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "squaredbox");
    SetExitKey(KEY_NULL);
    SetTraceLogLevel(LOG_WARNING);
    rlDisableBackfaceCulling();
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24.0f);

    load_game_settings();

    GameState state = GAME_RUNNING;

    game_init();

    float accumulator = 0.0f;
    while (!WindowShouldClose()) {
        if (!item_container_is_open() && !sign_editor_is_open() && IsKeyPressed(KEY_ESCAPE)) {
            if (state == GAME_RUNNING) state = GAME_PAUSED;
            else if (state == GAME_PAUSED) state = GAME_RUNNING;
            else if (state == GAME_SETTINGS) state = GAME_PAUSED;
        }

        if (state == GAME_RUNNING) {
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

        game_draw(state == GAME_RUNNING);

        if (state != GAME_RUNNING) DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){ 0, 0, 0, 128 });

        if (state == GAME_PAUSED) {
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
                state = GAME_RUNNING;
            }

            y += buttonH + gapBetweenButtons;

            if (GuiButton((Rectangle){ buttonX, y, buttonW, buttonH }, "Settings")) {
                state = GAME_SETTINGS;
            }

            y += buttonH + gapBetweenButtons;

            if (GuiButton((Rectangle){ buttonX, y, buttonW, buttonH }, "Quit")) {
                break;
            }
        }
        else if (state == GAME_SETTINGS) {
            // This function returns true when the back button is pressed
            if (game_settings_draw()) {
                state = GAME_PAUSED;
            }
        }

        EndDrawing();
    }

    game_free();

    CloseWindow();

    return 0;
}