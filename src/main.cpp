#include "app.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#ifdef __EMSCRIPTEN__
void webLoop() {
    if (!appStep()) {
        appShutdown();
        emscripten_cancel_main_loop();
    }
}
#endif

int main() {
    if (!appInit()) {
        return 1;
    }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(webLoop, 0, 1);
    return 0;
#else
    while (appStep()) {
    }
    appShutdown();
    return 0;
#endif
}
