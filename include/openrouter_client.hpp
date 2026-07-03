#pragma once

#include "config/config.hpp"

#include <nlohmann/json.hpp>

nlohmann::json send_chat_completion(
    const Config& config,
    const nlohmann::json& messages,
    const nlohmann::json& tools
);
