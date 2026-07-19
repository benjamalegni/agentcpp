#pragma once

#include "chat_provider.hpp"
#include "config/config.hpp"

class OpenRouterProvider final : public ChatProvider {
public:
    explicit OpenRouterProvider(Config config);
    CompletionResponse complete(const CompletionRequest& request) override;

private:
    Config config_;
};
