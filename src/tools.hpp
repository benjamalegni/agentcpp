#pragma once

#include "tools/tool.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class ToolRegistry {
public:
    void add(std::unique_ptr<Tool> tool);
    nlohmann::json definitions() const;
    std::string execute(
        std::string_view name,
        const nlohmann::json& arguments
    ) const;

private:
    std::vector<std::unique_ptr<Tool>> tools_;
};

ToolRegistry make_default_tool_registry();
