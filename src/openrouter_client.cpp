#include "openrouter_client.hpp"

#include <cpr/cpr.h>

#include <stdexcept>
#include <string>

using json = nlohmann::json;

json send_chat_completion(
    const Config& config,
    const json& messages,
    const json& tools
) {
    json request_body = {
        {"model", config.model},
        {"messages", messages},
        {"tools", tools}
    };

    cpr::Response response = cpr::Post(
        cpr::Url{config.base_url + "/chat/completions"},
        cpr::Header{
            {"Authorization", "Bearer " + config.api_key},
            {"Content-Type", "application/json"}
        },
        cpr::Body{request_body.dump()}
    );

    if (response.status_code != 200) {
        throw std::runtime_error("HTTP error: " + std::to_string(response.status_code));
    }

    try {
        return json::parse(response.text);
    } catch (const json::exception& exception) {
        throw std::runtime_error("Invalid JSON response: " + std::string(exception.what()));
    }
}
