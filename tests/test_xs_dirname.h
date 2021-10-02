#include "../src/utils_string.h"
#include "../src/utils.h"

mu_test(xs_dirname_null)
{
    char *result = xs_dirname(NULL);

    mu_assert("", strcmp(result,  ".") == 0);

    free(result);

    return 0;
}

mu_test(xs_dirname_full_path)
{
    char *result = xs_dirname("/usr/lib");

    mu_assert(result, strcmp(result,  "/usr") == 0);

    free(result);

    return 0;
}

mu_test(xs_dirname_slash_not_found)
{
    char *result = xs_dirname("usr");

    mu_assert("", strcmp(result,  ".") == 0);

    free(result);

    return 0;
}

mu_test(xs_dirname_trailing_slash)
{
    char *result = xs_dirname("/usr/");

    mu_assert("", strcmp(result, "/") == 0);

    free(result);

    return 0;
}

mu_test(xs_dirname_slash_only)
{
    char *result = xs_dirname("/");

    mu_assert("", strcmp(result, "/") == 0);

    free(result);

    return 0;
}

mu_test(xs_dirname_double_slash)
{
    char *result = xs_dirname("//");

    mu_assert("", strcmp(result, "/") == 0);

    free(result);

    return 0;
}

mu_test(xs_dirname_empty)
{
    char *result = xs_dirname("");

    mu_assert("", strcmp(result, ".") == 0);

    free(result);

    return 0;
}
