#include "../include/agent.hpp"
#include "config/config.hpp"
#include "tools.hpp"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
        auto prompt = parse_prompt(argc, argv);
        auto config = load_config();
        auto tools = make_default_tool_registry();

        return run_agent(config, prompt, tools);
}
