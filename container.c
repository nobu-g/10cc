#include "10cc.h"

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

Map *create_map() {
    Map *map = malloc(sizeof(Map));
    map->keys = create_vector();
    map->vals = create_vector();
    map->len = 0;
    return map;
}

void add_elem_to_map(Map *map, char *key, void *val) {
    for (int i=0; i<map->len; i++) {
        if (strcmp(map->keys->data[i], key) == 0) {
            map->vals->data[i] = val;
            return;
        }
    }
    push(map->keys, key);
    push(map->vals, val);
    map->len++;
}

void *get_elem_from_map(Map *map, char *key) {
    for (int i=0; i<map->len; i++) {
        if ((strcmp(map->keys->data[i], key) == 0)) {
            return map->vals->data[i];
        }
    }
    return NULL;
}
