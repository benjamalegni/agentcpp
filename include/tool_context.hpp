#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include <string_view>

struct ToolContext {
    std::filesystem::path workspace;
    size_t max_output_size = 1024 * 1024;
};
