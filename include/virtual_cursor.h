#ifndef VIRTUAL_CURSOR_H
#define VIRTUAL_CURSOR_H

#include "raylib.h"

typedef enum {
    CURSOR_MODE_NORMAL,
    CURSOR_MODE_JOYSTICK
} CursorMode;

void load_cursor_tex();
void update_cursor();
void draw_cursor();

Vector2 get_cursor();
CursorMode get_cursor_mode();

bool cursor_pressed();
bool cursor_released();

#endif