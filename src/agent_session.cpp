#include "agent_session.hpp"
#include "tools.hpp"

#include <nlohmann/json.hpp>

#include <filesystem>
#include <stdexcept>
#include <string>

using json = nlohmann::json;

namespace {

json parse_tool_arguments(const json& arguments) {
    if (arguments.is_string()) {
        return json::parse(arguments.get<std::string>());
    }
    return json::object();
}

json get_first_message(const json& result) {
    if (!result.contains("choices") || !result["choices"].is_array() ||
        result["choices"].empty()) {
        throw std::runtime_error("No choices in response");
    }

    const json& choice = result["choices"][0];
    if (!choice.contains("message") || !choice["message"].is_object()) {
        throw std::runtime_error("No message in response choice");
    }

    return choice["message"];
}

bool has_tool_calls(const json& message) {
    return message.contains("tool_calls") && message["tool_calls"].is_array() &&
           !message["tool_calls"].empty();
}

}  // namespace

AgentSession::AgentSession(
    ChatProvider& provider,
    ToolRegistry& tools,
    const Config& config,
    ToolContext tool_context
)
    : provider_(provider)
    , tools_(tools)
    , config_(config)
    , tool_context_(std::move(tool_context)) {
    available_tools_ = tools_.definitions();
}

void AgentSession::add_user_message(const std::string& content) {
    messages_.push_back({{"role", "user"}, {"content", content}});
}

void AgentSession::run() {
    if (cancelled_) return;

    if (!started_) {
        started_ = true;
    }

    while (!cancelled_ && !done_) {
        if (!step()) break;
    }

    if (!cancelled_) {
        if (done_) {
            notify({AgentEventType::Done, step_});
        } else if (step_ >= MAX_STEPS) {
            notify({AgentEventType::Error, step_, "", "", "", {}, "",
                    "Reached maximum steps (" + std::to_string(MAX_STEPS) + ")"});
        }
    }
}

bool AgentSession::step() {
    if (done_ || cancelled_) {
        done_ = true;
        return false;
    }

    step_++;
    if (step_ > MAX_STEPS) {
        done_ = true;
        return false;
    }

    notify({AgentEventType::Thinking, step_});

    CompletionResponse resp;
    try {
        CompletionRequest req{config_.model, messages_, available_tools_};
        resp = provider_.complete(req);
    } catch (const std::exception& e) {
        notify({AgentEventType::Error, step_, "", "", "", {}, "", e.what()});
        done_ = true;
        return false;
    }

    json message;
    try {
        message = get_first_message(resp.data);
    } catch (const std::exception& e) {
        notify({AgentEventType::Error, step_, "", "", "", {}, "",
                "Failed to parse response: " + std::string(e.what())});
        done_ = true;
        return false;
    }

    messages_.push_back(message);

    if (has_tool_calls(message)) {
        for (const auto& tool_call : message["tool_calls"]) {
            if (cancelled_) {
                done_ = true;
                return false;
            }

            const auto& function = tool_call.at("function");
            auto name = function.at("name").get<std::string>();
            auto args = function.contains("arguments")
                            ? function["arguments"]
                            : json::object();
            auto parsed = parse_tool_arguments(args);
            auto tool_call_id = tool_call.at("id").get<std::string>();

            if (tools_.requires_approval(name, parsed)) {
                notify({AgentEventType::ToolApprovalRequest, step_, "",
                        name, tool_call_id, parsed});

                {
                    std::unique_lock lock(approval_mutex_);
                    awaiting_approval_ = true;
                    tool_approved_ = false;
                    approval_cv_.wait(
                        lock, [this] { return !awaiting_approval_ || cancelled_; });
                }

                if (cancelled_) {
                    done_ = true;
                    return false;
                }

                if (!tool_approved_) {
                    add_tool_result_to_messages(
                        tool_call_id,
                        "User denied execution of tool: " + name);
                    continue;
                }
            }

            notify({AgentEventType::ToolExecution, step_, "", name,
                    tool_call_id, parsed});

            std::string result;
            try {
                result = tools_.execute(name, args, tool_context_);
                if (result.size() > tool_context_.max_output_size) {
                    result = result.substr(0, tool_context_.max_output_size)
                             + "\n...[output truncated]";
                }
            } catch (const std::exception& e) {
                result = "Error executing " + name + ": " + e.what();
            }

            notify({AgentEventType::ToolResult, step_, "", name,
                    tool_call_id, parsed, result});

            add_tool_result_to_messages(tool_call_id, result);
        }

        return true;
    }

    if (message.contains("content") && !message["content"].is_null()) {
        auto content = message["content"].get<std::string>();
        notify({AgentEventType::AssistantMessage, step_, content});
    }

    done_ = true;
    return false;
}

void AgentSession::add_tool_result_to_messages(
    const std::string& tool_call_id,
    const std::string& content) {
    messages_.push_back({
        {"role", "tool"},
        {"tool_call_id", tool_call_id},
        {"content", content},
    });
}

void AgentSession::cancel() {
    cancelled_ = true;
    {
        std::lock_guard lock(approval_mutex_);
        awaiting_approval_ = false;
    }
    approval_cv_.notify_all();
}

void AgentSession::approve_tool() {
    {
        std::lock_guard lock(approval_mutex_);
        awaiting_approval_ = false;
        tool_approved_ = true;
    }
    approval_cv_.notify_one();
}

void AgentSession::deny_tool() {
    {
        std::lock_guard lock(approval_mutex_);
        awaiting_approval_ = false;
        tool_approved_ = false;
    }
    approval_cv_.notify_one();
}

bool AgentSession::poll_event(AgentEvent& event) {
    std::lock_guard lock(event_mutex_);
    if (event_queue_.empty()) return false;
    event = std::move(event_queue_.front());
    event_queue_.pop();
    return true;
}

void AgentSession::set_event_callback(std::function<void()> cb) {
    event_callback_ = std::move(cb);
}

bool AgentSession::is_done() const { return done_; }
bool AgentSession::is_cancelled() const { return cancelled_; }

void AgentSession::notify(const AgentEvent& event) {
    {
        std::lock_guard lock(event_mutex_);
        event_queue_.push(event);
    }
    if (event_callback_) {
        event_callback_();
    }
}
