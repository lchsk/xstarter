#include "../src/utils_string.h"
#include "../src/utils.h"

mu_test(str_array_empty)
{
    char *test = xs_strdup("");
    StrArray *result = str_array_new(test, ",");

    mu_assert("str_array_empty", result->length == 0);

    str_array_free(result);

    return 0;
}

mu_test(str_array_no_delimiter)
{
    char *test = xs_strdup("abc def");
    StrArray *result = str_array_new(test, ",");

    mu_assert("str_array_no_delimiter", result->length == 1);
    mu_assert("str_array_no_delimiter", strcmp(result->data[0], "abc def") == 0);

    str_array_free(result);

    return 0;
}

mu_test(str_array_simple)
{
    char *test = xs_strdup("a,b");
    StrArray *result = str_array_new(test, ",");

    mu_assert("str_array_simple", result->length == 2);
    mu_assert("str_array_simple", strcmp(result->data[0], "a") == 0);
    mu_assert("str_array_simple", strcmp(result->data[1], "b") == 0);

    str_array_free(result);

    return 0;
}

mu_test(str_array_whitespace)
{
    char *test = xs_strdup("abc def");
    StrArray *result = str_array_new(test, " ");

    mu_assert("", result->length == 2);
    mu_assert("", strcmp(result->data[0], "abc") == 0);
    mu_assert("", strcmp(result->data[1], "def") == 0);

    str_array_free(result);

    return 0;
}

mu_test(str_array_newline)
{
    char *test = xs_strdup("abc\ndef");
    StrArray *result = str_array_new(test, "\n");

    mu_assert("", result->length == 2);
    mu_assert("", strcmp(result->data[0], "abc") == 0);
    mu_assert("", strcmp(result->data[1], "def") == 0);

    str_array_free(result);

    return 0;
}

mu_test(str_array_different_delimiters)
{
    char *test = xs_strdup("a,b;c,d");
    StrArray *result = str_array_new(test, ",;");

    mu_assert("", result->length == 4);
    mu_assert("", strcmp(result->data[0], "a") == 0);
    mu_assert("", strcmp(result->data[1], "b") == 0);
    mu_assert("", strcmp(result->data[2], "c") == 0);
    mu_assert("", strcmp(result->data[3], "d") == 0);

    str_array_free(result);

    return 0;
}

mu_test(str_array_several)
{
    char *test = xs_strdup("a;b;c;d;");
    StrArray *result = str_array_new(test, ";");

    mu_assert("", result->length == 4);
    mu_assert("", strcmp(result->data[0], "a") == 0);
    mu_assert("", strcmp(result->data[1], "b") == 0);
    mu_assert("", strcmp(result->data[2], "c") == 0);
    mu_assert("", strcmp(result->data[3], "d") == 0);

    str_array_free(result);

    return 0;
}

mu_test(str_array_strip_semicolons)
{
    char *test = xs_strdup("a: b: c: ");
    StrArray *result = str_array_new(test, ":");

    str_array_strip(result);

    mu_assert("", result->length == 4);
    mu_assert("", strcmp(result->data[0], "a") == 0);
    mu_assert("", strcmp(result->data[1], "b") == 0);
    mu_assert("", strcmp(result->data[2], "c") == 0);
    mu_assert("", strcmp(result->data[3], "") == 0);

    str_array_free(result);

    return 0;
}
