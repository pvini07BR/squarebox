#include "sign_editor.h"
#include "texture_atlas.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <raylib.h>
#include <thirdparty/raygui.h>

static bool open = false;
static char** lines = NULL;

static unsigned int cur_line = 0;
static unsigned int line_cursor = 0;

void sign_editor_open(char** new_lines) {
	if (!new_lines) return;
	lines = new_lines;
	open = true;
	line_cursor = (int)strlen(lines[cur_line]);
}

void sign_editor_close() {
	if (open) {
		open = false;
		lines = NULL;
		cur_line = 0;
		line_cursor = 0;
	}
}

void sign_editor_draw() {
	if (!open || !lines) return;

	const int fontSize = 34.0f;
	const float lineSpacing = fontSize / 1.5f;
	const float letterSpacing = fontSize / 8.0f;

	if (IsKeyPressed(KEY_UP) && cur_line > 0) {
		cur_line--;
		line_cursor = (int)strlen(lines[cur_line]);
	}
	if (IsKeyPressed(KEY_DOWN) && cur_line < (SIGN_LINE_COUNT - 1)) {
		cur_line++;
		line_cursor = (int)strlen(lines[cur_line]);
	}

	int key = GetCharPressed();
	if (key >= 32 && key <= 126) {
		if (line_cursor < SIGN_LINE_LENGTH - 1) {
			lines[cur_line][line_cursor++] = (char)key;
			lines[cur_line][line_cursor] = '\0';
		}
	}

	if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && line_cursor > 0) {
		lines[cur_line][--line_cursor] = '\0';
	}

	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) { 0, 0, 0, 128 });

	Vector2 center = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

	float signSize = 320.0f;

	float originY = center.y - (signSize / 4.2f);
	float posX = center.x;

	DrawTexturePro(
		texture_atlas_get(),
		texture_atlas_get_rect(41, false, false),
		(Rectangle) { center.x, center.y, signSize, signSize },
		(Vector2) { signSize / 2.0f, signSize / 2.0f },
		0.0f,
		WHITE
	);

	for (int i = 0; i < SIGN_LINE_COUNT; i++) {
		char* str = lines[i];
		Vector2 textMeasure = MeasureTextEx(GetFontDefault(), str, fontSize, letterSpacing);
		DrawTextPro(
            GetFontDefault(),
            str,
            (Vector2) { posX, originY },
            (Vector2) { textMeasure.x / 2.0f, 0.0f },
            0.0f,
            fontSize,
			letterSpacing,
            BLACK
        );

		if (i == cur_line) {
			char tmp[SIGN_LINE_LENGTH];
			int caretLen = line_cursor;
			if (caretLen > 0) {
				memcpy(tmp, str, caretLen);
			}
			tmp[caretLen] = '\0';
			Vector2 caretMeasure = MeasureTextEx(GetFontDefault(), tmp, (float)fontSize, letterSpacing);
			float caretX = posX + (caretMeasure.x / 2.0f);
			float caretY = originY;
			DrawRectangle((int)caretX, (int)caretY, 2, fontSize, BLACK);
		}

		originY += fontSize + lineSpacing;
	}
}

bool sign_editor_is_open() { return open; }