#pragma once

#include "tool.hpp"
#include "tool_context.hpp"

#include <nlohmann/json.hpp>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

class ToolRegistry {
public:
    void add(std::unique_ptr<Tool> tool);
    nlohmann::json definitions() const;
    bool requires_approval(
        std::string_view name,
        const nlohmann::json& arguments
    ) const;
    std::string execute(
        std::string_view name,
        const nlohmann::json& arguments,
        const ToolContext& context
    ) const;

private:
    const Tool* find_tool(std::string_view name) const;
    std::vector<std::unique_ptr<Tool>> tools_;
};

ToolRegistry make_default_tool_registry();
