#include "write.h"

#include <fstream>
#include <string>

using json = nlohmann::json;

std::string_view WriteTool::name() const noexcept {
    return "Write";
}

std::string_view WriteTool::description() const noexcept {
    return "Write content to a file, creating or overwriting it";
}

json WriteTool::parameters() const {
    return {
        {"type", "object"},
        {"properties", {
            {"file_path", {
                {"type", "string"},
                {"description", "The path to the file to write"},
            }},
            {"content", {
                {"type", "string"},
                {"description", "The content to write to the file"},
            }},
        }},
        {"required", json::array({"file_path", "content"})},
    };
}

std::string WriteTool::execute(const json& arguments) const {
    if (!arguments.contains("file_path") || !arguments["file_path"].is_string()) {
        return "Error: missing required argument: file_path";
    }

    if (!arguments.contains("content") || !arguments["content"].is_string()) {
        return "Error: missing required argument: content";
    }

    const std::string file_path = arguments["file_path"].get<std::string>();
    const std::string content = arguments["content"].get<std::string>();

    std::ofstream file(file_path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return "Error: unable to write file: " + file_path;
    }

    file << content;
    if (!file) {
        return "Error: unable to write file: " + file_path;
    }

    return "Successfully wrote file: " + file_path;
}
