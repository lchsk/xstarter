#include <stddef.h>

#include "test.h"

#include "test_settings.h"
#include "test_strings.h"
#include "test_expand_tilde.h"
#include "test_str_array.h"
#include "test_xs_dirname.h"

int tests_run;

static char* all_tests() {
    // Settings
    mu_run_test(test_settings_default_file);
    mu_run_test(test_settings_no_file);

    // Strings
    mu_run_test(test_str_copy_smaller_than_max_str_length);
    mu_run_test(test_str_copy_equal_to_max);

    // Tilde
    mu_run_test(tilde_expansion_empty);
    mu_run_test(tilde_expansion_in_the_middle);
    mu_run_test(tilde_expansion_nothing_to_expand);
    mu_run_test(tilde_expansion_single_expansion);
    mu_run_test(tilde_expansion_two_expansions);
    mu_run_test(tilde_expansion_multiple_expansions);

    // StrArray
    mu_run_test(str_array_empty);
    mu_run_test(str_array_no_delimiter);
    mu_run_test(str_array_simple);
    mu_run_test(str_array_whitespace);
    mu_run_test(str_array_newline);
    mu_run_test(str_array_different_delimiters);
    mu_run_test(str_array_several);
    mu_run_test(str_array_strip_semicolons);

    // XS dirname
    mu_run_test(xs_dirname_null);
    mu_run_test(xs_dirname_full_path);
    mu_run_test(xs_dirname_slash_not_found);
    mu_run_test(xs_dirname_trailing_slash);
    mu_run_test(xs_dirname_slash_only);
    mu_run_test(xs_dirname_double_slash);
    mu_run_test(xs_dirname_empty);

    return 0;
}

int main() {
    char *result = all_tests();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
