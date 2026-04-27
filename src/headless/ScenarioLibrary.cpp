#include "headless/ScenarioLibrary.h"

#include <algorithm>

#include "core/physics/Geometry.h"
#include "core/telemetry/TelemetryRow.h"

// ── Aides ────────────────────────────────────────────────────────────────────

static double nominalCMHeight(const AppConfig& cfg)
{
    const double L = cfg.character.body_height_m / 5.0;
    return computeNominalY(L, cfg.standing.d_pref, cfg.character.cm_pelvis_ratio);
}

// ── Fabriques de scénarios ───────────────────────────────────────────────────

// walk_3s — key_right maintenue pendant 1,5 s puis relâchée ; locomotion vers l'avant attendue.
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

// stand_still — aucune entrée ; le CM doit rester près de l'origine sans pas.
static ScenarioDef makeStandStill(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "stand_still";
    def.duration_s    = 2.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 0.0, 0.0 };
    def.init.terrain_seed = 42;

    // input_fn laissée nulle — aucune entrée

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("|cm_x| < 0.2 throughout", [](const std::vector<TelemetryRow>& rows) {
            return std::all_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_x > -0.2 && r.cm_x < 0.2; });
        });
        rec.addAssertion("loco_state Standing at end", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty()
                && rows.back().loco_state == LocomotionState::Standing;
        });
    };

    return def;
}

// walk_then_stop — marcher à droite pendant 1,5 s puis relâcher ; décélération vers Standing attendue.
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

// fast_walk — vitesse initiale élevée, touche maintenue tout du long ; teste une rafale de pas rapides.
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

// walk_max_from_start — démarrer immédiatement à la vitesse de marche maximale et vérifier que
// la première image simulée est déjà dans un régime de marche saturé.
static ScenarioDef makeWalkMaxFromStart(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "walk_max_from_start";
    def.duration_s    = 1.5;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { cfg.physics.walk_max_speed, 0.0 };
    def.init.terrain_seed = 42;

    def.input_fn = [](double) -> InputFrame {
        InputFrame f;
        f.key_right = true;
        return f;
    };

    def.setup_asserts = [cfg](TelemetryRecorder& rec) {
        rec.addAssertion("walk_max_from_start has first stepped row", [](const std::vector<TelemetryRow>& rows) {
            return rows.size() >= 2;
        });
        rec.addAssertion("walk_max_from_start enters Walking immediately", [](const std::vector<TelemetryRow>& rows) {
            return rows.size() >= 2 && rows[1].loco_state == LocomotionState::Walking;
        });
        rec.addAssertion("walk_max_from_start keeps max walk speed", [cfg](const std::vector<TelemetryRow>& rows) {
            return rows.size() >= 2
                && std::abs(rows[1].cm_vx - cfg.physics.walk_max_speed) <= 0.05;
        });
        rec.addAssertion("walk_max_from_start never exceeds walk cap", [cfg](const std::vector<TelemetryRow>& rows) {
            return std::all_of(rows.begin(), rows.end(),
                [cfg](const TelemetryRow& r){ return r.cm_vx <= cfg.physics.walk_max_speed + 0.05; });
        });
    };

    return def;
}

// run_3s — maintenir droite + course pendant 3 secondes ; vol explicite et mouvement avant soutenu attendus.
static ScenarioDef makeRun3s(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "run_3s";
    def.duration_s    = 3.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 2.4, 0.0 };
    def.init.terrain_seed = 42;

    def.input_fn = [](double) -> InputFrame {
        InputFrame f;
        f.key_right = true;
        f.key_run   = true;
        return f;
    };

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("run_3s cm_x > 4.0 at end", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty() && rows.back().cm_x > 4.0;
        });
        rec.addAssertion("run_3s enters Running", [](const std::vector<TelemetryRow>& rows) {
            return std::any_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.loco_state == LocomotionState::Running; });
        });
        rec.addAssertion("run_3s contains single support", [](const std::vector<TelemetryRow>& rows) {
            return std::any_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.foot_left_on_ground != r.foot_right_on_ground; });
        });
    };

    return def;
}

// walk_left — orientation négative et key_left ; teste la symétrie de la direction de locomotion.
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

// perturbation_recovery — marcher à droite, puis injecter une vitesse arrière brutale à t=0,5 s.
// Test de robustesse : le CM ne doit pas produire de NaN et la locomotion ne doit pas se bloquer.
static ScenarioDef makePerturbationRecovery(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "perturbation_recovery";
    def.duration_s    = 4.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 1.5, 0.0 };
    def.init.terrain_seed = 42;

    // Marcher à droite pendant 1,5 s. À t ∈ [0,50, 0,52), injecter un coup arrière brutal
    // via set_velocity, puis continuer à maintenir key_right pour provoquer la récupération.
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

// jump_from_stand — appuyer une fois sur saut depuis l'arrêt ; vérifier l'arc et un atterrissage propre.
static ScenarioDef makeJumpFromStand(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "jump_from_stand";
    def.duration_s    = 3.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 0.0, 0.0 };
    def.init.terrain_seed = 42;

    def.input_fn = [](double t) -> InputFrame {
        InputFrame f;
        f.jump = (t >= 0.4 && t < 0.5);
        return f;
    };

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("jump_from_stand reaches Airborne", [](const std::vector<TelemetryRow>& rows) {
            return std::any_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.loco_state == LocomotionState::Airborne; });
        });
        rec.addAssertion("jump_from_stand cm_vy peaks", [](const std::vector<TelemetryRow>& rows) {
            return std::any_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_vy > 2.0; });
        });
        rec.addAssertion("jump_from_stand lands cleanly", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty() && rows.back().loco_state != LocomotionState::Airborne;
        });
        rec.addAssertion("jump_from_stand no NaN in cm_y", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_y != r.cm_y; });
        });
    };

    return def;
}

// jump_from_walk — sauter en marchant ; vérifier l'arc, l'atterrissage et l'absence de NaN.
static ScenarioDef makeJumpFromWalk(const AppConfig& cfg)
{
    ScenarioDef def;
    def.name          = "jump_from_walk";
    def.duration_s    = 4.0;
    def.init.cm_pos   = { 0.0, nominalCMHeight(cfg) };
    def.init.cm_vel   = { 1.5, 0.0 };
    def.init.terrain_seed = 42;

    def.input_fn = [](double t) -> InputFrame {
        InputFrame f;
        f.key_right = (t < 3.0);
        f.jump = (t >= 0.5 && t < 0.6);
        return f;
    };

    def.setup_asserts = [](TelemetryRecorder& rec) {
        rec.addAssertion("jump_from_walk reaches Airborne", [](const std::vector<TelemetryRow>& rows) {
            return std::any_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.loco_state == LocomotionState::Airborne; });
        });
        rec.addAssertion("jump_from_walk cm_vy peaks", [](const std::vector<TelemetryRow>& rows) {
            return std::any_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_vy > 2.0; });
        });
        rec.addAssertion("jump_from_walk lands cleanly", [](const std::vector<TelemetryRow>& rows) {
            return !rows.empty() && rows.back().loco_state != LocomotionState::Airborne;
        });
        rec.addAssertion("jump_from_walk no NaN in cm_x", [](const std::vector<TelemetryRow>& rows) {
            return std::none_of(rows.begin(), rows.end(),
                [](const TelemetryRow& r){ return r.cm_x != r.cm_x; });
        });
    };

    return def;
}

// ── Catalogue ────────────────────────────────────────────────────────────────

const std::map<std::string, ScenarioFactory>& scenarioLibrary()
{
    static const std::map<std::string, ScenarioFactory> lib = {
        { "walk_3s",               makeWalk3s               },
        { "stand_still",           makeStandStill           },
        { "walk_then_stop",        makeWalkThenStop         },
        { "fast_walk",             makeFastWalk             },
        { "walk_max_from_start",   makeWalkMaxFromStart     },
        { "run_3s",                makeRun3s                },
        { "walk_left",             makeWalkLeft             },
        { "perturbation_recovery", makePerturbationRecovery },
        { "jump_from_stand",       makeJumpFromStand        },
        { "jump_from_walk",        makeJumpFromWalk         },
    };
    return lib;
}
