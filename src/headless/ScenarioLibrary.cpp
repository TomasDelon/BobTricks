#include "headless/ScenarioLibrary.h"

#include <algorithm>
#include <vector>

#include "core/physics/Geometry.h"
#include "core/telemetry/TelemetryRow.h"

// ── helpers ──────────────────────────────────────────────────────────────────

static double nominalCMHeight(const AppConfig& cfg)
{
    const double L = cfg.character.body_height_m / 5.0;
    return computeNominalY(L, cfg.standing.d_pref, cfg.character.cm_pelvis_ratio);
}

static int countRows(const std::vector<TelemetryRow>& rows,
                     bool (*pred)(const TelemetryRow&))
{
    return static_cast<int>(std::count_if(rows.begin(), rows.end(), pred));
}

// ── scenario factories ────────────────────────────────────────────────────────

// walk_3s — key_right held for 1.5 s then released; expect forward locomotion.
static ScenarioDef makeWalk3s(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "walk_3s";
    def.duration_s    = 3.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 2.0, 0.0 };
    def.init.terrain_seed = 42;

    def.input_fn = [](double t) -> InputFrame {
        InputFrame f;
        f.key_right = (t < 1.5);
        return f;
    };

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("cm_x > 1.5 at end", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty() && rows.back().cm_x > 1.5;
        });
        rec.addAssertion("loco_state Walking at some point", [](const std::vector<TelemetryRow>& rows) {
            return std::any_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.loco_state == LocomotionState::Walking; });
        });
        rec.addAssertion("no NaN in cm_x", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_x != r.cm_x; });
        });
    };

    return def;
}

// stand_still — no input; CM must stay near origin with no steps.
static ScenarioDef makeStandStill(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "stand_still";
    def.duration_s    = 2.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 0.0, 0.0 };
    def.init.terrain_seed = 42;

    // input_fn left null — no input

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("|cm_x| < 0.2 throughout", [](const std::vector<TelemetryRow>& rows) {
            return std::all_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_x > -0.2 && r.cm_x < 0.2; });
        });
        rec.addAssertion("no heel_strikes", [](const std::vector<TelemetryRow>& rows) {
            return countRows(rows, [](const TelemetryRow& r){ return r.heel_strike; }) == 0;
        });
        rec.addAssertion("loco_state Standing at end", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty()
                && rows.back().loco_state == LocomotionState::Standing;
        });
    };

    return def;
}

// walk_then_stop — walk right for 1.5 s then release; expect deceleration to Standing.
static ScenarioDef makeWalkThenStop(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "walk_then_stop";
    def.duration_s    = 5.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 1.2, 0.0 };
    def.init.terrain_seed = 42;

    def.input_fn = [](double t) -> InputFrame {
        InputFrame f;
        f.key_right = (t < 1.5);
        return f;
    };

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("cm_x > 0 at end", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty() && rows.back().cm_x > 0.0;
        });
        rec.addAssertion("reaches Standing before end", [](const std::vector<TelemetryRow>& rows) {
            return std::any_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.loco_state == LocomotionState::Standing; });
        });
        rec.addAssertion("no NaN in cm_y", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_y != r.cm_y; });
        });
    };

    return def;
}

// fast_walk — high initial speed, key held throughout; tests burst of rapid steps.
static ScenarioDef makeFastWalk(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "fast_walk";
    def.duration_s    = 3.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 3.0, 0.0 };
    def.init.terrain_seed = 42;

    def.input_fn = [](double t) -> InputFrame {
        InputFrame f;
        f.key_right = (t < 3.0);
        return f;
    };

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("cm_x > 3.0 at end", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty() && rows.back().cm_x > 3.0;
        });
        rec.addAssertion("no NaN in cm_x", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_x != r.cm_x; });
        });
        rec.addAssertion("no NaN in cm_y", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_y != r.cm_y; });
        });
    };

    return def;
}

// walk_left — negative facing and key_left; tests symmetric locomotion direction.
static ScenarioDef makeWalkLeft(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "walk_left";
    def.duration_s    = 3.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { -1.5, 0.0 };
    def.init.terrain_seed = 42;

    def.input_fn = [](double t) -> InputFrame {
        InputFrame f;
        f.key_left = (t < 3.0);
        return f;
    };

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("cm_x < -1.5 at end", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty() && rows.back().cm_x < -1.5;
        });
        rec.addAssertion("no NaN in cm_x", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_x != r.cm_x; });
        });
    };

    return def;
}

// perturbation_recovery — walk right, then inject a sudden backward velocity at t=0.5s.
// Safety test: CM must not produce NaN and locomotion must not freeze.
static ScenarioDef makePerturbationRecovery(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "perturbation_recovery";
    def.duration_s    = 4.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 1.5, 0.0 };
    def.init.terrain_seed = 42;

    // Walk right for 1.5 s.  At t ∈ [0.50, 0.52) inject a sharp backward kick
    // via set_velocity, then continue holding key_right to drive recovery.
    def.input_fn = [](double t) -> InputFrame {
        InputFrame f;
        f.key_right = (t < 1.5);
        if (t >= 0.50 && t < 0.52)
            f.set_velocity = Vec2{ -2.0, 0.0 };
        return f;
    };

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("no NaN in cm_x", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_x != r.cm_x; });
        });
        rec.addAssertion("no NaN in cm_y", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_y != r.cm_y; });
        });
        rec.addAssertion("not stuck Airborne at end", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty()
                && rows.back().loco_state != LocomotionState::Airborne;
        });
    };

    return def;
}

// upper_body_walk_gaze — walk right with a fixed gaze target to exercise
// head + arms without changing the underlying locomotion assertions model.
static ScenarioDef makeUpperBodyWalkGaze(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "upper_body_walk_gaze";
    def.duration_s    = 2.5;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 1.2, 0.0 };
    def.init.terrain_seed = 42;

    def.input_fn = [](double t) -> InputFrame {
        InputFrame f;
        f.key_right = (t < 2.0);
        f.gaze_target_world = Vec2{ 2.0, 2.6 };
        return f;
    };

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("upper_body_walk_gaze no NaN in cm_x", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_x != r.cm_x; });
        });
        rec.addAssertion("upper_body_walk_gaze no NaN in cm_y", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_y != r.cm_y; });
        });
        rec.addAssertion("upper_body_walk_gaze walks forward", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty() && rows.back().cm_x > 1.0;
        });
        rec.addAssertion("upper_body_walk_gaze reaches Walking", [](const std::vector<TelemetryRow>& rows) {
            return std::any_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.loco_state == LocomotionState::Walking; });
        });
    };

    return def;
}

// ── catalog ───────────────────────────────────────────────────────────────────

const std::map<std::string, ScenarioFactory>& scenarioLibrary()
{
    static const std::map<std::string, ScenarioFactory> lib = {
        { "walk_3s",               makeWalk3s               },
        { "stand_still",           makeStandStill           },
        { "walk_then_stop",        makeWalkThenStop         },
        { "fast_walk",             makeFastWalk             },
        { "walk_left",             makeWalkLeft             },
        { "perturbation_recovery", makePerturbationRecovery },
        { "upper_body_walk_gaze",  makeUpperBodyWalkGaze    },
    };
    return lib;
}
