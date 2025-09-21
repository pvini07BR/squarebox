#ifndef ENTITY_H
#define ENTITY_H

#include <raylib.h>

struct Entity;

typedef struct {
	Rectangle rect;
	Vector2 velocity;

	void* parent;
	void (*update)(struct Entity* entity, float deltaTime);
	void (*draw)(struct Entity* entity);
	void (*destroy)(struct Entity* entity);

	bool collides;
	bool gravity_affected;

	bool grounded;
	bool on_liquid;
	bool on_climbable;
} Entity;

void entity_update(Entity* entity, float deltaTime);
void entity_debug_draw(Entity* entity);

Vector2 entity_get_center(Entity* entity);

#endif