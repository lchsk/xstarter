#include <criterion/criterion.h>
#include "../src/utils_string.h"
#include "../src/utils.h"

static const char *mock_home = "/xstarter";

Test(tilde_expansion, empty)
{
    char *result = expand_tilde("", NULL);

    cr_assert_str_eq(result, "");
}

Test(tilde_expansion, in_the_middle)
{
    char *result = expand_tilde(
        "xstarter, ~/xstarter, xstarter",
        mock_home
    );

    cr_assert_str_eq(
        result,
        "xstarter, /xstarter/xstarter, xstarter"
    );
}

Test(tilde_expansion, nothing_to_expand)
{
    char *result = expand_tilde("xstarter", mock_home);

    cr_assert_str_eq(result, "xstarter");
}

Test(tilde_expansion, single_expansion)
{
    char *result = expand_tilde("~/xstarter", mock_home);

    cr_assert_str_eq(result, "/xstarter/xstarter");
}

Test(tilde_expansion, two_expansions)
{
    char *result = expand_tilde(
        "~/xstarter, $PATH, ~/xstarter",
        mock_home
    );

    cr_assert_str_eq(
        result,
        "/xstarter/xstarter, $PATH, /xstarter/xstarter"
    );
}

Test(tilde_expansion, multiple_expansions)
{
    char *result = expand_tilde(
        "~/xs1, $PATH, ~/xs2, ~/xs3, xs",
        mock_home
    );

    cr_assert_str_eq(
        result,
        "/xstarter/xs1, $PATH, /xstarter/xs2, /xstarter/xs3, xs");
}
