#include "virtual_cursor.h"
#include "raylib.h"
#include "raymath.h"
#include "types.h"

static Vector2 cursor_position = (Vector2) { 0.0f, 0.0f };
static CursorMode mode = CURSOR_MODE_NORMAL;
static Texture2D cursor_tex;

void load_cursor_tex() {
    cursor_tex = LoadTexture(ASSETS_PATH "cursor.png");
}

void update_cursor() {
    Vector2 dir = {
        GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X),
        GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y)
    };

    if (fabsf(dir.x) < GAMEPAD_STICK_DEADZONE) dir.x = 0.0f;
    if (fabsf(dir.y) < GAMEPAD_STICK_DEADZONE) dir.y = 0.0f;

    if ((dir.x != 0.0f || dir.y != 0.0f) && mode == CURSOR_MODE_NORMAL) {
        HideCursor();
        mode = CURSOR_MODE_JOYSTICK;
    }

    if ((fabsf(GetMouseDelta().x) > 0.0f || fabsf(GetMouseDelta().y) > 0.0f) && mode == CURSOR_MODE_JOYSTICK) {
        ShowCursor();
        mode = CURSOR_MODE_NORMAL;
    }

    if (mode == CURSOR_MODE_NORMAL) {
        cursor_position = GetMousePosition();
    } else {
        cursor_position = Vector2Add(cursor_position, Vector2Scale(dir, 500.0f * GetFrameTime()));
    }

    cursor_position = Vector2Clamp(cursor_position, Vector2Zero(), (Vector2) { GetScreenWidth(), GetScreenHeight() });
}

void draw_cursor() {
    if (mode == CURSOR_MODE_JOYSTICK) {
        DrawTexture(
            cursor_tex,
            cursor_position.x - (cursor_tex.width / 2.0f),
            cursor_position.y - (cursor_tex.height / 2.0f),
            WHITE
        );
    }
}

Vector2 get_cursor() {
    return cursor_position;
}

CursorMode get_cursor_mode() {
    return mode;
}

bool cursor_pressed() {
    if (mode == CURSOR_MODE_NORMAL) {
        return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    } else {
        return IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    }
}

bool cursor_released() {
    if (mode == CURSOR_MODE_NORMAL) {
        return IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
    } else {
        return IsGamepadButtonReleased(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    }
}