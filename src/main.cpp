#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
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
}

int main(int argc, char* argv[]) {
    if (argc < 3 || std::string(argv[1]) != "-p") {
        std::cerr << "Expected first argument to be '-p'" << std::endl;
        return 1;
    }

    std::string prompt = argv[2];

    if (prompt.empty()) {
        std::cerr << "Prompt must not be empty" << std::endl;
        return 1;
    }

    const char* api_key_env = std::getenv("OPENROUTER_API_KEY");
    const char* base_url_env = std::getenv("OPENROUTER_BASE_URL");

    std::string api_key = api_key_env ? api_key_env : "";
    std::string base_url = base_url_env ? base_url_env : "https://openrouter.ai/api/v1";

    if (api_key.empty()) {
        std::cerr << "OPENROUTER_API_KEY is not set" << std::endl;
        return 1;
    }

    json messages = json::array({{{"role", "user"}, {"content", prompt}}});
    json available_tools = get_tools();

    while (true) {
        json request_body = {
            {"model", "anthropic/claude-haiku-4.5"},
            {"messages", messages},
            {"tools", available_tools}
        };

        cpr::Response response = cpr::Post(
            cpr::Url{base_url + "/chat/completions"},
            cpr::Header{
                {"Authorization", "Bearer " + api_key},
                {"Content-Type", "application/json"}
            },
            cpr::Body{request_body.dump()}
        );

        if (response.status_code != 200) {
            std::cerr << "HTTP error: " << response.status_code << std::endl;
            return 1;
        }

        json result = json::parse(response.text);

        if (!result.contains("choices") || result["choices"].empty()) {
            std::cerr << "No choices in response" << std::endl;
            return 1;
        }

        json message = result["choices"][0]["message"];
        messages.push_back(message);

        if (message.contains("tool_calls") && message["tool_calls"].is_array() &&
            !message["tool_calls"].empty()) {
            for (const auto& tool_call : message["tool_calls"]) {
                const auto& function = tool_call["function"];
                std::string content = execute_tool(
                    function["name"].get<std::string>(),
                    function["arguments"]
                );
                messages.push_back({
                    {"role", "tool"},
                    {"tool_call_id", tool_call["id"].get<std::string>()},
                    {"content", content},
                });
            }

            continue;
        }

        if (message.contains("content") && !message["content"].is_null()) {
            std::cout << message["content"].get<std::string>();
        }

        break;
    }

    return 0;
}
