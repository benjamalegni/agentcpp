#pragma once

#include "../src/config/config.hpp"

#include <string>

class ToolRegistry;

int run_agent(
    const Config& config,
    const std::string& prompt,
    const ToolRegistry& tools
);
