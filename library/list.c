#include <assert.h>
#include <stdlib.h>
#include "list.h"

const size_t RESIZE_FACTOR = 2;
size_t DEFAULT_LIST_SIZE = 10;

typedef struct list {
    void **data;
    size_t capacity;
    size_t size;
    free_func_t freer;
} list_t;

typedef void (*free_func_t)(void *);

list_t *list_init(size_t initial_size, free_func_t freer) {
    list_t *list = malloc(sizeof(list_t));
    assert(list != NULL);
    if (initial_size > DEFAULT_LIST_SIZE) {
        list->data = malloc(sizeof(void *) * initial_size);
        assert(list->data != NULL);
        list->capacity = initial_size;
    } else {
        list->data = malloc(sizeof(void *) * DEFAULT_LIST_SIZE);
        assert(list->data != NULL);
        list->capacity = DEFAULT_LIST_SIZE;
    }
    list->size = 0;
    list->freer = freer;
    return list;
}

void ensure_capacity(list_t *list) {
    if (list->size == list->capacity) {
        void **new_data = malloc(sizeof(void *) * (list->capacity * RESIZE_FACTOR));
        for (int i = 0; i < list->size; i++) {
            new_data[i] = list->data[i];
        }
        list->data = new_data;
        list->capacity *= 2; 
    }
}

void list_free(list_t *list) {
    for (size_t i = 0; i < list->size; i++) {
        list->freer(list->data[i]);
    }
    free(list->data);
    free(list);
}

size_t list_size(list_t *list) {
    return list->size;
}

void *list_get(list_t *list, size_t index) {
    assert(index >= 0 && index < list->size);
    return list->data[index];
}

void *list_remove(list_t *list, size_t index) {
    assert(index >= 0 && index < list->size);
    void *return_value = list->data[index];
    for (size_t i = index; i < list->size-1; i++) {
        list->data[i] = list->data[i+1];
    }
    list->size--;
    return return_value;
}

void list_add(list_t *list, void *value) {
    assert(value != NULL);
    ensure_capacity(list);
    assert(list->size < list->capacity);
    list->data[list->size] = value;
    list->size++;
}
