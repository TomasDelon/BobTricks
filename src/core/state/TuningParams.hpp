#pragma once

namespace bobtricks {

/**
 * \brief Parametres ajustables du runtime procedural.
 */
struct TuningParams {
    struct Timing {
        double fixedStepSeconds {1.0 / 60.0};
        double walkCycleDurationS {1.0};
        double runCycleDurationS {0.65};
    } timing {};

    struct Debug {
        double defaultTimeScale {1.0};
        bool startPaused {false};
    } debug {};
};

}  // namespace bobtricks
