cmake_minimum_required(VERSION 3.16)
project(codecrafters-claude-code)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(cpr CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(ftxui CONFIG REQUIRED)

add_library(claude_code_core STATIC
    src/agent_session.cpp
    src/openrouter_provider.cpp
    src/tools.cpp
    src/tools/read.cpp
    src/tools/write.cpp
    src/tools/bash.cpp
    src/config/config.cpp
)

target_include_directories(claude_code_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/include
)

target_link_libraries(claude_code_core PUBLIC
    cpr::cpr
    nlohmann_json::nlohmann_json
)

add_executable(claude-code
    src/main.cpp
    src/tui/tui_app.cpp
)

target_link_libraries(claude-code PRIVATE
    claude_code_core
    ftxui::component
    ftxui::dom
    ftxui::screen
)

target_include_directories(claude-code PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/include
)
