#include "light_queue.h"

#include "stdlib.h"
#include <stdio.h>

LightQueue* lightQueue = NULL;

LightQueue* get_light_queue() {
    return lightQueue;
}

void light_queue_init(int capacity) {
    if (capacity <= 0) {
        fprintf(stderr, "light_queue_init: capacidade inválida (%d)\n", capacity);
        return;
    }

    if (lightQueue) {
        free(lightQueue->nodes);
        free(lightQueue);
    }

    lightQueue = (LightQueue*)malloc(sizeof(LightQueue));
    lightQueue->nodes = (LightNode*)malloc(sizeof(LightNode) * capacity);
    lightQueue->front = 0;
    lightQueue->rear = 0;
    lightQueue->size = 0;
    lightQueue->capacity = capacity;
}

void light_queue_free() {
    if (lightQueue) {
        free(lightQueue->nodes);
        free(lightQueue);
        lightQueue = NULL;
    }
}

void light_queue_push(Chunk* chunk, Vector2i localPos, uint8_t lightLevel) {
    if (lightQueue->size >= lightQueue->capacity) return;

    lightQueue->nodes[lightQueue->rear].chunk = chunk;
    lightQueue->nodes[lightQueue->rear].localPosition = localPos;
    lightQueue->nodes[lightQueue->rear].lightLevel = lightLevel;
    lightQueue->rear = (lightQueue->rear + 1) % lightQueue->capacity;
    lightQueue->size++;
}

bool light_queue_pop(LightNode* node) {
    if (lightQueue->size == 0) return false;

    *node = lightQueue->nodes[lightQueue->front];
    lightQueue->front = (lightQueue->front + 1) % lightQueue->capacity;
    lightQueue->size--;
    return true;
}