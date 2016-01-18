#include <unistd.h>
#include <stdbool.h>
#include "utils.h"

bool
running_from_term()
{
    return isatty(0);
}
