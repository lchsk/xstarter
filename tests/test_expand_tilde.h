#include "../src/utils_string.h"
#include "../src/utils.h"

static const char *mock_home = "/xstarter";

mu_test(tilde_expansion_empty)
{
    char *result = expand_tilde("", NULL);

    mu_assert("not equal tilde_expansion_empty", strcmp(result, "") == 0);

    return 0;
}

mu_test(tilde_expansion_in_the_middle)
{
    char *result = expand_tilde(
        "xstarter, ~/xstarter, xstarter",
        mock_home
    );

    mu_assert("not equal tilde_expansion_in_the_middle", strcmp(result, "xstarter, /xstarter/xstarter, xstarter") == 0);

    return 0;
}

mu_test(tilde_expansion_nothing_to_expand)
{
    char *result = expand_tilde("xstarter", mock_home);

    mu_assert("tilde_expansion_nothing_to_expand", strcmp(result, "xstarter") == 0);

    return 0;
}

mu_test(tilde_expansion_single_expansion)
{
    char *result = expand_tilde("~/xstarter", mock_home);

    mu_assert("tilde_expansion_single_expansion", strcmp(result, "/xstarter/xstarter") == 0);

    return 0;
}

mu_test(tilde_expansion_two_expansions)
{
    char *result = expand_tilde(
        "~/xstarter, $PATH, ~/xstarter",
        mock_home
    );

    mu_assert("tilde_expansion_single_expansion", strcmp(result, "/xstarter/xstarter, $PATH, /xstarter/xstarter") == 0);

    return 0;
}

mu_test(tilde_expansion_multiple_expansions)
{
    char *result = expand_tilde(
        "~/xs1, $PATH, ~/xs2, ~/xs3, xs",
        mock_home
    );

    mu_assert("tilde_expansion_single_expansion", strcmp(result, "/xstarter/xs1, $PATH, /xstarter/xs2, /xstarter/xs3, xs") == 0);

    return 0;
}
