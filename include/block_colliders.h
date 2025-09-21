#ifndef BLOCK_COLLIDERS_H
#define BLOCK_COLLIDERS_H

#define MAX_RECTS_PER_COLLIDER 2

#include <stdint.h>
#include <stddef.h>

#include <raylib.h>

typedef enum {
	BLOCK_COLLIDER_QUAD,
	BLOCK_COLLIDER_SLAB,
	BLOCK_COLLIDER_STAIRS,
	BLOCK_COLLIDER_NUB,
	BLOCK_COLLIDER_TRAPDOOR,
	BLOCK_COLLIDER_COUNT
} BlockColliderEnum;

typedef struct {
	size_t collider_count;
	Rectangle colliders[MAX_RECTS_PER_COLLIDER];
} BlockCollider;

void block_colliders_init();
BlockCollider* block_colliders_get(BlockColliderEnum idx);
void block_colliders_get_rects(BlockColliderEnum idx, int rotation, size_t* rect_count, Rectangle output[MAX_RECTS_PER_COLLIDER]);

#endif