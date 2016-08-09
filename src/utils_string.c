#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils_string.h"
#include "utils.h"

#define MAX_LEN (4096)

str_array_t
*str_array_new(char *input_str, char const *delimiters)
{
    str_array_t *out = malloc(sizeof(str_array_t));

    *out = (str_array_t) {
        .base_str = input_str
    };

    char *txt = strtok(input_str, delimiters);

    if (! txt) {
        return NULL;
    }

    while (txt) {
        out->data = realloc(out->data, sizeof(char*) * ++(out->length));
        out->data[out->length - 1] = txt;
        txt = strtok(NULL, delimiters);
    }

    return out;
}

void
str_array_free(str_array_t *str_array)
{
    if (str_array == NULL) return;

    free(str_array->base_str);
    free(str_array->data);
    free(str_array);
}

char *strip(char *str)
{
    char *end;

    // Trim leading space
    while (isspace(*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

void
str_array_strip(str_array_t *str_array)
{
    for (int i = 0; i < str_array->length; i++) {
        str_array->data[i] = strip(str_array->data[i]);
    }
}

char*
expand_tilde(char *str, const char *home)
{
    if (str == NULL || home == NULL)
        return str;

    int len = strlen(home);

    static char dest[MAX_LEN];

    char *from = str;
    int i = 0;

    for (int i = 0; str[i]; i++) {
        if (i >= (MAX_LEN - 1)) {
            set_err(ERR_DIRS_TOO_LONG);
            strncpy(dest, str, MAX_LEN -1);
            return dest;
        }

        if (str[i] == '~') {
            strncat(dest, from, &str[i] - from);
            strcat(dest, home);

            from = str + i + 1;
        }
    }

    strcat(dest, from);

    return dest;
}
