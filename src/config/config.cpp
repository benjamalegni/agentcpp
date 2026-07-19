#include "config.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

bool wants_tui(int argc, char* argv[]) {
    if (argc < 2) return true;
    std::string arg1 = argv[1];
    return arg1 != "-p";
}

std::string parse_prompt(int argc, char* argv[]) {
    if (argc < 3 || std::string(argv[1]) != "-p") {
        throw std::runtime_error("Usage: claude-code -p \"<prompt>\"");
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
    const char* model_env = std::getenv("OPENROUTER_MODEL");

    std::string api_key = api_key_env ? api_key_env : "";
    if (api_key.empty()) {
        throw std::runtime_error(
            "OPENROUTER_API_KEY is not set\n"
            "Set it in your environment or use: export OPENROUTER_API_KEY=<key>");
    }

    return Config{
        api_key,
        base_url_env ? base_url_env : "https://openrouter.ai/api/v1",
        model_env ? model_env : "anthropic/claude-haiku-4.5",
    };
}

void print_usage() {
    std::cout << "claude-code - An AI coding assistant\n\n"
              << "Usage:\n"
              << "  claude-code -p \"<prompt>\"    Run in batch mode\n"
              << "  claude-code                   Start interactive TUI\n\n"
              << "Environment variables:\n"
              << "  OPENROUTER_API_KEY   API key (required)\n"
              << "  OPENROUTER_BASE_URL  API base URL (optional)\n"
              << "  OPENROUTER_MODEL     Model name (optional)\n";
}
