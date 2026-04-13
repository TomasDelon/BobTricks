#include "app/Application.hpp"

#include <memory>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

namespace {
std::unique_ptr<bobtricks::Application> application;
}

#ifdef __EMSCRIPTEN__
void webLoop() {
    if (application == nullptr || !application->runFrame()) {
        if (application != nullptr) {
            application->shutdown();
            application.reset();
        }
        emscripten_cancel_main_loop();
    }
}
#endif

int main() {
    application = std::make_unique<bobtricks::Application>();
    if (!application->initialize()) {
        application.reset();
        return 1;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(webLoop, 0, 1);
    return 0;
#else
    const int exitCode = application->run();
    application->shutdown();
    application.reset();
    return exitCode;
#endif
}
