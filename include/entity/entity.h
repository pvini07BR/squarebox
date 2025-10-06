#ifndef ENTITY_H
#define ENTITY_H

#include <stddef.h>
#include <raylib.h>

typedef struct Entity Entity;

typedef struct Entity {
	Rectangle rect;
	Vector2 velocity;
	bool to_remove;

	void* parent;
	void (*update)(struct Entity* entity, float deltaTime);
	void (*draw)(struct Entity* entity);
	void (*destroy)(struct Entity* entity);

	bool collides;
	bool gravity_affected;

	bool grounded;
	bool on_slippery;

	bool on_liquid;
	bool on_climbable;
} Entity;

void entity_update(Entity* entity, float deltaTime);
void entity_debug_draw(Entity* entity);

Vector2 entity_get_center(Entity* entity);

#endif