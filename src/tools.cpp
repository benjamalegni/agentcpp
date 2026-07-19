#include "tools.hpp"

#include "bash.h"
#include "read.h"
#include "write.h"

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

using json = nlohmann::json;

namespace {
json parse_tool_arguments(const json& arguments) {
    if (arguments.is_string()) {
        return json::parse(arguments.get<std::string>());
    }
    if (arguments.is_object()) {
        return arguments;
    }
    return json::object();
}

bool is_path_in_workspace(
    const std::string& file_path,
    const std::filesystem::path& workspace
) {
    auto abs_path = std::filesystem::absolute(std::filesystem::path(file_path));
    auto abs_ws = std::filesystem::absolute(workspace);
    auto rel = std::filesystem::relative(abs_path, abs_ws);
    return !rel.empty() && rel.string()[0] != '.';
}
}

void ToolRegistry::add(std::unique_ptr<Tool> tool) {
    tools_.push_back(std::move(tool));
}

json ToolRegistry::definitions() const {
    json defs = json::array();
    for (const auto& tool : tools_) {
        defs.push_back(tool->definition());
    }
    return defs;
}

bool ToolRegistry::requires_approval(
    std::string_view name,
    const json& arguments
) const {
    if (name == "Bash") return true;
    if (name == "Write") return true;
    return false;
}

const Tool* ToolRegistry::find_tool(std::string_view name) const {
    for (const auto& tool : tools_) {
        if (tool->name() == name) return tool.get();
    }
    return nullptr;
}

std::string ToolRegistry::execute(
    std::string_view name,
    const json& arguments,
    const ToolContext& context
) const {
    const Tool* tool = find_tool(name);
    if (!tool) {
        return "Error: unknown tool: " + std::string(name);
    }

    json parsed = parse_tool_arguments(arguments);

    if ((name == "Read" || name == "Write") && parsed.contains("file_path")) {
        if (!is_path_in_workspace(
                parsed["file_path"].get<std::string>(), context.workspace)) {
            return "Error: file path is outside the workspace";
        }
    }

    if (name == "Bash" && parsed.contains("command")) {
        const auto& cmd = parsed["command"].get<std::string>();
        if (cmd.empty()) {
            return "Error: empty command";
        }
    }

    try {
        return tool->execute(parsed);
    } catch (const json::exception& e) {
        return "Error: invalid tool arguments: " + std::string(e.what());
    } catch (const std::exception& e) {
        return "Error: " + std::string(e.what());
    }
}

ToolRegistry make_default_tool_registry() {
    ToolRegistry registry;
    registry.add(std::make_unique<ReadTool>());
    registry.add(std::make_unique<WriteTool>());
    registry.add(std::make_unique<BashTool>());
    return registry;
}
