#pragma once

#include "config/config.hpp"

#include <string>

class ToolRegistry;

int run_agent(
    const Config& config,
    const std::string& prompt,
    const ToolRegistry& tools
);
