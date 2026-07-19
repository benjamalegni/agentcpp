#include "bash.h"

#include <array>
#include <cstdio>
#include <memory>
#include <string>

using json = nlohmann::json;

std::string_view BashTool::name() const noexcept {
    return "Bash";
}

std::string_view BashTool::description() const noexcept {
    return "Execute a shell command";
}

json BashTool::parameters() const {
    return {
        {"type", "object"},
        {"properties",
         {{"command",
           {{"type", "string"}, {"description", "The command to execute"}}}}},
        {"required", json::array({"command"})},
    };
}

std::string BashTool::execute(const json& arguments) const {
    if (!arguments.contains("command") || !arguments["command"].is_string()) {
        return "Error: missing command in bash request";
    }

    const std::string command_string = arguments["command"].get<std::string>();

    std::array<char, 4096> buffer;
    std::string stdout_result;
    std::string stderr_result;

    {
        std::unique_ptr<FILE, decltype(&pclose)> pipe(
            popen((command_string + " 2>&1").c_str(), "r"), pclose);

        if (!pipe) {
            return "Error: failed to execute command";
        }

        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            stdout_result += buffer.data();
        }
    }

    if (stdout_result.empty()) {
        return "[command completed with no output]";
    }

    return stdout_result;
}
