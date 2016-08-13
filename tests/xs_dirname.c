#include <criterion/criterion.h>
#include "../src/utils_string.h"
#include "../src/utils.h"

Test(xs_dirname, null)
{
    char *result = xs_dirname(NULL);

    cr_assert_str_eq(result,  ".");

    free(result);
}

Test(xs_dirname, full_path)
{
    char *result = xs_dirname("/usr/lib");

    cr_assert_str_eq(result,  "/usr");

    free(result);
}

Test(xs_dirname, slash_not_found)
{
    char *result = xs_dirname("usr");

    cr_assert_str_eq(result,  ".");

    free(result);
}

Test(xs_dirname, trailing_slash)
{
    char *result = xs_dirname("/usr/");

    cr_assert_str_eq(result, "/");

    free(result);
}

Test(xs_dirname, slash_only)
{
    char *result = xs_dirname("/");

    cr_assert_str_eq(result, "/");

    free(result);
}

Test(xs_dirname, double_slash)
{
    char *result = xs_dirname("//");

    cr_assert_str_eq(result, "/");

    free(result);
}

Test(xs_dirname, empty)
{
    char *result = xs_dirname("");

    cr_assert_str_eq(result, ".");

    free(result);
}
