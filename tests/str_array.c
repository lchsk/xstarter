#include <criterion/criterion.h>
#include "../src/utils_string.h"
#include "../src/utils.h"

Test(str_array, empty)
{
    char *test = strdup("");
    str_array_t *result = str_array_new(test, ",");

    cr_assert_eq(result->length, 0);

    str_array_free(result);
}

Test(str_array, no_delimiter)
{
    char *test = strdup("abc def");
    str_array_t *result = str_array_new(test, ",");

    cr_assert_eq(result->length, 1);
    cr_assert_str_eq(result->data[0], "abc def");

    str_array_free(result);
}

Test(str_array, simple)
{
    char *test = strdup("a,b");
    str_array_t *result = str_array_new(test, ",");

    cr_assert_eq(result->length, 2);
    cr_assert_str_eq(result->data[0], "a");
    cr_assert_str_eq(result->data[1], "b");

    str_array_free(result);
}

Test(str_array, whitespace)
{
    char *test = strdup("abc def");
    str_array_t *result = str_array_new(test, " ");

    cr_assert_eq(result->length, 2);
    cr_assert_str_eq(result->data[0], "abc");
    cr_assert_str_eq(result->data[1], "def");

    str_array_free(result);
}

Test(str_array, newline)
{
    char *test = strdup("abc\ndef");
    str_array_t *result = str_array_new(test, "\n");

    cr_assert_eq(result->length, 2);
    cr_assert_str_eq(result->data[0], "abc");
    cr_assert_str_eq(result->data[1], "def");

    str_array_free(result);
}

Test(str_array, different_delimiters)
{
    char *test = strdup("a,b;c,d");
    str_array_t *result = str_array_new(test, ",;");

    cr_assert_eq(result->length, 4);
    cr_assert_str_eq(result->data[0], "a");
    cr_assert_str_eq(result->data[1], "b");
    cr_assert_str_eq(result->data[2], "c");
    cr_assert_str_eq(result->data[3], "d");

    str_array_free(result);
}

Test(str_array, several)
{
    char *test = strdup("a;b;c;d;");
    str_array_t *result = str_array_new(test, ";");

    cr_assert_eq(result->length, 4);
    cr_assert_str_eq(result->data[0], "a");
    cr_assert_str_eq(result->data[1], "b");
    cr_assert_str_eq(result->data[2], "c");
    cr_assert_str_eq(result->data[3], "d");

    str_array_free(result);
}

Test(str_array, strip)
{
    char *test = strdup("a: b: c: ");
    str_array_t *result = str_array_new(test, ":");

    str_array_strip(result);

    cr_assert_eq(result->length, 4);
    cr_assert_str_eq(result->data[0], "a");
    cr_assert_str_eq(result->data[1], "b");
    cr_assert_str_eq(result->data[2], "c");
    cr_assert_str_eq(result->data[3], "");

    str_array_free(result);
}
