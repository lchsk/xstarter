#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include <string.h>
#define _GNU_SOURCE //asks stdio.h to include asprintf
#include <stdio.h>

#define Sasprintf(buffer, ...)              \
{                                           \
    char *tmp_string = buffer;              \
    asprintf(&(buffer), __VA_ARGS__);       \
    free(tmp_string);                       \
}

typedef struct str_array_t {
    char **data;
    char *base_str;
    int length;
} str_array_t;

str_array_t* str_array_new(char* input_str, char const* delimiters);

void str_array_free(str_array_t* str_array);

#endif
