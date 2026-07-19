cmake_minimum_required(VERSION 3.13)

project(codecrafters-claude-code)

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
        include/*.h
        include/*.hpp
        src/*.cpp
        src/*.h
        src/*.hpp)

include(FetchContent)

FetchContent_Declare(
        cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG        1.10.x
)
FetchContent_MakeAvailable(cpr)

FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG        v3.11.3
)
FetchContent_MakeAvailable(nlohmann_json)


add_executable(claude-code ${SOURCE_FILES}
        src/tools/read.cpp
        src/tools/read.cpp
        src/tools/write.cpp
        src/tools/include/read.h
        src/tools/include/tool.hpp
        src/tools/bash.cpp
        src/tools/include/bash.h)

target_include_directories(claude-code PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tools/include)

target_link_libraries(claude-code PRIVATE cpr::cpr nlohmann_json::nlohmann_json)
