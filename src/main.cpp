#include <cstring>
#include <memory>
#include <thread>

#include "app/app.hpp"
#include "app/router.hpp"
#include "ui/tui.hpp"
#include "util/logger.hpp"

int main(int argc, char* argv[])
{
    Logger::instance().set_logfile_dir("app.log");

    bool enable_server = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--server") == 0)
            enable_server = true;
    }

    try {
        App app;

        TUI tui(app.tui_queue());

        std::unique_ptr<Router> router;
        if (enable_server) {
            router = std::make_unique<Router>(app.server_queue(), 8080);
            std::thread([&router] { router->run(); }).detach();
        }

        app.set_tui(&tui);

        while (true) {
            std::thread tui_thread([&tui] { tui.run(); });
            app.run();
            tui_thread.join();

            if (app.last_action() == App::Action::Attach) {
                app.attach_agent(app.attach_index());
                app.reset_for_reentry();
            } else {
                break;
            }
        }
    } catch (const std::exception& e) {
        LOG("(MAIN) fatal exception:", e.what());
        return 1;
    }
}
