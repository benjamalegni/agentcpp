#include "agent.hpp"
#include "config/config.hpp"

#include <exception>
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        auto prompt = parse_prompt(argc, argv);
        auto config = load_config();

        return run_agent(config, prompt);
    } catch (const std::exception& exception) {
        std::cerr << exception.what() << std::endl;
        return 1;
    }
}
