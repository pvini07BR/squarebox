#include "game_settings.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <raylib.h>
#include <raymath.h>

#include "types.h"
#include "chunk_manager.h"
#include "game.h"

static GameSettings settings = {
	.chunk_view_width = 5,
	.chunk_view_height = 3,
	.wall_brightness = 128,
	.wall_ao_brightness = 64,
	.smooth_lighting = true,
	.wall_ao = true,
};

static TempGameSettings tempSettings;

static bool editing_chunk_view_width = false;
static bool editing_chunk_view_height = false;

void game_settings_to_temp() {
	tempSettings.chunk_view_width = settings.chunk_view_width;
	tempSettings.chunk_view_height = settings.chunk_view_height;
	tempSettings.wall_brightness = settings.wall_brightness;
	tempSettings.wall_ao_brightness = settings.wall_ao_brightness;
	tempSettings.smooth_lighting = settings.smooth_lighting;
	tempSettings.wall_ao = settings.wall_ao;
}

void temp_to_game_settings() {
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
			TraceLog(LOG_WARNING, "The config file config.bin was not found. The default settings values will be used.");
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

bool game_settings_draw(struct nk_context* ctx) {
	bool backPressed = false;
	const float label_width = 300.0f;
	const float element_width = 150.0f;
	const float padding = 10.0f;

	Vector2 size = { 620 + padding, 243 + padding };
	Vector2 screenCenter = {
		(GetScreenWidth() / 2.0f) - (size.x / 2.0f),
		(GetScreenHeight() / 2.0f) - (size.y / 2.0f),
	};

	nk_style_push_style_item(ctx, &ctx->style.window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 128)));
	nk_style_push_vec2(ctx, &ctx->style.window.padding, nk_vec2(padding, padding));

	if (nk_begin(ctx, "Settings", nk_rect(screenCenter.x, screenCenter.y, size.x, size.y), NK_WINDOW_NO_SCROLLBAR)) {
		nk_layout_row_begin(ctx, NK_STATIC, 30, 5);
		{
			nk_layout_row_push(ctx, label_width);
			nk_label(ctx, "Chunk view size", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, element_width);
			tempSettings.chunk_view_width = nk_propertyi(ctx, "Width:", 1, tempSettings.chunk_view_width, GAME_SETTINGS_MAX_CHUNK_VIEW, 1, 0.1f);
			tempSettings.chunk_view_height = nk_propertyi(ctx, "Height:", 1, tempSettings.chunk_view_height, GAME_SETTINGS_MAX_CHUNK_VIEW, 1, 0.1f);
			nk_spacing(ctx, 2);

			nk_layout_row_push(ctx, label_width);
			nk_label(ctx, "Wall Brightness", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, element_width * 2.0f);
			nk_slider_float(ctx, 0.0, &tempSettings.wall_brightness, 255.0f, 1.0f);
			nk_spacing(ctx, 3);

			nk_layout_row_push(ctx, label_width);
			nk_label(ctx, "Smooth Lighting", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, element_width * 2.0f);
			nk_checkbox_label(ctx, "", &tempSettings.smooth_lighting);
			nk_spacing(ctx, 3);

			nk_layout_row_push(ctx, label_width);
			nk_label(ctx, "Wall Ambient Occlusion", NK_TEXT_LEFT);
			nk_layout_row_push(ctx, element_width * 2.0f);
			nk_checkbox_label(ctx, "", &tempSettings.wall_ao);
			nk_spacing(ctx, 3);

			if (tempSettings.wall_ao) {
				nk_layout_row_push(ctx, label_width);
				nk_label(ctx, "Wall AO Brightness", NK_TEXT_ALIGN_LEFT);
				nk_layout_row_push(ctx, element_width * 2.0f);
				nk_slider_float(ctx, 0.0, &tempSettings.wall_ao_brightness, 255.0f, 1.0f);
				nk_spacing(ctx, 3);
			}
			else {
				nk_spacing(ctx, 5);
			}

			nk_spacing(ctx, 5);

			nk_layout_row_push(ctx, size.x / 5.0f);
			nk_spacing(ctx, 1);
			backPressed = nk_button_label(ctx, "Back");
			if (nk_button_label(ctx, "Default")) {
				tempSettings.chunk_view_width = 5;
				tempSettings.chunk_view_height = 3;
				tempSettings.wall_brightness = 128.0f;
				tempSettings.wall_ao_brightness = 64.0f;
				tempSettings.smooth_lighting = true;
				tempSettings.wall_ao = true;
			}
			if (nk_button_label(ctx, "Apply")) {
				game_settings_apply();
			}
			nk_spacing(ctx, 1);
		}
		nk_layout_row_end(ctx);
	}
	nk_end(ctx);

	nk_style_pop_style_item(ctx);
	nk_style_pop_vec2(ctx);

	if (backPressed) {
		game_settings_to_temp();
		return true;
	}

	return false;
}

void game_settings_apply() {
	if (settings.chunk_view_height != tempSettings.chunk_view_height || settings.chunk_view_width != tempSettings.chunk_view_width) {
		temp_to_game_settings();
		if (!game_is_demo_mode()) chunk_manager_set_view(tempSettings.chunk_view_width, tempSettings.chunk_view_height);
	}
	else if (settings.wall_ao != tempSettings.wall_ao || settings.smooth_lighting != tempSettings.smooth_lighting || settings.wall_ao_brightness != tempSettings.wall_ao_brightness || settings.wall_brightness != tempSettings.wall_brightness) {
		temp_to_game_settings();
		chunk_manager_update_lighting();
	}

	save_game_settings();
}

GameSettings* get_game_settings() {
	return &settings;
}