#ifndef LIGHT_QUEUE_H
#define LIGHT_QUEUE_H

#include "chunk.h"

typedef struct {
    Chunk* chunk;
    Vector2i localPosition;
    uint8_t lightLevel;
} LightNode;

typedef struct {
    LightNode* nodes;
    int front;
    int rear;
    int size;
    int capacity;
} LightQueue;

LightQueue* get_light_queue();
void light_queue_init(int capacity);
void light_queue_free();
void light_queue_push(Chunk* chunk, Vector2i localPos, uint8_t lightLevel);
bool light_queue_pop(LightNode* node);

#endif