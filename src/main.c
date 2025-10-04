#include "game_settings.h"
#include "item_container.h"
#include "world_manager.h"
#include "chunk_manager.h"
#include "game.h"

#include <limits.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define TICK_DELTA (1.0f / 20.0f)

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "thirdparty/raylib-nuklear.h"

const char* splashTexts[] = {
    "Made with Raylib!",
	"Written entirely in C!",
    "NOT Terraria!",
    "NOT written in Rust!",
    "Segfault free!",
    "2^Box"
};

#define SPLASH_TEXT_COUNT (sizeof(splashTexts) / sizeof(splashTexts[0]))

typedef enum {
	MENU_STATE_MAIN,
	MENU_STATE_SETTINGS,
    MENU_STATE_WORLDS,
	MENU_STATE_PAUSED
} MenuState;

MenuState menuState = MENU_STATE_MAIN;
bool paused = false;
char fpsBuf[8];

float bounce_wave(float x, float min, float max) {
    float f = fabs(sin(x));
    return min + (max - min) * f;
}

void collect_game_info() {
    Player* player = game_get_player();
    if (player) {
        get_world_info()->player_position = player_get_position(player);
        get_world_info()->player_flying = !player->entity.gravity_affected;
        for (int i = 0; i < 10; i++) {
            get_world_info()->hotbar_items[i] = inventory_get_item(0, i);
        }
    }
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1280, 720, "squarebox");
    SetExitKey(KEY_NULL);
    SetTraceLogLevel(LOG_WARNING);
    rlDisableBackfaceCulling();

    int fontSize = 24;
    struct nk_context* ctx = InitNuklear(fontSize);

    load_game_settings();
    if (get_game_settings()->vsync) {
		SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
	}

    world_manager_init();
    
    game_init();
    
    #ifdef LOAD_WORLD
    if (world_manager_load_world_info(TextFormat("worlds/%s", LOAD_WORLD))) {
        game_set_demo_mode(false);
        game_set_draw_ui(true);
        menuState = MENU_STATE_PAUSED;
    }
    #endif

    Texture2D logo = LoadTexture(ASSETS_PATH "logo.png");

    Label splashLabel = create_label(splashTexts[GetRandomValue(0, (SPLASH_TEXT_COUNT - 1))], 32.0f, 2.0f, GetFontDefault());

	Label versionLabel = create_label("Version InDev", 24.0f, 2.0f, GetFontDefault());
	Label creditsLabel = create_label("Made by pvini07BR", 24.0f, 2.0f, GetFontDefault());

    float accumulator = 0.0f;
    bool closeGame = false;


    while (!WindowShouldClose() && !closeGame) {
        if (IsKeyPressed(KEY_F11)) ToggleBorderlessWindowed();

        if (!game_is_ui_open() && IsKeyPressed(KEY_ESCAPE)) {
            if (!game_is_demo_mode() && menuState == MENU_STATE_PAUSED) {
                game_set_draw_ui(paused);
                paused = !paused;
            }
            else if (menuState == MENU_STATE_SETTINGS) {
                if (!game_is_demo_mode()) {
                    menuState = MENU_STATE_PAUSED;
                }
                else {
                    menuState = MENU_STATE_MAIN;
                }
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

        UpdateNuklear(ctx);
        if (!game_is_demo_mode() && paused) {
            DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) { 0, 0, 0, 128 });
        }

        if (menuState == MENU_STATE_MAIN || (menuState == MENU_STATE_PAUSED && paused)) {
            Vector2 screenCenter = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

            if (menuState == MENU_STATE_MAIN) {
                Vector2 logoPos = { screenCenter.x - (logo.width / 2.0f), (GetScreenHeight() / 8.0f) - (logo.height / 2.0f)};
                DrawTexture(logo, logoPos.x, logoPos.y, WHITE);

                draw_label(
                    &splashLabel,
                    Vector2Add(logoPos, Vector2SubtractValue((Vector2) { logo.width, logo.height }, 15.0f)),
                    -20.0f,
                    bounce_wave(GetTime() * 6.0f, 0.9f, 1.0f),
                    YELLOW,
                    LABEL_ALIGN_CENTER,
                    LABEL_ALIGN_CENTER
                );

                draw_label(
                    &versionLabel,
                    (Vector2) { 0.0f, GetScreenHeight() },
                    0.0f,
                    1.0f,
                    WHITE,
                    LABEL_ALIGN_BEGIN,
                    LABEL_ALIGN_END
                );

                draw_label(
                    &creditsLabel,
                    (Vector2) { GetScreenWidth(), GetScreenHeight() },
                    0.0f,
                    1.0f,
                    WHITE,
                    LABEL_ALIGN_END,
                    LABEL_ALIGN_END
                );
            }

            Vector2 menuSize = (Vector2){ 220, 225 };

            nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_hide());

            if (nk_begin(ctx, "Menu", nk_rect(screenCenter.x - (menuSize.x / 2.0f), screenCenter.y - (menuSize.y / 2.0f), menuSize.x, menuSize.y), NK_WINDOW_NO_SCROLLBAR)) {
                nk_layout_row_dynamic(ctx, 40.0f, 1);
                if (nk_button_label(ctx, menuState == MENU_STATE_MAIN ? "Play" : "Resume")) {
                    if (menuState == MENU_STATE_MAIN) {
                        menuState = MENU_STATE_WORLDS;
                    }
                    else if (menuState == MENU_STATE_PAUSED) {
                        game_set_draw_ui(true);
                        paused = false;
                    }
                }
                nk_spacing(ctx, 1);
                if (nk_button_label(ctx, "Settings")) {
                    menuState = MENU_STATE_SETTINGS;
                }
                nk_spacing(ctx, 1);
                if (nk_button_label(ctx, menuState == MENU_STATE_MAIN ? "Quit" : "Quit & Save")) {
                    if (menuState == MENU_STATE_MAIN) {
                        closeGame = true;
                    }
                    else if (menuState == MENU_STATE_PAUSED) {
                        collect_game_info();
                        game_set_demo_mode(true);
                        world_manager_save_world_info_and_unload();
                        chunk_manager_relocate((Vector2i) { 0, 0 });
                        paused = false;
                        menuState = MENU_STATE_MAIN;
                    }
                }
            }
            nk_end(ctx);

            nk_style_pop_style_item(ctx);
        }
        else if (menuState == MENU_STATE_SETTINGS) {
            if (game_settings_draw(ctx)) {
                if (!game_is_demo_mode()) {
                    menuState = MENU_STATE_PAUSED;
                }
                else {
                    menuState = MENU_STATE_MAIN;
                }
            }
        }
        else if (menuState == MENU_STATE_WORLDS && game_is_demo_mode()) {
            WorldListReturnType ret = world_manager_draw_list(ctx);
            if (ret == WORLD_RETURN_CLOSE) {
                menuState = MENU_STATE_MAIN;
            } else if (ret == WORLD_RETURN_OPEN_WORLD) {
                game_set_demo_mode(false);
                game_set_draw_ui(true);
                menuState = MENU_STATE_PAUSED;
            }
        }

        DrawNuklear(ctx);

        if (get_game_settings()->drawfps) {
            sprintf(fpsBuf, "FPS: %d", GetFPS());
            DrawText(fpsBuf, 0, 0, 24.0f, WHITE);
        }

        EndDrawing();
    }

    if (!game_is_demo_mode() && world_manager_is_world_loaded()) {
	    collect_game_info();
        world_manager_save_world_info();
    }

    game_free();
    world_manager_free();

    UnloadNuklear(ctx);

    CloseWindow();

    return 0;
}