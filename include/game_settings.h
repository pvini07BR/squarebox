#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include <stdbool.h>
#include <stdint.h>

#define GAME_SETTINGS_FILE_NAME "settings.bin"

typedef struct {
	uint8_t chunk_view_width;
	uint8_t chunk_view_height;
	uint8_t wall_brightness;
	uint8_t wall_ao_brightness;
	bool smooth_lighting;
	bool wall_ao;
} GameSettings;

bool save_game_settings();
bool load_game_settings();
GameSettings* get_game_settings();

#endif