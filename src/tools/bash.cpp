//
// Created by luka on 7/19/26.
//
#import "bash.h"

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
            {"properties", {
                {"command", {
                    {"type", "string"},
                    {"description", "The command to execute"},
                }},
            }},
            {"required", json::array({"command"})},
        };
}

std::string BashTool::execute(const json& arguments) const {
    if (!arguments.contains("command") || !arguments["command"].is_string()) {
        return "Error: missing command in bash request or invalid request";
    }

    const std::string command_string = arguments["command"].get<std::string>();
    const char * command = command_string.c_str();

    // bash command execution with pipe

    std::array<char, 128> buffer;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);

    if (!pipe) {
        return "Error opening pipe for bash command execution";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}
