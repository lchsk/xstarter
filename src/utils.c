#include <unistd.h>
#include "utils.h"

static char* _app_to_open_path;

void open_app()
{
    if (_app_to_open_path)
    popen(_app_to_open_path, "w");
}

void
app_to_open(char* path)
{
    _app_to_open_path = path;
}

int
running_from_term()
{
    return isatty(0);
}
