#include "tools.hpp"

#include "tools/read.h"
#include "tools/write.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>

using json = nlohmann::json;

// ADR / TDA in spanish of ToolRegistry, where it is possible 
// to do multiple operations with tools, such as adding new tools, getting the definitions of the tools and executing a tool by its name and arguments.

namespace {
json parse_tool_arguments(const json& arguments) {
    if (arguments.is_string()) {
        return json::parse(arguments.get<std::string>());
    }
    return json::object();
}
}

void ToolRegistry::add(std::unique_ptr<Tool> tool) {
    tools_.push_back(std::move(tool));
}

json ToolRegistry::definitions() const {
    json definitions = json::array();
    for (const auto& tool : tools_) {
        definitions.push_back(tool->definition());
    }
    return definitions;
}

std::string ToolRegistry::execute(std::string_view name, const json& arguments) const {
    const Tool* selected_tool = nullptr;
    for (const auto& tool : tools_) {
        if (tool->name() == name) {
            selected_tool = tool.get();
            break;
        }
    }

    if (selected_tool == nullptr) {
        return "Error: unknown tool: " + std::string(name);
    }

    try {
        return selected_tool->execute(parse_tool_arguments(arguments));
    } catch (const json::exception& exception) {
        return "Error: invalid tool arguments: " + std::string(exception.what());
    }
}

ToolRegistry make_default_tool_registry() {
    ToolRegistry registry;
    registry.add(std::make_unique<ReadTool>());
    registry.add(std::make_unique<WriteTool>());
    // add more later on
    return registry;
}
