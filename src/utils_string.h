#ifndef XSTARTER_UTILS_STRING_H
#define XSTARTER_UTILS_STRING_H

#include <stdio.h>
#include <string.h>

typedef struct StrArray {
    char **data;
    char *base_str;
    int length;
} StrArray;

char *xs_strdup(const char *str);

StrArray *str_array_new(char *input_str, char const *delimiters);
void str_array_strip(StrArray *str_array);
void str_array_free(StrArray *str_array);

/* Replace tilde with a home directory */
char *expand_tilde(char *str, const char *home);

/* Return a newly allocated dirname */
char *xs_dirname(char *str);

/* Copy string from src to destination */
void str_copy(char *dest, const char *src, size_t dest_size);

#endif
