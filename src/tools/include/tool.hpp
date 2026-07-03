#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>

class Tool {
public:
    virtual ~Tool() = default;

    virtual std::string_view name() const noexcept = 0;
    virtual std::string_view description() const noexcept = 0;
    virtual nlohmann::json parameters() const = 0;
    virtual std::string execute(const nlohmann::json& arguments) const = 0;

    nlohmann::json definition() const {
        return {
            {"type", "function"},
            {"function", {
                {"name", std::string(name())},
                {"description", std::string(description())},
                {"parameters", parameters()},
            }},
        };
    }
};
