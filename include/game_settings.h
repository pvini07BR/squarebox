#ifndef GAME_SETTINGS_H
#define GAME_SETTINGS_H

#include <raylib.h>

#include <stdbool.h>
#include <stdint.h>

#include "thirdparty/microui.h"

#define GAME_SETTINGS_FILE_NAME "settings.bin"
#define GAME_SETTINGS_MAX_CHUNK_VIEW 16

typedef struct {
	Color player_color;
	uint8_t chunk_view_width;
	uint8_t chunk_view_height;
	uint8_t wall_brightness;
	uint8_t wall_ao_brightness;
	bool vsync;
	bool drawfps;
	bool smooth_lighting;
	bool wall_ao;
} GameSettings;

// Had to make a separate struct so it can communicate properly with microui
typedef struct {
	float wall_brightness;
	float wall_ao_brightness;
	float chunk_view_width;
	float chunk_view_height;
	float player_color[3];
	int vsync;
	int drawfps;
	int smooth_lighting;
	int wall_ao;
} TempGameSettings;

void game_settings_to_temp();
void temp_to_game_settings();

bool save_game_settings();
bool load_game_settings();

// Returns true if the back button has been pressed
bool game_settings_draw(mu_Context* ctx);
void game_settings_apply();
GameSettings* get_game_settings();

#endif