#include "item_container.h"
#include "game_settings.h"
#include "world_manager.h"
#include "sign_editor.h"
#include "game.h"

#include <limits.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <time.h>

#define RAYGUI_IMPLEMENTATION
#include "thirdparty/raygui.h"

#define TICK_DELTA (1.0f / 20.0f)

#define SPLASH_TEXT_COUNT 3
const char* splashTexts[] = {
    "Made with Raylib!",
	"Written entirely in C!",
    "NOT Terraria!"
};

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "squarebox");
    SetExitKey(KEY_NULL);
    SetTraceLogLevel(LOG_WARNING);
    rlDisableBackfaceCulling();
    GuiSetStyle(DEFAULT, TEXT_SIZE, 24.0f);

    load_game_settings();

    world_manager_init();
    world_manager_create_world((WorldInfo) {
        .name = "Test World",
        .seed = 12345,
        .player_position = { 0, 0 }
	});

    game_init();

    const float spacing = 50.0f;
    const Vector2 buttonPadding = { 30.0f, 20.0f };

    Texture2D logo = LoadTexture(ASSETS_PATH "logo.png");

	//SetRandomSeed(time(NULL));
    Label splashLabel = create_label(splashTexts[GetRandomValue(0, (SPLASH_TEXT_COUNT - 1))], 32.0f, 4.0f, GetFontDefault());

    Label playButtonLabel = create_label("Play", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());
    Label settingsButtonLabel = create_label("Settings", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());
    Label quitButtonLabel = create_label("Quit", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());

	Label versionLabel = create_label("Version InDev", 24.0f, 2.0f, GetFontDefault());
	Label creditsLabel = create_label("Made by pvini07BR", 24.0f, 2.0f, GetFontDefault());

    Vector2 bounds[] = {
        Vector2Add(settingsButtonLabel.bounds, buttonPadding),
        Vector2Add(settingsButtonLabel.bounds, buttonPadding),
        Vector2Add(settingsButtonLabel.bounds, buttonPadding)
    };

    Vector2 totalSize = { 0.0f, 0.0f };
    for (size_t i = 0; i < 3; i++) {
        totalSize.x = fmaxf(totalSize.x, bounds[i].x);
        totalSize.y += bounds[i].y;
		if (i < 2) totalSize.y += spacing;
	}

    float accumulator = 0.0f;
    bool closeGame = false;

    while (!WindowShouldClose() && !closeGame) {
        accumulator += GetFrameTime();
        while (accumulator >= TICK_DELTA) {
            game_tick();
            accumulator -= TICK_DELTA;
        }
    
        game_update(GetFrameTime());

        BeginDrawing();

        ClearBackground(BLACK);

        game_draw();

        if (game_is_demo_mode()) {
			Vector2 screenCenter = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
            Vector2 logoPos = { screenCenter.x - (logo.width / 2.0f), (GetScreenHeight() / 8.0f) - (logo.height / 2.0f)};

            DrawTexture(logo, logoPos.x, logoPos.y, WHITE);

            DrawTextPro(
                splashLabel.font,
                splashLabel.str,
                Vector2Add(logoPos, Vector2SubtractValue((Vector2) { logo.width, logo.height }, 15.0f)),
                (Vector2) { splashLabel.bounds.x / 2.0f, splashLabel.bounds.y / 2.0f },
                -20.0f,
                splashLabel.fontSize,
                splashLabel.spacing,
                YELLOW
            );

            screenCenter = Vector2Subtract(screenCenter, Vector2Scale(totalSize, 0.5f));

			//DrawRectangle(screenCenter.x, screenCenter.y, totalSize.x, totalSize.y, (Color) { 0, 0, 0, 128 });

            for (int i = 0; i < 4; i++) {
                Rectangle rect = { screenCenter.x + ((totalSize.x / 2.0f) - (bounds[i].x / 2.0f)), screenCenter.y, bounds[i].x, bounds[i].y};
                //DrawRectangleRec(rect, (Color) { (i == 0) * 255, (i == 1) * 255, (i == 2) * 255, 255 });
                switch (i) {
                case 0:
                    if (GuiButton(rect, playButtonLabel.str)) {
                        if (world_manager_load_world_info("test_world")) {
                            game_set_demo_mode(false);
                        }
                    }
                    break;
                case 1:
                    if (GuiButton(rect, settingsButtonLabel.str)) {
                        // Open settings
                    }
                    break;
                case 2:
                    if (GuiButton(rect, quitButtonLabel.str)) {
                        closeGame = true;
                    }
                    break;
                }
                screenCenter.y += bounds[i].y + spacing;
            }

            DrawTextPro(
                versionLabel.font,
                versionLabel.str,
                (Vector2) { 0.0f, GetScreenHeight() },
                (Vector2) { 0.0f, creditsLabel.bounds.y },
                0.0f,
                versionLabel.fontSize,
                versionLabel.spacing,
                WHITE
            );

            DrawTextPro(
                creditsLabel.font,
                creditsLabel.str,
                (Vector2) { GetScreenWidth(), GetScreenHeight() },
                (Vector2) { creditsLabel.bounds.x, creditsLabel.bounds.y },
                0.0f,
                creditsLabel.fontSize,
                creditsLabel.spacing,
                WHITE
            );
        }
        /*

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
        */

        EndDrawing();
    }

    if (!game_is_demo_mode() && world_manager_is_world_loaded()) {
	    Player* player = game_get_player();
        if (player) {
		    get_world_info()->player_position = player_get_position(player);
        }
        world_manager_save_world_info_and_unload();
    }

    game_free();

    CloseWindow();

    return 0;
}