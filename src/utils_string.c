#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils_string.h"

str_array_t*
str_array_new(char* input_str, char const* delimiters)
{
    str_array_t* out = malloc(sizeof(str_array_t));

    *out = (str_array_t) {
        .base_str = input_str
    };

    char* txt = strtok(input_str, delimiters);

    if (! txt) {
        return NULL;
    }

    while (txt) {
        out->data = realloc(out->data, sizeof(char*) *  ++(out->length));
        out->data[out->length - 1] = txt;
        txt = strtok(NULL, delimiters);
    }

    return out;
}

void
str_array_free(str_array_t* str_array)
{
    if (str_array == NULL) return;

    free(str_array->base_str);
    free(str_array->data);
    free(str_array);
}
