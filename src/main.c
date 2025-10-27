#include "game_settings.h"
#include "item_container.h"
#include "world_manager.h"
#include "chunk_manager.h"
#include "registries/texture_atlas.h"
#include "game.h"

#include <stdlib.h>
#include <limits.h>

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

#define TICK_DELTA (1.0f / 20.0f)

#include "thirdparty/microui.h"
#include "thirdparty/murl.h"

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

    mu_Context* ctx = malloc(sizeof(mu_Context));
    mu_init(ctx);

    Font font = LoadFontEx(ASSETS_PATH "nokiafc22.ttf", 20, NULL, 0);
    murl_setup_font_ex(ctx, &font);

    load_game_settings();
    if (get_game_settings()->vsync) {
		SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
	}

    texture_atlas_init();

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

        murl_handle_input(ctx);

        mu_begin(ctx);

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

            ctx->style->colors[MU_COLOR_WINDOWBG].a = 0;
            ctx->style->colors[MU_COLOR_BORDER].a = 0;

            if (mu_begin_window_ex(ctx, "Menu", mu_rect(0, 0, 0, 0), MU_OPT_NOTITLE | MU_OPT_NOSCROLL)) {
                mu_Container* win = mu_get_current_container(ctx);

                win->rect.w = win->content_size.x + (ctx->style->padding * 2);
                win->rect.h = win->content_size.y + (ctx->style->padding * 2);

                win->rect.x = (GetScreenWidth() / 2.0f) - (win->rect.w / 2.0f);
                win->rect.y = (GetScreenHeight() / 2.0f) - (win->rect.h / 2.0f);

                mu_layout_row(ctx, 1, (int[1]) { 300 }, 50);

                if (mu_button(ctx, menuState == MENU_STATE_MAIN ? "Play" : "Resume")) {
                    if (menuState == MENU_STATE_MAIN) {
                        menuState = MENU_STATE_WORLDS;
                    }
                    else if (menuState == MENU_STATE_PAUSED) {
                        game_set_draw_ui(true);
                        paused = false;
                    }
                }

                mu_layout_next(ctx);

                if (mu_button(ctx, "Settings")) {
                    menuState = MENU_STATE_SETTINGS;
                }

                mu_layout_next(ctx);

                if (mu_button(ctx, menuState == MENU_STATE_MAIN ? "Quit" : "Quit & Save")) {
                    if (menuState == MENU_STATE_MAIN) {
                        closeGame = true;
                    }
                    else if (menuState == MENU_STATE_PAUSED) {
                        collect_game_info();
                        game_set_demo_mode(true);
                        world_manager_save_world_info_and_unload();
                        chunk_manager_relocate((Vector2i) { 0, 0 });
                        chunk_manager_set_view(5, 3);
                        paused = false;
                        menuState = MENU_STATE_MAIN;
                    }
                }

                mu_end_window(ctx);
            }

            ctx->style->colors[MU_COLOR_WINDOWBG].a = 255;
            ctx->style->colors[MU_COLOR_BORDER].a = 255;
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

        mu_end(ctx);

        murl_render(ctx);

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

    CloseWindow();

    free(ctx);

    return 0;
}
