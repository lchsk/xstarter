#include "../src/utils_string.h"

mu_test(test_str_copy_smaller_than_max_str_length) {
    char dest[5];
    char src[5];

    memcpy(dest, "", sizeof(dest));
    memcpy(src, "", sizeof(src));

    str_copy(src, "test", sizeof(src));
    str_copy(dest, src, sizeof(dest));

    mu_assert("not equal", strcmp(dest, "test") == 0);
    mu_assert("not valid", src[4] == '\0');
    mu_assert("not valid", dest[4] == '\0');

    return 0;
}

mu_test(test_str_copy_equal_to_max) {
    char dest[4];
    char src[4];

    memcpy(dest, "", sizeof(dest));
    memcpy(src, "", sizeof(src));

    str_copy(src, "test", sizeof(src));
    str_copy(dest, src, sizeof(dest));

    mu_assert("", strcmp(src, "tes") == 0);
    mu_assert("", strcmp(dest, "tes") == 0);

    return 0;
}
