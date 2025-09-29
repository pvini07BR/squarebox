#ifndef SIGN_EDITOR_H
#define SIGN_EDITOR_H

#include <stdbool.h>

#define SIGN_LINE_COUNT 3
#define SIGN_LINE_LENGTH 16

typedef struct {
	char lines[SIGN_LINE_COUNT][SIGN_LINE_LENGTH];
} SignLines;

void sign_editor_open(SignLines* lines);
void sign_editor_close();
void sign_editor_draw();
bool sign_editor_is_open();

#endif