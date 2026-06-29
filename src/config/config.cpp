#include "config.hpp"

#include <cstdlib>
#include <stdexcept>
#include <string>

std::string parse_prompt(int argc, char* argv[]) {
    if (argc < 3 || std::string(argv[1]) != "-p") {
        throw std::runtime_error("Expected first argument to be '-p'");
    }

    std::string prompt = argv[2];
    if (prompt.empty()) {
        throw std::runtime_error("Prompt must not be empty");
    }

    return prompt;
}

Config load_config() {
    const char* api_key_env = std::getenv("OPENROUTER_API_KEY");
    const char* base_url_env = std::getenv("OPENROUTER_BASE_URL");

    std::string api_key = api_key_env ? api_key_env : "";
    if (api_key.empty()) {
        throw std::runtime_error("OPENROUTER_API_KEY is not set");
    }

    return Config{
        api_key,
        base_url_env ? base_url_env : "https://openrouter.ai/api/v1",
        "anthropic/claude-haiku-4.5",
    };
}
