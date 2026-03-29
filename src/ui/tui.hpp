#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/screen/box.hpp>

#include "app/event.hpp"
#include "model/agent.hpp"

// BSP tree node for tiling pane layout
enum class SplitDir { HORIZONTAL, VERTICAL };

struct BspNode {
    int id = 0;
    SplitDir dir = SplitDir::VERTICAL;
    int split_size = 0;
    ftxui::Box last_box = {};
    std::unique_ptr<BspNode> left;
    std::unique_ptr<BspNode> right;

    bool is_leaf() const { return !left && !right; }

    static std::unique_ptr<BspNode> make_leaf(int leaf_id) {
        auto n = std::make_unique<BspNode>();
        n->id = leaf_id;
        return n;
    }

    static bool is_leaf_with_id(const std::unique_ptr<BspNode>& child, int target) {
        return child && child->is_leaf() && child->id == target;
    }
};

// BSP tree helpers
BspNode* find_node(BspNode* node, int id);
BspNode* find_parent(BspNode* node, int target_id);
int first_leaf_id(BspNode* node);
int count_leaves(BspNode* node);

class TUI
{
public:
    explicit TUI(EventQueue<TUIEvent>& queue);

    void run();
    void update(const std::map<int, std::unique_ptr<agent::Agent>>& agents);
    void post_event();

private:
    EventQueue<TUIEvent>& event_queue_;
    ftxui::ScreenInteractive screen_;

    // Agent view data (thread-safe copy from App)
    struct AgentView
    {
        std::string name;
        agent::AgentState state;
        std::string preview;
    };

    std::mutex render_mutex_;
    std::map<int, AgentView> agent_views_;

    // BSP tree state
    std::unique_ptr<BspNode> bsp_root_;
    int next_pane_id_ = 0;
    int focused_id_ = -1;
    bool insert_mode_ = false;
    bool show_quit_dialog_ = false;

    // FTXUI component state
    ftxui::Component root_container_;
    int tab_index_ = 0;
    std::function<void()> rebuild_;
    std::map<int, ftxui::Component> leaf_map_;

    // Tree operations
    void split_pane(SplitDir dir);
    void delete_pane();

    // Component building
    ftxui::Component build_component(BspNode* node);
    ftxui::Component build_app();

    // Key forwarding for insert mode
    void forward_event_to_agent(const ftxui::Event& event);
};
