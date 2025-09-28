#include "game_settings.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <raylib.h>
#include <raymath.h>
#include "thirdparty/raygui.h"

#include "types.h"
#include "chunk_manager.h"

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

bool game_settings_draw() {
	// Making UI in raygui is a pain.
	bool backButtonPressed = false;
	const float spacing = 50.0f;
	const float line_vert_spacing = 40.0f;
	const float button_width = 100.0f;

	Label chunkViewSizeLabel = create_label("Chunk view size", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());
	Label wallBrightnessLabel = create_label("Wall brightness", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());
	Label smoothLightLabel = create_label("Smooth Lighting", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());
	Label wallAOLabel = create_label("Wall Ambient Occlusion", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());
	Label wallAOBrightnessLabel = create_label("Wall Ambient Occlusion Brightness", GuiGetStyle(DEFAULT, TEXT_SIZE), GuiGetStyle(DEFAULT, TEXT_SPACING), GetFontDefault());
	
	float sliderWidth = 100.0f + MeasureTextEx(GetFontDefault(), "x", GuiGetStyle(DEFAULT, TEXT_SIZE), 0.0f).x;
	float totalWidth = wallAOBrightnessLabel.bounds.x + spacing + sliderWidth;
	float totalHeight = (line_vert_spacing * 5.0f) + (line_vert_spacing + 20.0f) + 40.0f;

	float centerX = (GetScreenWidth() / 2.0f) - (totalWidth / 2.0f);
	float centerY = (GetScreenHeight() / 2.0f) - (totalHeight / 2.0f);

	float originX = centerX;
	float originY = centerY;

	float biggest = wallAOBrightnessLabel.bounds.x;

	GuiLabel((Rectangle) { originX, originY, chunkViewSizeLabel.bounds.x, chunkViewSizeLabel.bounds.y }, chunkViewSizeLabel.str);
	originX += biggest + spacing;
	if (GuiValueBox((Rectangle) { originX, originY, 50, 30 }, NULL, &tempSettings.chunk_view_width, 1, 20, editing_chunk_view_width)) editing_chunk_view_width = !editing_chunk_view_width;
	if (GuiValueBox((Rectangle) { originX += 65, originY, 50, 30 }, "x", &tempSettings.chunk_view_height, 1, 20, editing_chunk_view_height)) editing_chunk_view_height = !editing_chunk_view_height;
	originX = centerX; originY += line_vert_spacing;
	GuiLabel((Rectangle) { originX, originY, wallBrightnessLabel.bounds.x, wallBrightnessLabel.bounds.y }, wallBrightnessLabel.str);
	originX += biggest + spacing;
	GuiSlider((Rectangle) { originX, originY, sliderWidth, 30 }, "0", "255", &tempSettings.wall_brightness, 0.0f, 255.0f);
	originX = centerX; originY += line_vert_spacing;
	GuiLabel((Rectangle) { originX, originY, smoothLightLabel.bounds.x, smoothLightLabel.bounds.y }, smoothLightLabel.str);
	originX += biggest + spacing;
	GuiCheckBox((Rectangle) { originX, originY, 30, 30 }, NULL, &tempSettings.smooth_lighting);
	originX = centerX; originY += line_vert_spacing;
	GuiLabel((Rectangle) { originX, originY, wallAOLabel.bounds.x, wallAOLabel.bounds.y }, wallAOLabel.str);
	originX += biggest + spacing;
	GuiCheckBox((Rectangle) { originX, originY, 30, 30 }, NULL, &tempSettings.wall_ao);
	if (tempSettings.wall_ao) {
		originX = centerX; originY += line_vert_spacing;
		GuiLabel((Rectangle) { originX, originY, wallAOBrightnessLabel.bounds.x, wallAOBrightnessLabel.bounds.y }, wallAOBrightnessLabel.str);
		originX += biggest + spacing;
		GuiSlider((Rectangle) { originX, originY, sliderWidth, 30 }, "0", "255", &tempSettings.wall_ao_brightness, 0.0f, 255.0f);
	}

	originX = centerX + (totalWidth / 2.0f) - ((button_width * 3.0f + spacing * 2.0f) / 2.0f);
	originY += (line_vert_spacing + 20.0f);

	backButtonPressed = GuiButton((Rectangle) { originX, originY, button_width, 40 }, "Back");
	originX += button_width + spacing;
	if (GuiButton((Rectangle) { originX, originY, button_width, 40 }, "Default")) {
		tempSettings.chunk_view_width = 5;
		tempSettings.chunk_view_height = 3;
		tempSettings.wall_brightness = 128.0f;
		tempSettings.wall_ao_brightness = 64.0f;
		tempSettings.smooth_lighting = true;
		tempSettings.wall_ao = true;
	}
	originX += button_width + spacing;
	if (GuiButton((Rectangle) { originX, originY, button_width, 40 }, "Apply")) {
		game_settings_apply();
	}

	if (backButtonPressed) {
		game_settings_to_temp();
	}
	return backButtonPressed;
	return false;
}

void game_settings_apply() {
	if (settings.chunk_view_height != tempSettings.chunk_view_height || settings.chunk_view_width != tempSettings.chunk_view_width) {
		temp_to_game_settings();
		chunk_manager_set_view(tempSettings.chunk_view_width, tempSettings.chunk_view_height);
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