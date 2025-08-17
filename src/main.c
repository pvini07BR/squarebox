#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#include <stdio.h>

#include "chunk_manager.h"

bool placingWall = false;

int main() {
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(1280, 720, "mijocraft");

	chunk_manager_init();

	Camera2D camera = {
		.target =  { (CHUNK_WIDTH*TILE_SIZE)/2.0f, (CHUNK_WIDTH*TILE_SIZE)/2.0f },
		.offset =  { GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f },
		.rotation = 0.0f,
		.zoom = 1.0f
	};

	Vector2i currentChunkPos = { 0, 0 };

	char buffer[1024];

	while (!WindowShouldClose()) {
		camera.offset = (Vector2){
			.x = GetScreenWidth() / 2.0f, 
			.y = GetScreenHeight() / 2.0f
		};

		if (IsKeyPressed(KEY_TAB)) {
			placingWall = !placingWall;
		}

		if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
			Vector2 mousePos = GetScreenToWorld2D(GetMousePosition(), camera);

			Vector2i blockPos = {
				(int)floorf((float)mousePos.x / (float)TILE_SIZE),
				(int)floorf((float)mousePos.y / (float)TILE_SIZE)
			};
			
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				chunk_manager_set_block(blockPos, 0, placingWall);
			}
			else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
				chunk_manager_set_block(blockPos, 1, placingWall);
			}
		}

		Vector2 input = { 0 };

		if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) {
			input.x = -1.0f;
		}
		else if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) {
			input.x = 1.0f;
		}

		if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) {
			input.y = -1.0f;
		}
		else if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) {
			input.y = 1.0f;
		}

		if (IsKeyPressed(KEY_EQUAL)) {
			camera.zoom *= 1.1f;
		}
		else if (IsKeyPressed(KEY_MINUS)) {
			camera.zoom /= 1.1f;
		}

		input = Vector2Normalize(input);

		float speed = 500.0;
		if (IsKeyDown(KEY_LEFT_SHIFT)) {
			speed *= 4.0;
		}

		camera.target.x += input.x * speed * GetFrameTime();
		camera.target.y += input.y * speed * GetFrameTime();

		Vector2i cameraChunkPos = {
			(int)floorf(camera.target.x / (CHUNK_WIDTH * TILE_SIZE)),
			(int)floorf(camera.target.y / (CHUNK_WIDTH * TILE_SIZE))
		};

		if (cameraChunkPos.x != currentChunkPos.x || cameraChunkPos.y != currentChunkPos.y) {
			chunk_manager_relocate(cameraChunkPos);
			currentChunkPos = cameraChunkPos;
		}

		BeginDrawing();
		ClearBackground((Color){ 122, 122, 170 });
		BeginMode2D(camera);
		
		chunk_manager_draw();

		EndMode2D();

		sprintf(buffer,
			"FPS: %d\nChunk position: (%d, %d)\nPlacing Wall: %s",
			GetFPS(),
			cameraChunkPos.x,
			cameraChunkPos.y,
			placingWall ? "true" : "false"
		);
		DrawText(buffer, 0, 0, 24, WHITE);

		EndDrawing();
	}
	
	chunk_manager_free();
	CloseWindow();
	return 0;
}