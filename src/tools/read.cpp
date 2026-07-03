#include "read.h"

#include <fstream>
#include <sstream>
#include <string>

using json = nlohmann::json;

std::string_view ReadTool::name() const noexcept {
    return "Read";
}

std::string_view ReadTool::description() const noexcept {
    return "Read and return the contents of a file";
}

json ReadTool::parameters() const {
    return {
        {"type", "object"},
        {"properties", {
            {"file_path", {
                {"type", "string"},
                {"description", "The path to the file to read"},
            }},
        }},
        {"required", json::array({"file_path"})},
    };
}

std::string ReadTool::execute(const json& arguments) const {
    if (!arguments.contains("file_path") || !arguments["file_path"].is_string()) {
        return "Error: missing required argument: file_path";
    }

    const std::string file_path = arguments["file_path"].get<std::string>();
    std::ifstream file(file_path);
    if (!file) {
        return "Error: unable to read file: " + file_path;
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
