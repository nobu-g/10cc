#include "9cc.h"

int INITIAL_VECTOR_SIZE = 32;

Vector *create_vector() {
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *) * INITIAL_VECTOR_SIZE);
    vec->capacity = INITIAL_VECTOR_SIZE;
    vec->len = 0;
    return vec;
}

void push(Vector *vec, void *elem) {
    if (vec->len == vec->capacity) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}
