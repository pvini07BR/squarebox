#include "entity_list.h"
#include <stdlib.h>

static Entity* entities[MAX_ENTITY_COUNT];
static size_t entity_count = 0;

bool entity_list_add(Entity* entity) {
	if (entity_count < MAX_ENTITY_COUNT) {
		entities[entity_count++] = entity;
		return true;
	}
	return false;
}

bool entity_list_remove_at(size_t idx) {
	if (idx >= entity_count) return false;
	Entity* e = entities[idx];
	if (e->destroy) e->destroy(e);
	entities[idx] = entities[--entity_count];
	return true;
}

void entity_list_update(float deltaTime) {
	for (int i = 0; i < entity_count; i++) {
		Entity* e = entities[i];
		
		if (e->update) e->update(e, deltaTime);
		entity_update(e, deltaTime);
	}
}

void entity_list_draw() {
	for (int i = 0; i < entity_count; i++) {
		Entity* e = entities[i];

		if (e->draw) e->draw(e);
		//entity_debug_draw(e);
	}
}

void entity_list_free() {
	for (int i = 0; i < entity_count; i++) {
		Entity* e = entities[i];
		if (e && e->destroy) {
			e->destroy(e);
		}
	}
	entity_count = 0;
}