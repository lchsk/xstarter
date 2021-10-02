#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"
#include "utils_string.h"

StrArray *str_array_new(char *input_str, char const *delimiters)
{
    StrArray *out = smalloc(sizeof(StrArray));

    *out = (StrArray){.length = 0, .base_str = input_str};

    char *txt = strtok(input_str, delimiters);

    if (! txt) {
        return out;
    }

    while (txt) {
        out->data = realloc(out->data, sizeof(char *) * ++(out->length));
        out->data[out->length - 1] = txt;
        txt = strtok(NULL, delimiters);
    }

    return out;
}

void str_array_free(StrArray *str_array)
{
    if (str_array == NULL)
        return;

    free(str_array->base_str);
    free(str_array->data);
    free(str_array);
}

char *strip(char *str)
{
    char *end;

    // Trim leading space
    while (isspace(*str))
        str++;

    if (*str == 0) // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
        end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

void str_array_strip(StrArray *str_array)
{
    for (int i = 0; i < str_array->length; i++) {
        str_array->data[i] = strip(str_array->data[i]);
    }
}

char *expand_tilde(char *str, const char *home)
{
    if (str == NULL || home == NULL)
        return str;

    static char dest[MAX_LEN];
    memcpy(dest, "", MAX_LEN);

    char *from = str;

    for (int i = 0; str[i]; i++) {
        if (i >= (MAX_LEN - 1)) {
            set_err(ERR_DIRS_TOO_LONG);
            str_copy(dest, str, MAX_LEN - 1);

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

char *xs_dirname(char *str)
{
    char *result;

    /* Special cases */

    if (str == NULL || strcmp(str, ".") == 0 || strcmp(str, "..") == 0 ||
        strcmp(str, "") == 0) {
        result = smalloc(2);
        strcpy(result, ".");

        return result;
    }

    int i = strlen(str);

    if (str[i - 1] == '/') {
        result = smalloc(2);
        strcpy(result, "/");

        return result;
    }

    bool found = false;

    while (--i) {
        if (str[i] == '/') {
            found = true;
            break;
        }
    }

    if (found) {
        result = smalloc(i + 1);
        str_copy(result, str, i + 1);
        result[i] = '\0';
    } else {
        result = smalloc(2);
        strcpy(result, ".");
    }

    return result;
}

char *xs_strdup(const char *str)
{
    size_t len = 1 + strlen(str);
    char *ptr = smalloc(len);

    return ptr ? memcpy(ptr, str, len) : NULL;
}

void str_copy(char *dest, const char *src, size_t dest_size)
{
    const size_t size = strlen(src);

    if (dest_size > size) {
        memcpy(dest, src, size);
        dest[size] = '\0';
    } else {
        memcpy(dest, src, dest_size - 1);
        dest[dest_size - 1] = '\0';
    }
}
