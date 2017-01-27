#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include <string.h>
#include <stdio.h>

typedef struct str_array_t {
    char **data;
    char *base_str;
    int length;
} str_array_t;

str_array_t *str_array_new(char *input_str, char const *delimiters);
void str_array_strip(str_array_t *str_array);
void str_array_free(str_array_t *str_array);

/* Replace tilde with a home directory */
char *expand_tilde(char *str, const char *home);

/* Return a newly allocated dirname */
char *xs_dirname(char *str);

#endif
