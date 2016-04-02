#include <unistd.h>

#include <string.h>
#include "utils_string.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h> //abort

char app_path[1024 + 1];

str_array_t*
str_array_new(char* input_str, char const* delimiters)
{
    str_array_t* out = malloc(sizeof(str_array_t));
    *out = (str_array_t){
        .base_str = input_str
    };

    char* txt = strtok(input_str, delimiters);

    if ( ! txt) return NULL;

    while (txt)
    {
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

char*
get_application_path()
{
    // TODO:
    // readlink("/proc/curproc/file", buf, bufsize) (FreeBSD)
    // readlink("/proc/self/path/a.out", buf, bufsize) (Solaris)
    // argv[0]

    if (readlink("/proc/self/exe", app_path, sizeof(app_path) - 1) == -1) {
        printf("Can't find application path");
        return NULL;
    } else {
        return app_path;
    }
}
