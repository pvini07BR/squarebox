#include "sign_editor.h"
#include "registries/texture_atlas.h"
#include "types.h"

#include <string.h>

#include <raylib.h>

static bool open = false;
static SignLines* lines = NULL;

static unsigned int cur_line = 0;
static unsigned int line_cursor = 0;

void sign_editor_open(SignLines* new_lines) {
	if (!new_lines) return;
	lines = new_lines;
	open = true;
	line_cursor = (int)strlen(lines->lines[cur_line]);
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
		line_cursor = (int)strlen(lines->lines[cur_line]);
	}
	if (IsKeyPressed(KEY_DOWN) && cur_line < (SIGN_LINE_COUNT - 1)) {
		cur_line++;
		line_cursor = (int)strlen(lines->lines[cur_line]);
	}

	int key;
	while ((key = GetCharPressed()) > 0) {
		if (key >= 32 && key <= 126 && line_cursor < SIGN_LINE_LENGTH - 1) {
			lines->lines[cur_line][line_cursor++] = (char)key;
			lines->lines[cur_line][line_cursor] = '\0';
		}
	}

	if ((IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) && line_cursor > 0) {
		lines->lines[cur_line][--line_cursor] = '\0';
	}

	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color) { 0, 0, 0, 128 });

	Vector2 center = { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };

	float signSize = 320.0f;

	float originY = center.y - (signSize / 4.2f);
	float posX = center.x;

	DrawTexturePro(
		texture_atlas_get(),
		texture_atlas_get_rect(ATLAS_SIGN, 1),
		(Rectangle) { center.x, center.y, signSize, signSize },
		(Vector2) { signSize / 2.0f, signSize / 2.0f },
		0.0f,
		WHITE
	);

	for (unsigned int i = 0; i < SIGN_LINE_COUNT; i++) {
		Vector2 textMeasure = MeasureTextEx(GetFontDefault(), lines->lines[i], fontSize, letterSpacing);

		DrawTextPro(
            GetFontDefault(),
			lines->lines[i],
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
				memcpy(tmp, lines->lines[i], caretLen);
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