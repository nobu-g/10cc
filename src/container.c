#include "10cc.h"

int INITIAL_VECTOR_SIZE = 32;

Vector *vec_create() {
    Vector *vec = malloc(sizeof(Vector));
    vec->data = malloc(sizeof(void *) * INITIAL_VECTOR_SIZE);
    vec->capacity = INITIAL_VECTOR_SIZE;
    vec->len = 0;
    return vec;
}

void vec_push(Vector *vec, void *elem) {
    if (vec->len == vec->capacity) {
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void *) * vec->capacity);
    }
    vec->data[vec->len++] = elem;
}

void *vec_get(Vector *vec, int index) {
    if (index >= vec->len) {
        return NULL;
    }
    return vec->data[index];
}

void *vec_set(Vector *vec, int index, void *elem) {
    if (index >= vec->len) {
        return NULL;
    }
    vec->data[index] = elem;
    return elem;
}

Map *map_create() {
    Map *map = malloc(sizeof(Map));
    map->keys = vec_create();
    map->vals = vec_create();
    map->len = 0;
    return map;
}

void map_insert(Map *map, char *key, void *val) {
    for (int i = 0; i < map->len; i++) {
        if (strcmp(map->keys->data[i], key) == 0) {
            map->vals->data[i] = val;
            return;
        }
    }
    vec_push(map->keys, key);
    vec_push(map->vals, val);
    map->len++;
}

void *map_at(Map *map, char *key) {
    for (int i = 0; i < map->len; i++) {
        if ((strcmp(map->keys->data[i], key) == 0)) {
            return map->vals->data[i];
        }
    }
    return NULL;
}
