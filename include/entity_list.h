#ifndef ENTITY_LIST_H
#define ENTITY_LIST_H

#include <stddef.h>

#include "entity.h"

#define MAX_ENTITY_COUNT 1024

bool entity_list_add(Entity* entity);
bool entity_list_remove_at(size_t idx);
void entity_list_update(float deltaTime);
void entity_list_draw();
void entity_list_free();

#endif