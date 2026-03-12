#include "platform/web/WebInputBridge.hpp"

#include "platform/sdl/SDLPlatformRuntime.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <string_view>
#endif

namespace bobtricks {

#ifdef __EMSCRIPTEN__
namespace {
SDLPlatformRuntime* activeWebRuntime = nullptr;

LocomotionMode modeFromWebValue(int modeValue) {
    switch (modeValue) {
    case 1:
        return LocomotionMode::Walk;
    case 2:
        return LocomotionMode::Run;
    default:
        return LocomotionMode::Stand;
    }
}

extern "C" {
EMSCRIPTEN_KEEPALIVE void bobtricks_request_mode(int modeValue) {
    if (activeWebRuntime != nullptr) {
        activeWebRuntime->requestMode( modeFromWebValue(modeValue) );
    }
}

EMSCRIPTEN_KEEPALIVE void bobtricks_toggle_pause() {
    if (activeWebRuntime != nullptr) {
        activeWebRuntime->requestPauseToggle();
    }
}

EMSCRIPTEN_KEEPALIVE void bobtricks_speed_up() {
    if (activeWebRuntime != nullptr) {
        activeWebRuntime->requestSpeedUp();
    }
}

EMSCRIPTEN_KEEPALIVE void bobtricks_slow_down() {
    if (activeWebRuntime != nullptr) {
        activeWebRuntime->requestSlowDown();
    }
}

EMSCRIPTEN_KEEPALIVE void bobtricks_request_reset() {
    if (activeWebRuntime != nullptr) {
        activeWebRuntime->requestReset();
    }
}

EMSCRIPTEN_KEEPALIVE void bobtricks_request_quit() {
    if (activeWebRuntime != nullptr) {
        activeWebRuntime->requestQuit();
    }
}

EMSCRIPTEN_KEEPALIVE void bobtricks_toggle_fullscreen() {
    if (activeWebRuntime != nullptr) {
        activeWebRuntime->requestFullscreenToggle();
    }
}
}

EM_BOOL onEmscriptenKeyDown(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData) {
    (void)eventType;

    auto* runtime = static_cast<SDLPlatformRuntime*>(userData);
    if (runtime == nullptr || keyEvent == nullptr) {
        return EM_FALSE;
    }

    const std::string_view code = keyEvent->code;
    const std::string_view key = keyEvent->key;

    if (code == "Escape" || key == "Escape") {
        runtime->requestQuit();
    } else if (code == "F11" || key == "F11") {
        runtime->requestFullscreenToggle();
    } else if (code == "Space" || key == " ") {
        runtime->requestPauseToggle();
    } else if (code == "Minus" || key == "-") {
        runtime->requestSlowDown();
    } else if (code == "Equal" || key == "+" || key == "=") {
        runtime->requestSpeedUp();
    } else if (code == "KeyR" || key == "r" || key == "R") {
        runtime->requestReset();
    } else if (code == "Digit1" || key == "1") {
        runtime->requestMode(LocomotionMode::Stand);
    } else if (code == "Digit2" || key == "2") {
        runtime->requestMode(LocomotionMode::Walk);
    } else if (code == "Digit3" || key == "3") {
        runtime->requestMode(LocomotionMode::Run);
    }

    return EM_TRUE;
}

EM_BOOL onEmscriptenBlur(int eventType, const EmscriptenFocusEvent* focusEvent, void* userData) {
    (void)eventType;
    (void)focusEvent;

    auto* runtime = static_cast<SDLPlatformRuntime*>(userData);
    if (runtime == nullptr) {
        return EM_FALSE;
    }

    runtime->clearPendingWebRequests();
    return EM_FALSE;
}
}  // namespace
#endif

void WebInputBridge::install(SDLPlatformRuntime& platformRuntime) {
#ifdef __EMSCRIPTEN__
    activeWebRuntime = &platformRuntime;
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &platformRuntime, true, &onEmscriptenKeyDown);
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, &platformRuntime, true, &onEmscriptenBlur);
#else
    (void)platformRuntime;
#endif
}

void WebInputBridge::uninstall(SDLPlatformRuntime& platformRuntime) {
#ifdef __EMSCRIPTEN__
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, nullptr);
    emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, nullptr);
    if (activeWebRuntime == &platformRuntime) {
        activeWebRuntime = nullptr;
    }
#else
    (void)platformRuntime;
#endif
}

}  // namespace bobtricks
