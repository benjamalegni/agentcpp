#include "tools.hpp"

#include <fstream>
#include <sstream>
#include <string>

using json = nlohmann::json;

namespace {
std::string read_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file) {
        return "Error: unable to read file: " + file_path;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

json parse_tool_arguments(const json& arguments) {
    if (arguments.is_string()) {
        return json::parse(arguments.get<std::string>());
    }

    if (arguments.is_object()) {
        return arguments;
    }

    return json::object();
}
}

json get_tools() {
    return json::array({
        {
            {"type", "function"},
            {"function", {
                {"name", "Read"},
                {"description", "Read and return the contents of a file"},
                {"parameters", {
                    {"type", "object"},
                    {"properties", {
                        {"file_path", {
                            {"type", "string"},
                            {"description", "The path to the file to read"}
                        }}
                    }},
                    {"required", json::array({"file_path"})}
                }}
            }}
        }
    });
}

std::string execute_tool(const std::string& name, const json& arguments) {
    if (name != "Read") {
        return "Error: unknown tool: " + name;
    }

    try {
        json parsed_arguments = parse_tool_arguments(arguments);
        if (!parsed_arguments.contains("file_path") || !parsed_arguments["file_path"].is_string()) {
            return "Error: missing required argument: file_path";
        }

        return read_file(parsed_arguments["file_path"].get<std::string>());
    } catch (const json::exception& exception) {
        return "Error: invalid tool arguments: " + std::string(exception.what());
    }
}
