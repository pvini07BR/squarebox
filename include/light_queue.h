#ifndef LIGHT_QUEUE_H
#define LIGHT_QUEUE_H

#include "chunk.h"

#define QUEUE_CAPACITY CHUNK_AREA

typedef struct {
    Vector2u position;
    uint8_t light;
} LightNode;

typedef struct {
    LightNode data[QUEUE_CAPACITY];
    int front;
    int rear;
    int size;
} LightQueue;

void light_queue_init(LightQueue* q);
bool light_queue_is_empty(LightQueue* q);
bool light_queue_is_full(LightQueue* q);
void light_queue_add(LightQueue* q, Vector2u position, uint8_t light);
LightNode light_queue_remove(LightQueue* q);

#endif