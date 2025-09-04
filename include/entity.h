#ifndef ENTITY_H
#define ENTITY_H

#include <raylib.h>

typedef struct {
	Rectangle rect;
	Vector2 velocity;
	Rectangle* contacts[4];

	bool collides;
	bool grounded;
} Entity;

void entity_update(Entity* entity, float deltaTime);
void entity_draw(Entity* entity);

#endif