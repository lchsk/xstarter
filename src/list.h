#ifndef LIST_H
#define LIST_H

#include <stddef.h>

typedef struct {
    size_t size;
    size_t cap;
    void **data;
} List;

List *list_new(size_t initial_size);
void list_free(List *list);
void list_append(List *list, void *value);
void *list_get(List *list, size_t index);
size_t list_size(List *list);
void list_del(List *list, size_t index);
size_t list_in(List *list, void *value);

#endif /* LIST_H */