#pragma once

#include "config/AppConfig.h"
#include <string>

namespace ConfigIO
{
    // Returns true on success. On failure, config is left unchanged.
    bool load(const std::string& path, AppConfig& config);

    // Returns true on success.
    bool save(const std::string& path, const AppConfig& config);
}
