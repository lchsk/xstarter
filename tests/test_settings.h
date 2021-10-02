#include <stddef.h>

#include "../src/settings.h"

mu_test(test_settings_default_file) {
    config_free(config_get());

    char* path = "../xstarter.conf";
    CmdLine cmdline = (CmdLine) {
        .config_path = path,
    };

    Config* conf = config_load(&cmdline);

    mu_assert("terminal", strcmp(conf->section_main->terminal, "xterm") == 0);
    mu_assert("dirs", strcmp(conf->section_main->dirs->base_str, "$PATH") == 0);
    mu_assert("emacs_bindings", conf->section_main->emacs_bindings == true);
    mu_assert("recent_apps_first", conf->section_main->recent_apps_first == true);
    mu_assert("numeric_shortcuts", conf->section_main->numeric_shortcuts == true);
    mu_assert("use_cache", conf->section_main->use_cache == true);
    mu_assert("auto_cache_refresh", conf->section_main->auto_cache_refresh == true);
    mu_assert("allow_spaces", conf->section_main->allow_spaces == true);
    mu_assert("min_query_len", conf->section_main->min_query_len == 1);

    mu_assert("selected color", strcmp(conf->section_colors->selected, "f44336") == 0);

    return 0;
}

mu_test(test_settings_no_file) {
    config_free(config_get());

    char* path = "../doesnt_exist";
    CmdLine cmdline = (CmdLine) {
        .config_path = path,
    };

    Config* conf = config_load(&cmdline);

    mu_assert("terminal", strcmp(conf->section_main->terminal, "xterm") == 0);
    mu_assert("dirs", strcmp(conf->section_main->dirs->base_str, "$PATH") == 0);
    mu_assert("emacs_bindings", conf->section_main->emacs_bindings == true);
    mu_assert("recent_apps_first", conf->section_main->recent_apps_first == true);
    mu_assert("numeric_shortcuts", conf->section_main->numeric_shortcuts == true);
    mu_assert("use_cache", conf->section_main->use_cache == true);
    mu_assert("auto_cache_refresh", conf->section_main->auto_cache_refresh == true);
    mu_assert("allow_spaces", conf->section_main->allow_spaces == true);
    mu_assert("min_query_len", conf->section_main->min_query_len == 1);

    mu_assert("selected color", strcmp(conf->section_colors->selected, "f44336") == 0);

    return 0;
}
