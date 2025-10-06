#include "game_settings.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <raylib.h>
#include <raymath.h>

#include "chunk_manager.h"
#include "game.h"
#include "entity/player.h"
#include "thirdparty/microui.h"

static GameSettings settings = {
	.player_color = { 255, 0, 0, 255 },
	.vsync = true,
	.chunk_view_width = 5,
	.chunk_view_height = 3,
	.wall_brightness = 128,
	.wall_ao_brightness = 64,
	.smooth_lighting = true,
	.wall_ao = true,
};

static TempGameSettings tempSettings;

void game_settings_to_temp() {
	tempSettings.player_color[0] = settings.player_color.r;
	tempSettings.player_color[1] = settings.player_color.g;
	tempSettings.player_color[2] = settings.player_color.b;

	tempSettings.vsync = settings.vsync;
	tempSettings.drawfps = settings.drawfps;
	tempSettings.chunk_view_width = settings.chunk_view_width;
	tempSettings.chunk_view_height = settings.chunk_view_height;
	tempSettings.wall_brightness = settings.wall_brightness;
	tempSettings.wall_ao_brightness = settings.wall_ao_brightness;
	tempSettings.smooth_lighting = settings.smooth_lighting;
	tempSettings.wall_ao = settings.wall_ao;
}

void temp_to_game_settings() {
	settings.player_color.r = tempSettings.player_color[0];
	settings.player_color.g = tempSettings.player_color[1];
	settings.player_color.b = tempSettings.player_color[2];

	settings.vsync = tempSettings.vsync;
	settings.drawfps = tempSettings.drawfps;
	settings.chunk_view_width = (uint8_t)Clamp(tempSettings.chunk_view_width, 1, GAME_SETTINGS_MAX_CHUNK_VIEW);
	settings.chunk_view_height = (uint8_t)Clamp(tempSettings.chunk_view_height, 1, GAME_SETTINGS_MAX_CHUNK_VIEW);
	settings.wall_brightness = (uint8_t)Clamp(tempSettings.wall_brightness, 0.0f, 255.0f);
	settings.wall_ao_brightness = (uint8_t)Clamp(tempSettings.wall_ao_brightness, 0.0f, 255.0f);
	settings.smooth_lighting = tempSettings.smooth_lighting;
	settings.wall_ao = tempSettings.wall_ao;
}

bool save_game_settings() {
	FILE* file_ptr = fopen(GAME_SETTINGS_FILE_NAME, "wb");
	if (!file_ptr) {
		TraceLog(LOG_ERROR, "Could not open config.bin: %s", strerror(errno));
		return false;
	}
	size_t written = fwrite(&settings, sizeof(GameSettings), 1, file_ptr);

	if (written != 1) {
		TraceLog(LOG_ERROR, "Could not write to the settings file: %s", strerror(errno));
		fclose(file_ptr);
		return false;
	}

	fclose(file_ptr);

	return true;
}

bool load_game_settings() {
	game_settings_to_temp();

	FILE* file_ptr = fopen(GAME_SETTINGS_FILE_NAME, "rb");
	if (!file_ptr) {
		if (errno == ENOENT) {
			TraceLog(LOG_WARNING, "The config file config.bin was not found. The default settings will be used instead.");
		}
		else {
			TraceLog(LOG_ERROR, "Could not open config.bin: %s", strerror(errno));
		}
		return false;
	}

	GameSettings loadedSettings;

	size_t written = fread(&loadedSettings, sizeof(GameSettings), 1, file_ptr);
	if (written != 1) {
		TraceLog(LOG_ERROR, "Could not read to the settings file: %s", strerror(errno));
		fclose(file_ptr);
		return false;
	}

	fclose(file_ptr);

	settings = loadedSettings;
	game_settings_to_temp();

	return true;
}

bool game_settings_draw(mu_Context* ctx) {
	bool backPressed = false;

	if (IsKeyPressed(KEY_ESCAPE)) return true;

	if (mu_begin_window_ex(ctx, "Game Settings", mu_rect(0, 0, 500, 0), MU_OPT_NOSCROLL | MU_OPT_NOCLOSE)) {
		mu_Container* win = mu_get_current_container(ctx);

		win->rect.h = win->content_size.y + (ctx->style->padding * 2) + 25;

		win->rect.x = (GetScreenWidth() / 2.0f) - (win->rect.w / 2.0f);
		win->rect.y = (GetScreenHeight() / 2.0f) - (win->rect.h / 2.0f);

		if (mu_header_ex(ctx, "Player Color", MU_OPT_EXPANDED)) {
			mu_layout_height(ctx, ctx->style->padding);
			mu_layout_next(ctx);

			mu_layout_row(ctx, 2, (int[]) { -78, -1 }, 74);

			mu_layout_begin_column(ctx);
			mu_layout_row(ctx, 2, (int[]) { 80, -1 }, 74 / 3);
			mu_label(ctx, "Red:");   mu_slider_ex(ctx, &tempSettings.player_color[0], 0, 255, 1, "%.0f", MU_OPT_ALIGNCENTER);
			mu_label(ctx, "Green:"); mu_slider_ex(ctx, &tempSettings.player_color[1], 0, 255, 1, "%.0f", MU_OPT_ALIGNCENTER);
			mu_label(ctx, "Blue:");  mu_slider_ex(ctx, &tempSettings.player_color[2], 0, 255, 1, "%.0f", MU_OPT_ALIGNCENTER);
			mu_layout_end_column(ctx);

			mu_Rect r = mu_layout_next(ctx);
			mu_draw_rect(ctx, r, mu_color(tempSettings.player_color[0], tempSettings.player_color[1], tempSettings.player_color[2], 255));
		}

		mu_layout_height(ctx, ctx->style->padding);
		mu_layout_next(ctx);

		if (mu_header_ex(ctx, "Settings", MU_OPT_EXPANDED)) {
			mu_layout_height(ctx, ctx->style->padding);
			mu_layout_next(ctx);

			mu_layout_row(ctx, 2, (int[2]) { -32, -1 }, 30);

			mu_label(ctx, "VSync");
			mu_checkbox(ctx, "", &tempSettings.vsync);

			mu_label(ctx, "Always Show FPS");
			mu_checkbox(ctx, "", &tempSettings.drawfps);

			mu_layout_row(ctx, 2, (int[2]) { -272, -1 }, 30);
			mu_label(ctx, "Chunk view size");
			mu_layout_begin_column(ctx);
			mu_layout_row(ctx, 2, (int[]) { 80, -1 }, 0);
			mu_label(ctx, "Width:");  mu_slider_ex(ctx, &tempSettings.chunk_view_width, 1, GAME_SETTINGS_MAX_CHUNK_VIEW, 1.0f, "%.0f", 0);
			mu_label(ctx, "Height:"); mu_slider_ex(ctx, &tempSettings.chunk_view_height, 1, GAME_SETTINGS_MAX_CHUNK_VIEW, 1.0f, "%.0f", 0);
			mu_layout_end_column(ctx);

			mu_label(ctx, "Wall Brightness");
			mu_slider_ex(ctx, &tempSettings.wall_brightness, 0, 255, 1, "%.0f", MU_OPT_ALIGNCENTER);

			mu_layout_row(ctx, 2, (int[2]) { -32, -1 }, 30);

			mu_label(ctx, "Smooth Lighting");
			mu_checkbox(ctx, "", &tempSettings.smooth_lighting);

			mu_label(ctx, "Wall Ambient Occlusion");
			mu_checkbox(ctx, "", &tempSettings.wall_ao);

			if (tempSettings.wall_ao) {
				mu_layout_row(ctx, 2, (int[2]) { -272, -1 }, 30);
				mu_label(ctx, "Wall AO Brightness");
				mu_slider_ex(ctx, &tempSettings.wall_ao_brightness, 0, 255, 1, "%.0f", MU_OPT_ALIGNCENTER);
			}
			else {
				mu_layout_next(ctx);
			}
		}

		mu_layout_height(ctx, ctx->style->padding);
		mu_layout_next(ctx);

		mu_layout_row(ctx, 0, NULL, 30);

		mu_layout_width(ctx, (win->rect.w / 3) - (ctx->style->padding / 2.0f));

		backPressed = mu_button(ctx, "Back");
		if (mu_button(ctx, "Default")) {
			tempSettings.player_color[0] = 255.0f;
			tempSettings.player_color[1] = 0.0f;
			tempSettings.player_color[2] = 0.0f;

			tempSettings.vsync = true;
			tempSettings.drawfps = false;
			tempSettings.chunk_view_width = 5;
			tempSettings.chunk_view_height = 3;
			tempSettings.wall_brightness = 128.0f;
			tempSettings.wall_ao_brightness = 64.0f;
			tempSettings.smooth_lighting = true;
			tempSettings.wall_ao = true;
		}
		if (mu_button(ctx, "Apply")) {
			game_settings_apply();
		}

		mu_end_window(ctx);
	}

	if (backPressed) {
		game_settings_to_temp();
		return true;
	}

	return false;
}

void game_settings_apply() {
	Player* player = game_get_player();
	if (player) {
		player->color.r = tempSettings.player_color[0];
		player->color.g = tempSettings.player_color[1];
		player->color.b = tempSettings.player_color[2];
	}

	if (tempSettings.vsync) {
		SetTargetFPS(GetMonitorRefreshRate(GetCurrentMonitor()));
	}
	else {
		SetTargetFPS(0);
	}

	if (settings.chunk_view_height != tempSettings.chunk_view_height || settings.chunk_view_width != tempSettings.chunk_view_width) {
		if (!game_is_demo_mode()) chunk_manager_set_view(tempSettings.chunk_view_width, tempSettings.chunk_view_height);
	}
	else if (settings.wall_ao != tempSettings.wall_ao || settings.smooth_lighting != tempSettings.smooth_lighting || settings.wall_ao_brightness != tempSettings.wall_ao_brightness || settings.wall_brightness != tempSettings.wall_brightness) {
		chunk_manager_update_lighting();
	}

	temp_to_game_settings();
	save_game_settings();
}

GameSettings* get_game_settings() {
	return &settings;
}