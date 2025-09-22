#include "lists/container_vector.h"
#include "item_container.h"
#include <stdlib.h>

void container_vector_init(ContainerVector* vec) {
    if (!vec) return;

    vec->data = NULL;
    vec->size = 0;
}

size_t container_vector_add(ContainerVector* vec, const char* name, uint8_t rows, uint8_t columns, bool immutable) {
    if (!vec) return -1;

    if (vec->data == NULL) {
        vec->data = malloc(sizeof(ItemContainer));
        if (vec->data) {
            item_container_create(&vec->data[vec->size], name, rows, columns, immutable);
            vec->size++;
            return vec->size;
        } else { return 0; }
    } else {
        vec->data = realloc(vec->data, (vec->size + 1) * sizeof(ItemContainer));
        if (vec->data) {
            vec->size++;
            item_container_create(&vec->data[vec->size - 1], name, rows, columns, immutable);
            return vec->size;
        } else { return 0; }
    }
}

void container_vector_remove(ContainerVector* vec, size_t idx) {
    if (!vec || idx >= vec->size) return;
    
    if (vec->data == NULL) return;
    
    item_container_free(&vec->data[idx]);
}

ItemContainer* container_vector_get(ContainerVector* vec, size_t idx) {
    if (idx < 0 || idx > (vec->size - 1)) return NULL;
    return &vec->data[idx];
}

void container_vector_free(ContainerVector* vec) {
    if (!vec) return;
    if (vec->data) {
        for (int i = 0; i < vec->size; i++) {
            item_container_free(&vec->data[i]);
        }
        free(vec->data);
        vec->data = NULL;
        vec->size = 0;
    }
}