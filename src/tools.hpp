#pragma once

#include <nlohmann/json.hpp>

#include <string>

nlohmann::json get_tools();

std::string execute_tool(
    const std::string& name,
    const nlohmann::json& arguments
);
