#include "block_colliders.h"
#include "types.h"

static BlockCollider colliders[BLOCK_COLLIDER_COUNT];

void block_colliders_init() {
	colliders[BLOCK_COLLIDER_QUAD] = (BlockCollider){
		.collider_count = 1,
		.colliders = { { .x = 0.0f, .y = 0.0f, .width = TILE_SIZE, .height = TILE_SIZE } }
	};

	colliders[BLOCK_COLLIDER_SLAB] = (BlockCollider){
		.collider_count = 1,
		.colliders = { {.x = 0.0f, .y = TILE_SIZE / 2.0f, .width = TILE_SIZE, .height = TILE_SIZE / 2.0f } }
	};

	colliders[BLOCK_COLLIDER_STAIRS] = (BlockCollider){
		.collider_count = 2,
		.colliders = {
			{ .x = TILE_SIZE / 2.0f, .y = 0.0f, .width = TILE_SIZE / 2.0f, .height = TILE_SIZE / 2.0f },
			{ .x = 0.0f, .y = TILE_SIZE / 2.0f, .width = TILE_SIZE, .height = TILE_SIZE / 2.0f },
		}
	};

	colliders[BLOCK_COLLIDER_NUB] = (BlockCollider){
		.collider_count = 1,
		.colliders = { {.x = TILE_SIZE / 2.0f, .y = TILE_SIZE / 2.0f, .width = TILE_SIZE / 2.0f, .height = TILE_SIZE / 2.0f } }
	};

    colliders[BLOCK_COLLIDER_TRAPDOOR] = (BlockCollider){
        .collider_count = 1,
        .colliders = { {.x = 0.0f, .y = TILE_SIZE * 0.875f, .width = TILE_SIZE, .height = TILE_SIZE * 0.125f } }
    };
}

BlockCollider* block_colliders_get(BlockColliderEnum idx) {
	if (idx < 0 || idx >= BLOCK_COLLIDER_COUNT) return NULL;
	return &colliders[idx];
}

void block_colliders_get_rects(BlockColliderEnum idx, int rotation, size_t* rect_count, Rectangle output[MAX_RECTS_PER_COLLIDER]) {
    if (idx < 0 || idx >= BLOCK_COLLIDER_COUNT) return;

    size_t count = colliders[idx].collider_count;
    if (count > MAX_RECTS_PER_COLLIDER) count = MAX_RECTS_PER_COLLIDER;
    *rect_count = count;

    int rot = rotation & 3;

    const float halfTile = TILE_SIZE * 0.5f;

    for (size_t i = 0; i < count; i++) {
        Rectangle r = colliders[idx].colliders[i];

        float rcx = r.x + r.width * 0.5f;
        float rcy = r.y + r.height * 0.5f;

        float tx = rcx - halfTile;
        float ty = rcy - halfTile;

        float nx, ny;
        float nw, nh;

        switch (rot) {
        case 0: // 0°
            nx = tx; ny = ty;
            nw = r.width; nh = r.height;
            break;
        case 1: // 90° CCW
            // (x, y) -> (-y, x)
            nx = -ty;
            ny = tx;
            nw = r.height;
            nh = r.width;
            break;
        case 2: // 180°
            nx = -tx;
            ny = -ty;
            nw = r.width;
            nh = r.height;
            break;
        case 3: // 270° CCW
            // (x, y) -> (y, -x)
            nx = ty;
            ny = -tx;
            nw = r.height;
            nh = r.width;
            break;
        default:
            nx = tx; ny = ty;
            nw = r.width; nh = r.height;
            break;
        }

        float final_cx = nx + halfTile;
        float final_cy = ny + halfTile;

        output[i].width = nw;
        output[i].height = nh;
        output[i].x = final_cx - nw * 0.5f;
        output[i].y = final_cy - nh * 0.5f;
    }
}