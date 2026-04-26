#include "core/simulation/SimVerbosity.h"

#include <cstdarg>
#include <cstdio>

bool g_sim_verbose = true;

void simLog(const char* format, ...)
{
    if (!g_sim_verbose) return;

    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);
}
