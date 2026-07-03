#include "agent.hpp"

#include "openrouter_client.hpp"
#include "tools.hpp"

#include <iostream>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

namespace {
json get_first_message(const json& result) {
    if (!result.contains("choices") || !result["choices"].is_array() || result["choices"].empty()) {
        throw std::runtime_error("No choices in response");
    }

    const json& choice = result["choices"][0];
    if (!choice.contains("message") || !choice["message"].is_object()) {
        throw std::runtime_error("No message in response choice");
    }

    return choice["message"];
}
}

int run_agent(
    const Config& config,
    const std::string& prompt,
    const ToolRegistry& tools
) {
    json messages = json::array({{{"role", "user"}, {"content", prompt}}});
    json available_tools = tools.definitions();

    while (true) {
        json result = send_chat_completion(config, messages, available_tools);
        json message = get_first_message(result);

        messages.push_back(message);

        if (message.contains("tool_calls") && message["tool_calls"].is_array() &&
            !message["tool_calls"].empty()) {
            for (const auto& tool_call : message["tool_calls"]) {
                const auto& function = tool_call.at("function");
                std::string content = tools.execute(
                    function.at("name").get<std::string>(),
                    function.contains("arguments") ? function["arguments"] : json::object()
                );
                messages.push_back({
                    {"role", "tool"},
                    {"tool_call_id", tool_call.at("id").get<std::string>()},
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
