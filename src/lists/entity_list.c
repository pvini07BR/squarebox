#include "lists/entity_list.h"
#include <stdint.h>
#include <stdio.h>

static Entity* entities[MAX_ENTITY_COUNT];
static size_t entity_count = 0;

bool entity_list_add(Entity* entity) {
	if (entity_count < MAX_ENTITY_COUNT) {
		entity->to_remove = false;
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
	for (size_t i = 0; i < entity_count; i++) {
		Entity* e = entities[i];

		if (e->update) e->update(e, deltaTime);
		entity_update(e, deltaTime);
	}

	// Remove entities that has been marked to be removed
	size_t i = 0;
    while (i < entity_count) {
        Entity* e = entities[i];
        if (e->to_remove) {
            Entity* last = entities[--entity_count];
            if (i != entity_count) {
                entities[i] = last;
            } else {
                entities[i] = NULL;
            }

            if (e->destroy) e->destroy(e);
        } else {
            i++;
        }
    }
}

void entity_list_draw(bool draw_bounds) {
	for (size_t i = 0; i < entity_count; i++) {
		Entity* e = entities[i];

		if (e->draw) e->draw(e);
		if (draw_bounds) entity_debug_draw(e);
	}
}

void entity_list_remove_all() {
	for (size_t i = 0; i < entity_count; i++) {
		entities[i]->to_remove = true;
	}
}

void entity_list_clear() {
	for (size_t i = 0; i < entity_count; i++) {
		Entity* e = entities[i];
		if (e && e->destroy) {
			e->destroy(e);
		}
	}
	entity_count = 0;
}