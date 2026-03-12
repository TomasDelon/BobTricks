#pragma once

namespace bobtricks {

/**
 * \brief Snapshot minimal expose aux interfaces de debug.
 */
struct DebugSnapshot {
    bool paused {false};
    double timeScale {1.0};
};

}  // namespace bobtricks
