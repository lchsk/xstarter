#include <assert.h>
#include <stdlib.h>

#include "list.h"

static void resize(List *list);

// How full the map must be in order to trigger resize
static const float LIST_RESIZE_TRIGGER = 0.6;

// By what factor we're enlarging the list
static const size_t LIST_RESIZE_FACTOR = 2;

size_t list_in(List *list, void *value)
{
    assert(list != NULL);

    for (int i = 0; i < list->size; i++) {
        if (list->data[i] == value) {
            return 1;
        }
    }

    return 0;
}

List *list_new(size_t initial_size)
{
    List *list = calloc(1, sizeof(List));

    list->data = calloc(initial_size, sizeof(void *));
    list->cap = initial_size;
    list->size = 0;

    return list;
}

void list_free(List *list)
{
    if (!list) {
        return;
    }

    if (list->data) {
        free(list->data);
    }

    free(list);
}

void list_append(List *list, void *value)
{
    assert(list != NULL);

    resize(list);

    list->data[list->size] = value;
    list->size++;
}

void *list_get(List *list, size_t index)
{
    if (index >= list->size) {
        return NULL;
    }

    return list->data[index];
}

size_t list_size(List *list)
{
    assert(list != NULL);

    return list->size;
}

void list_del(List *list, size_t index)
{
    assert(list != NULL);

    list->data[index] = NULL;

    for (int i = index; i < list->size - 1; i++) {
        list->data[i] = list->data[i + 1];
    }

    list->size--;
}

static void resize(List *list)
{
    assert(list != NULL);

    if (list->size < list->cap * LIST_RESIZE_TRIGGER) {
        return;
    }

    list->cap = list->cap * LIST_RESIZE_FACTOR;

    void **data = calloc(list->cap, sizeof(void *));

    for (int i = 0; i < list->cap / LIST_RESIZE_FACTOR; i++) {
        void *value = list->data[i];

        if (value) {
            data[i] = value;
        }
    }

    free(list->data);

    list->data = data;
}
