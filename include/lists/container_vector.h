#ifndef CONTAINER_VECTOR_H
#define CONTAINER_VECTOR_H

#include "item_container.h"
#include <stddef.h>

typedef struct {
    ItemContainer* data;
    size_t size;
} ContainerVector;

void container_vector_init(ContainerVector* vec);
// Returns the index of the added element
size_t container_vector_add(ContainerVector* vec, const char* name, uint8_t rows, uint8_t columns, bool immutable);
void container_vector_remove(ContainerVector* vec, size_t idx);
ItemContainer* container_vector_get(ContainerVector* vec, size_t idx);
void container_vector_free(ContainerVector* vec);

#endif