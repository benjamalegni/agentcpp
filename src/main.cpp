#include "agent.hpp"
#include "config/config.hpp"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
        auto prompt = parse_prompt(argc, argv);
        auto config = load_config();

        return run_agent(config, prompt);
}
