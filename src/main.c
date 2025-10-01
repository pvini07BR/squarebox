#include "game_settings.h"
#include "world_manager.h"
#include "chunk_manager.h"
#include "game.h"

#include <limits.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <time.h>

#define RAYGUI_IMPLEMENTATION
#include "thirdparty/raygui.h"

#define TICK_DELTA (1.0f / 20.0f)

const char* splashTexts[] = {
    "Made with Raylib!",
	"Written entirely in C!",
    "NOT Terraria!",
    "NOT written in Rust!",
    "Segfault free!",
    "2^Box"
};

#define SPLASH_TEXT_COUNT (sizeof(splashTexts) / sizeof(splashTexts[0]))

bool settings = false;
bool paused = false;

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

    Label splashLabel = create_label(splashTexts[GetRandomValue(0, (SPLASH_TEXT_COUNT - 1))], 32.0f, 2.0f, GetFontDefault());

    Label playButtonLabel = create_label("Play", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());
    Label settingsButtonLabel = create_label("Settings", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());
    Label quitButtonLabel = create_label("Quit", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());

	Label versionLabel = create_label("Version InDev", 24.0f, 2.0f, GetFontDefault());
	Label creditsLabel = create_label("Made by pvini07BR", 24.0f, 2.0f, GetFontDefault());

	Label pausedLabel = create_label("Paused", 72.0f, 5.0f, GetFontDefault());
	Label resumeLabel = create_label("Resume", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());
	Label saveAndQuitLabel = create_label("Save & Quit", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());

    Vector2 bounds[] = {
        Vector2Add(saveAndQuitLabel.bounds, buttonPadding),
        Vector2Add(saveAndQuitLabel.bounds, buttonPadding),
        Vector2Add(saveAndQuitLabel.bounds, buttonPadding)
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
        if (!game_is_demo_mode()) {
            if (!game_is_ui_open() && IsKeyPressed(KEY_ESCAPE)) {
                paused = !paused;
            }
        }

        if (game_is_demo_mode() || (!game_is_demo_mode() && !paused)) {
            accumulator += GetFrameTime();
            while (accumulator >= TICK_DELTA) {
                game_tick();
                accumulator -= TICK_DELTA;
            }
    
            game_update(GetFrameTime());
        }

        BeginDrawing();

        ClearBackground(BLACK);

        game_draw();

        if (game_is_demo_mode() || (!game_is_demo_mode() && paused)) {
            if (!game_is_demo_mode() && paused) {
				DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) { 0, 0, 0, 128 });
            }

            if (!settings) {
			    Vector2 screenCenter = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

                if (game_is_demo_mode()) {
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
                }
                else {
                    DrawTextPro(
                        pausedLabel.font,
                        pausedLabel.str,
                        (Vector2) { GetScreenWidth() / 2.0f, GetScreenHeight() / 8.0f },
                        (Vector2) { pausedLabel.bounds.x / 2.0f, pausedLabel.bounds.y / 2.0f },
                        0.0f,
                        pausedLabel.fontSize,
						pausedLabel.spacing,
						WHITE
					);
                }

                screenCenter = Vector2Subtract(screenCenter, Vector2Scale(totalSize, 0.5f));

                for (int i = 0; i < 4; i++) {
                    Rectangle rect = { screenCenter.x + ((totalSize.x / 2.0f) - (bounds[i].x / 2.0f)), screenCenter.y, bounds[i].x, bounds[i].y};
					
                    const char* playButtonLabelText = game_is_demo_mode() ? playButtonLabel.str : resumeLabel.str;
					const char* quitButtonLabelText = game_is_demo_mode() ? quitButtonLabel.str : "Save & Quit";

                    switch (i) {
                    case 0:
                        if (GuiButton(rect, playButtonLabelText)) {
                            if (game_is_demo_mode()) {
                                if (world_manager_load_world_info("test_world")) {
                                    game_set_demo_mode(false);
                                    paused = false;
                                }
                            }
                            else {
                                paused = false;
                            }
                        }
                        break;
                    case 1:
                        if (GuiButton(rect, settingsButtonLabel.str)) {
                            settings = true;
                        }
                        break;
                    case 2:
                        if (GuiButton(rect, quitButtonLabelText)) {
                            if (game_is_demo_mode()) {
                                closeGame = true;
                            }
                            else {
                                Player* player = game_get_player();
                                if (player) get_world_info()->player_position = player_get_position(player);
                                game_set_demo_mode(true);
                                world_manager_save_world_info_and_unload();
                                chunk_manager_relocate((Vector2i) { 0, 0 });
                                paused = false;
							}
                        }
                        break;
                    }
                    screenCenter.y += bounds[i].y + spacing;
                }

                if (game_is_demo_mode()) {
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
            }
            else {
                if (game_settings_draw()) {
                    settings = false;
                }
            }
        }

        EndDrawing();
    }

    if (!game_is_demo_mode() && world_manager_is_world_loaded()) {
	    Player* player = game_get_player();
        if (player) {
		    get_world_info()->player_position = player_get_position(player);
        }
        world_manager_save_world_info();
    }

    game_free();

    CloseWindow();

    return 0;
}