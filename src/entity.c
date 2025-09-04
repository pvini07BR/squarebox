#include "entity.h"
#include "defines.h"
#include "chunk_manager.h"
#include "block_registry.h"
#include "block_colliders.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <raymath.h>
#include <rlgl.h>

#define TO_BLOCK_COORDS(value) ((int)floorf((float)value / (float)TILE_SIZE))

typedef struct {
	Rectangle rect;
	float t;
} RectPair;

int compare_rects(const void* a, const void* b) {
	RectPair* pair_a = (RectPair*)a;
	RectPair* pair_b = (RectPair*)b;

	if (pair_a->t < pair_b->t) return -1;
	if (pair_a->t > pair_b->t) return 1;
	return 0;
}

static bool ray_vs_rect(const Vector2 ray_origin, const Vector2 ray_dir, const Rectangle* target, Vector2* contact_point, Vector2* contact_normal, float* t_hit_near) {
	Vector2 rect_pos = (Vector2){
		.x = target->x,
		.y = target->y
	};
	Vector2 rect_size = (Vector2){
		.x = target->width,
		.y = target->height
	};

	*contact_normal = Vector2Zero();
	*contact_point = Vector2Zero();

	Vector2 invdir = Vector2Divide(Vector2One(), ray_dir);

	Vector2 t_near = Vector2Multiply(Vector2Subtract(rect_pos, ray_origin), invdir);
	Vector2 t_far = Vector2Multiply(Vector2Subtract(Vector2Add(rect_pos, rect_size), ray_origin), invdir);

	if (isnan(t_far.y) || isnan(t_far.x)) return false;
	if (isnan(t_near.y) || isnan(t_near.x)) return false;

	if (t_near.x > t_far.x) {
		float temp = t_near.x;
		t_near.x = t_far.x;
		t_far.x = temp;
	}
	if (t_near.y > t_far.y) {
		float temp = t_near.y;
		t_near.y = t_far.y;
		t_far.y = temp;
	}

	if (t_near.x > t_far.y || t_near.y > t_far.x) return false;

	*t_hit_near = fmaxf(t_near.x, t_near.y);

	float t_hit_far = fminf(t_far.x, t_far.y);

	if (t_hit_far < 0) return false;

	*contact_point = Vector2Scale(Vector2Add(ray_origin, ray_dir), *t_hit_near);

	if (t_near.x > t_near.y) {
		if (invdir.x < 0) *contact_normal = (Vector2){ 1.0f, 0.0f };
		else *contact_normal = (Vector2){ -1.0f, 0.0f };
	}
	else if (t_near.x < t_near.y) {
		if (invdir.y < 0) *contact_normal = (Vector2){ 0.0f, 1.0f };
		else *contact_normal = (Vector2){ 0.0f, -1.0f };
	}

	return true;
}

static bool entity_vs_rect(const Entity* entity, const Rectangle* staticRect, const float deltaTime, Vector2* contact_point, Vector2* contact_normal, float* contact_time) {
	if (!entity) return false;
	if (entity->velocity.x == 0 && entity->velocity.y == 0) return false;

	Vector2 entityPos = (Vector2){
		.x = entity->rect.x,
		.y = entity->rect.y
	};
	Vector2 entitySize = (Vector2){
		.x = entity->rect.width,
		.y = entity->rect.height
	};

	Rectangle expanded_target = (Rectangle){
		.x = staticRect->x - (entity->rect.width / 2.0f),
		.y = staticRect->y - (entity->rect.height / 2.0f),
		.width = staticRect->width + entity->rect.width,
		.height = staticRect->height + entity->rect.height,
	};

	if (ray_vs_rect(
		Vector2Add(entityPos, Vector2Scale(entitySize, 0.5f)),
		Vector2Scale(entity->velocity, deltaTime),
		&expanded_target, contact_point, contact_normal, contact_time)
	) {
		return (*contact_time >= 0.0f && *contact_time < 1.0f);
	}
	else {
		return false;
	}
}

static bool resolve_entity_vs_rect(Entity* entity, Rectangle* staticRect, const float deltaTime, Vector2* contact_normal_output) {
	Vector2 contact_point;
	float contact_time = 0.0f;
	if (entity_vs_rect(entity, staticRect, deltaTime, &contact_point, contact_normal_output, &contact_time)) {
		/*
		entity->contacts[0] = contact_normal.y > 0 ? staticRect : NULL;
		entity->contacts[1] = contact_normal.x < 0 ? staticRect : NULL;
		entity->contacts[2] = contact_normal.y < 0 ? staticRect : NULL;
		entity->contacts[3] = contact_normal.x > 0 ? staticRect : NULL;
		*/

		Vector2 thing = Vector2Scale((Vector2) { fabsf(entity->velocity.x), fabsf(entity->velocity.y) }, 1.0f - contact_time);
		Vector2 addValue = Vector2Multiply(*contact_normal_output, thing);
		entity->velocity = Vector2Add(entity->velocity, addValue);
		return true;
	}
	return false;
}

void entity_update(Entity* entity, float deltaTime) {
	if (!entity) return;

	if (entity->collides) {
		Vector2 nextPosition = (Vector2){
			.x = entity->rect.x + entity->velocity.x * deltaTime,
			.y = entity->rect.y + entity->velocity.y * deltaTime
		};

		Vector2 topLeft = (Vector2){
			.x = fminf(entity->rect.x, nextPosition.x),
			.y = fminf(entity->rect.y, nextPosition.y)
		};

		Vector2 bottomRight = (Vector2){
			.x = fmaxf(entity->rect.x, nextPosition.x) + entity->rect.width,
			.y = fmaxf(entity->rect.y, nextPosition.y) + entity->rect.height
		};

		size_t rect_count = 0;
		RectPair rects[CHUNK_AREA];

		for (int x = TO_BLOCK_COORDS(topLeft.x); x < TO_BLOCK_COORDS(bottomRight.x) + 1; x++) {
			for (int y = TO_BLOCK_COORDS(topLeft.y); y < TO_BLOCK_COORDS(bottomRight.y) + 1; y++) {
				BlockInstance block = chunk_manager_get_block((Vector2i) { x, y }, false);
				BlockRegistry* reg = br_get_block_registry(block.id);
				if (!reg) continue;
				if (!(reg->flags & BLOCK_FLAG_SOLID)) continue;

				BlockVariant variant = br_get_block_variant(block.id, block.state);
					
				Rectangle collider_rects[MAX_RECTS_PER_COLLIDER];
				size_t collider_count = 0;
				block_colliders_get_rects(variant.collider_idx, variant.rotation, &collider_count, collider_rects);

				for (int i = 0; i < collider_count; i++) {
					Rectangle rect = collider_rects[i];
					rect.x += x * TILE_SIZE;
					rect.y += y * TILE_SIZE;

					Vector2 cp, cn;
					float t = 0.0f;
					if (entity_vs_rect(entity, &rect, deltaTime, &cp, &cn, &t)) {
						rects[rect_count].rect = rect;
						rects[rect_count].t = t;
						rect_count++;
					}
				}
			}
		}

		qsort(rects, rect_count, sizeof(RectPair), compare_rects);

		entity->grounded = false;

		for (int i = 0; i < rect_count; i++) {
			Rectangle* rect = &rects[i];

			Vector2 contact_normal;
			if (resolve_entity_vs_rect(entity, rect, deltaTime, &contact_normal)) {
				if (contact_normal.y < 0.0f) {
					entity->grounded = true;
				}
				/*
				if (entity->grounded && contact_normal.x != 0.0f) {
					float diff = (entity->rect.y + entity->rect.height) - rect->y;
					if (diff < (entity->rect.height * 0.6f)) {
						entity->rect.y -= rect->height + 1;
					}
				}
				*/
			}
		}
	}

	entity->rect.x += entity->velocity.x * deltaTime;
	entity->rect.y += entity->velocity.y * deltaTime;
}

void entity_draw(Entity* entity) {
	if (!entity) return;

	rlPushMatrix();

	rlTranslatef(
		entity->rect.x,
		entity->rect.y,
		0.0f
	);

	DrawRectangleLines(
		0.0f,
		0.0f,
		entity->rect.width,
		entity->rect.height,
		(Color){ 255, 0, 0, 255 }
	);

	rlPopMatrix();
}
