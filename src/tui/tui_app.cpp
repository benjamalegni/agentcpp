#include "tui/tui_app.hpp"
#include "agent_session.hpp"

#include <ftxui/component/captured_mouse.hpp>
#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/loop.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>

namespace {

struct TuiState {
    std::vector<std::string> chat_lines;
    std::string input_text;
    std::string status;
    bool processing = false;
    bool showing_approval = false;
    std::string approval_text;
};

ftxui::Element render_history(const std::shared_ptr<TuiState>& state) {
    ftxui::Elements items;
    for (const auto& line : state->chat_lines) {
        items.push_back(ftxui::paragraphAlignLeft(line));
    }
    if (state->processing) {
        items.push_back(ftxui::text(""));
        items.push_back(ftxui::text(state->status));
    }
    return ftxui::vbox(std::move(items)) | ftxui::vscroll_indicator |
           ftxui::frame | ftxui::yflex;
}

ftxui::Element render_approval_dialog(
    const std::shared_ptr<TuiState>& state
) {
    return ftxui::window(
               ftxui::text(" Approval Required ") | ftxui::bold |
                   ftxui::color(ftxui::Color::Yellow),
               ftxui::vbox({
                   ftxui::paragraphAlignLeft(state->approval_text),
                   ftxui::separator(),
                   ftxui::hbox({
                       ftxui::text("  Enter: Approve  |  Esc: Deny  ") |
                           ftxui::dim,
                   }) | ftxui::center,
               })) |
           ftxui::border | ftxui::clear_under | ftxui::center;
}

ftxui::Element render_status_bar(const std::shared_ptr<TuiState>& state) {
    auto status = state->status.empty() ? ftxui::text(" Ready ")
                                        : ftxui::text(" " + state->status + " ");
    if (state->processing) {
        status = status | ftxui::color(ftxui::Color::Green);
    }
    return status | ftxui::bold;
}

}  // namespace

int run_tui(AgentSession& session) {
    auto state = std::make_shared<TuiState>();
    auto screen = ftxui::ScreenInteractive::Fullscreen();
    std::jthread agent_thread;

    state->chat_lines.push_back(
        "Welcome to claude-code. Type a message and press Enter to start.");
    state->chat_lines.push_back("");

    auto input = ftxui::Input(&state->input_text, "Type a message...");

    auto history = ftxui::Renderer(
        [state] { return render_history(state); }
    );

    auto container = ftxui::Container::Vertical({
        history,
        input,
    });

    container |= ftxui::CatchEvent([&](const ftxui::Event& event) {
        if (event.is_character() && event.character()[0] == '\x03') {
            screen.ExitLoopClosure();
            return true;
        }

        if (state->showing_approval) {
            if (event == ftxui::Event::Return) {
                session.approve_tool();
                state->showing_approval = false;
                state->chat_lines.push_back("  [Tool approved]");
            } else if (event == ftxui::Event::Escape) {
                session.deny_tool();
                state->showing_approval = false;
                state->chat_lines.push_back("  [Tool denied]");
            }
            return true;
        }

        if (event == ftxui::Event::Return && !state->processing &&
            !state->input_text.empty()) {
            auto msg = std::move(state->input_text);
            state->input_text.clear();

            size_t pos = 0;
            while (pos < msg.size()) {
                auto next = msg.find('\n', pos);
                auto line = msg.substr(pos, next - pos);
                state->chat_lines.push_back("> " + line);

                if (next == std::string::npos) {
                    state->chat_lines.push_back("");
                }
                pos = (next == std::string::npos) ? msg.size() : next + 1;
            }

            state->processing = true;
            state->status = "Sending request...";

            agent_thread = std::jthread([&session, state, msg, &screen] {
                session.add_user_message(msg);
                session.run();
                screen.PostEvent(ftxui::Event::Custom);
            });

            return true;
        }

        return false;
    });

    auto component = ftxui::Renderer(container, [state, container] {
        auto body = ftxui::vbox({
            render_history(state) | ftxui::yflex,
            ftxui::separator(),
            container->Render() | ftxui::notflex,
            render_status_bar(state),
        });

        if (state->showing_approval) {
            body = ftxui::dbox({
                body,
                render_approval_dialog(state),
            });
        }

        return body;
    });

    session.set_event_callback([&screen] {
        screen.PostEvent(ftxui::Event::Custom);
    });

    auto loop = ftxui::Loop(&screen, component);

    while (!loop.HasQuitted()) {
        loop.RunOnce();

        AgentEvent event;
        while (session.poll_event(event)) {
            switch (event.type) {
                case AgentEventType::Thinking:
                    state->status = "Thinking...";
                    break;

                case AgentEventType::AssistantMessage:
                    state->chat_lines.push_back(event.content);
                    state->processing = false;
                    state->status = "Ready";
                    break;

                case AgentEventType::ToolExecution:
                    state->status = "Executing: " + event.tool_name;
                    break;

                case AgentEventType::ToolResult: {
                    state->chat_lines.push_back("  [" + event.tool_name +
                                                " completed]");
                    auto result_preview =
                        event.tool_result.substr(0, 800);
                    if (!result_preview.empty()) {
                        state->chat_lines.push_back("    " + result_preview);
                    }
                    break;
                }

                case AgentEventType::ToolApprovalRequest:
                    state->approval_text =
                        "Tool: " + event.tool_name + "\n";
                    if (event.tool_arguments.contains("command")) {
                        state->approval_text +=
                            "Command: " +
                            event.tool_arguments["command"]
                                .get<std::string>();
                    } else {
                        state->approval_text +=
                            "Arguments: " +
                            event.tool_arguments.dump(2);
                    }
                    state->showing_approval = true;
                    break;

                case AgentEventType::Error:
                    state->chat_lines.push_back(
                        "Error: " + event.error_message);
                    state->processing = false;
                    state->status = "Error";
                    break;

                case AgentEventType::Done:
                    state->status = "Ready";
                    break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    session.cancel();
    if (agent_thread.joinable()) agent_thread.join();

    return 0;
}
