//
// Created by luka on 7/19/26.
//

#ifndef CODECRAFTERS_CLAUDE_CODE_BASH_H
#define CODECRAFTERS_CLAUDE_CODE_BASH_H

#endif //CODECRAFTERS_CLAUDE_CODE_BASH_H
#include "tool.hpp"

using namespace std;

class BashTool final : public Tool {
public:
    std::string_view name() const noexcept override;
    std::string_view description() const noexcept override;
    nlohmann::json parameters() const override;
    std::string execute(const nlohmann::json& arguments) const override;
};
