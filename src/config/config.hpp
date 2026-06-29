#pragma once

#include <string>

struct Config {
    std::string api_key;
    std::string base_url;
    std::string model;
};

std::string parse_prompt(int argc, char* argv[]);
Config load_config();
