#pragma once

#include "tool.hpp"

class ReadTool final : public Tool {
public:
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    nlohmann::json parameters() const override;
    std::string execute(const nlohmann::json& arguments) const override;
};
