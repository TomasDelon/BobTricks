#include "app/Application.hpp"

namespace bobtricks {

Application::Application()
    : consoleDebugUi_(debugCommandBus_), imguiDebugUi_(debugCommandBus_) {
}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    if (initialized_) {
        return true;
    }

    if (!platformRuntime_.initialize()) {
        return false;
    }

    platformRuntime_.setMinimumWindowSize(320, 240);

    if (!renderer_.initialize(platformRuntime_.getNativeWindowHandle())) {
        platformRuntime_.shutdown();
        return false;
    }

    simulationCore_.initialize();
    currentIntent_ = {};
    initialized_ = true;
    running_ = true;
    return true;
}

int Application::run() {
    while (runFrame()) {
    }
    return 0;
}

bool Application::runFrame() {
    if (!initialized_ || !running_) {
        return false;
    }

    const PlatformEvents platformEvents = platformRuntime_.pollEvents();
    if (platformEvents.quitRequested) {
        running_ = false;
        return false;
    }
    if (platformEvents.toggleFullscreenRequested) {
        platformRuntime_.toggleFullscreen();
    }
    if (platformEvents.togglePauseRequested) {
        debugCommandBus_.togglePause();
    }
    if (platformEvents.speedUpRequested) {
        debugCommandBus_.speedUp();
    }
    if (platformEvents.slowDownRequested) {
        debugCommandBus_.slowDown();
    }

    currentIntent_ = inputMapper_.map(platformEvents, currentIntent_);

    consoleDebugUi_.tick();
    imguiDebugUi_.tick();

    simulationLoop_.addFrameTime(
        platformRuntime_.tickSeconds(),
        debugCommandBus_.getTimeScale(),
        debugCommandBus_.isPaused()
    );

    while (simulationLoop_.shouldRunStep()) {
        simulationCore_.setIntent(currentIntent_);
        simulationCore_.step(SimulationLoop::kFixedStepSeconds);
        simulationLoop_.consumeStep();
    }

    const RenderState renderState = renderStateAdapter_.build(
        simulationCore_.getCharacterState(),
        platformRuntime_.getWindowSize()
    );
    renderer_.render(renderState);
    return true;
}

void Application::shutdown() {
    if (!initialized_) {
        return;
    }

    renderer_.shutdown();
    platformRuntime_.shutdown();
    initialized_ = false;
    running_ = false;
}

}  // namespace bobtricks
