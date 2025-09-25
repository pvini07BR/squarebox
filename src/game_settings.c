#include "game_settings.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <raylib.h>

static GameSettings settings = {
	.chunk_view_width = 5,
	.chunk_view_height = 3,
	.wall_brightness = 128,
	.wall_ao_brightness = 128,
	.smooth_lighting = true,
	.wall_ao = true,
};

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

void load_game_settings() {
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

	return true;
}

GameSettings* get_game_settings() {
	return &settings;
}