#include "light_queue.h"

void light_queue_init(LightQueue* q) {
    q->front = 0;
    q->rear = -1;
    q->size = 0;
}

bool light_queue_is_empty(LightQueue* q) {
    return q->size == 0;
}

bool light_queue_is_full(LightQueue* q) {
    return q->size == QUEUE_CAPACITY;
}

void light_queue_add(LightQueue* q, Vector2u position, uint8_t light) {
    if (light_queue_is_full(q)) return;
    q->rear = (q->rear + 1) % QUEUE_CAPACITY;
    q->data[q->rear] = (LightNode){ position, light };
    q->size++;
}

LightNode light_queue_remove(LightQueue* q) {
    LightNode empty = { .position = { 255, 255 }, .light = 255 };
    if (light_queue_is_empty(q)) return empty;

    LightNode n = q->data[q->front];
    q->front = (q->front + 1) % QUEUE_CAPACITY;
    q->size--;
    return n;
}
