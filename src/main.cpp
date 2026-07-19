#include "agent_session.hpp"
#include "chat_provider.hpp"
#include "config/config.hpp"
#include "openrouter_provider.hpp"
#include "tool_context.hpp"
#include "tools.hpp"
#include "tui/tui_app.hpp"

#include <filesystem>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

int run_batch(const Config& config, const std::string& prompt) {
    auto provider = std::make_shared<OpenRouterProvider>(config);
    auto tools = make_default_tool_registry();

    ToolContext ctx;
    ctx.workspace = std::filesystem::current_path();

    AgentSession session(*provider, tools, config, std::move(ctx));
    session.set_event_callback([]{});

    session.add_user_message(prompt);

    try {
        session.run();
    } catch (const std::exception& e) {
        std::cerr << "\nError: " << e.what() << std::endl;
        return 1;
    }

    AgentEvent event;
    while (session.poll_event(event)) {
        if (event.type == AgentEventType::AssistantMessage) {
            std::cout << event.content << std::flush;
        } else if (event.type == AgentEventType::Error) {
            std::cerr << "\nError: " << event.error_message << std::endl;
        }
    }

    return 0;
}

int main(int argc, char* argv[]) {
    try {
        auto config = load_config();

        if (wants_tui(argc, argv)) {
            auto provider = std::make_shared<OpenRouterProvider>(config);
            auto tools = make_default_tool_registry();

            ToolContext ctx;
            ctx.workspace = std::filesystem::current_path();

            AgentSession session(*provider, tools, config, std::move(ctx));
            return run_tui(session);
        }

        auto prompt = parse_prompt(argc, argv);
        return run_batch(config, prompt);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
