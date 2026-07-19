#pragma once

#include <nlohmann/json.hpp>
#include <string>

struct CompletionRequest {
    std::string model;
    nlohmann::json messages;
    nlohmann::json tools;
};

struct CompletionResponse {
    nlohmann::json data;
};

class ChatProvider {
public:
    virtual ~ChatProvider() = default;
    virtual CompletionResponse complete(const CompletionRequest& request) = 0;
};
