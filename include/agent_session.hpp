#pragma once

#include "chat_provider.hpp"
#include "config/config.hpp"
#include "tool_context.hpp"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <nlohmann/json.hpp>
#include <queue>
#include <string>
#include <vector>

class ToolRegistry;

enum class AgentEventType {
    Thinking,
    AssistantMessage,
    ToolExecution,
    ToolResult,
    ToolApprovalRequest,
    Error,
    Done
};

struct AgentEvent {
    AgentEventType type{};
    int step = 0;
    std::string content;
    std::string tool_name;
    std::string tool_call_id;
    nlohmann::json tool_arguments;
    std::string tool_result;
    std::string error_message;
};

class AgentObserver {
public:
    virtual ~AgentObserver() = default;
    virtual void on_event(const AgentEvent& event) = 0;
};

class AgentSession {
public:
    AgentSession(
        ChatProvider& provider,
        ToolRegistry& tools,
        const Config& config,
        ToolContext tool_context
    );

    void add_user_message(const std::string& content);
    void run();
    void cancel();

    void approve_tool();
    void deny_tool();

    bool poll_event(AgentEvent& event);
    void set_event_callback(std::function<void()> cb);

    bool is_done() const;
    bool is_cancelled() const;

    static constexpr int MAX_STEPS = 50;

private:
    void notify(const AgentEvent& event);
    bool step();

    void add_tool_result_to_messages(
        const std::string& tool_call_id,
        const std::string& content
    );

    ChatProvider& provider_;
    ToolRegistry& tools_;
    Config config_;
    ToolContext tool_context_;

    nlohmann::json messages_;
    nlohmann::json available_tools_;

    int step_ = 0;
    bool started_ = false;
    bool done_ = false;
    std::atomic<bool> cancelled_{false};

    bool awaiting_approval_ = false;
    bool tool_approved_ = false;
    std::mutex approval_mutex_;
    std::condition_variable approval_cv_;

    std::queue<AgentEvent> event_queue_;
    std::mutex event_mutex_;
    std::function<void()> event_callback_;
};
