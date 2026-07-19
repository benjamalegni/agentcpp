#include "openrouter_provider.hpp"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>

using json = nlohmann::json;

OpenRouterProvider::OpenRouterProvider(Config config)
    : config_(std::move(config)) {}

CompletionResponse OpenRouterProvider::complete(const CompletionRequest& request) {
    json request_body = {
        {"model", request.model},
        {"messages", request.messages},
        {"tools", request.tools},
    };

    cpr::Response response = cpr::Post(
        cpr::Url{config_.base_url + "/chat/completions"},
        cpr::Header{
            {"Authorization", "Bearer " + config_.api_key},
            {"Content-Type", "application/json"},
        },
        cpr::Body{request_body.dump()},
        cpr::Timeout{60000}
    );

    if (response.error) {
        throw std::runtime_error(
            "Connection error: " + response.error.message);
    }

    if (response.status_code != 200) {
        std::string detail;
        try {
            detail = json::parse(response.text).dump(2);
        } catch (...) {
            detail = response.text.substr(0, 500);
        }
        throw std::runtime_error(
            "HTTP " + std::to_string(response.status_code) + ": " + detail);
    }

    try {
        CompletionResponse result;
        result.data = json::parse(response.text);
        return result;
    } catch (const json::exception& e) {
        throw std::runtime_error("Invalid JSON response: " + std::string(e.what()));
    }
}
