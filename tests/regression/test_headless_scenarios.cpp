#include "tests/TestSupport.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "config/AppConfig.h"
#include "core/physics/Geometry.h"
#include "core/simulation/SimulationCore.h"
#include "core/simulation/SimVerbosity.h"
#include "headless/ScenarioLibrary.h"
#include "headless/ScenarioRunner.h"

namespace {

struct ParsedRow {
    double      cm_x = 0.0;
    double      cm_y = 0.0;
    bool        heel_strike = false;
    std::string loco_state;
};

std::vector<std::string> splitCsvLine(const std::string& line)
{
    std::vector<std::string> out;
    std::stringstream ss(line);
    std::string cell;
    while (std::getline(ss, cell, ','))
        out.push_back(cell);
    return out;
}

std::vector<ParsedRow> parseRows(const std::string& csv, TestSuite& suite)
{
    std::vector<ParsedRow> rows;
    std::stringstream      in(csv);
    std::string            line;
    bool                   header_seen = false;

    while (std::getline(in, line)) {
        if (line.empty())
            continue;
        const auto cols = splitCsvLine(line);
        if (!header_seen) {
            header_seen = true;
            TEST_EXPECT_MSG(suite, cols.size() >= 18, "unexpected telemetry header width");
            continue;
        }
        TEST_EXPECT_MSG(suite, cols.size() >= 18, "unexpected telemetry row width");
        if (cols.size() < 18)
            continue;

        ParsedRow row;
        row.cm_x = std::stod(cols[1]);
        row.cm_y = std::stod(cols[3]);
        row.heel_strike = (cols[16] == "1");
        row.loco_state = cols[17];
        rows.push_back(row);
    }

    return rows;
}

int countHeelStrikes(const std::vector<ParsedRow>& rows)
{
    return static_cast<int>(std::count_if(rows.begin(), rows.end(),
        [](const ParsedRow& row) { return row.heel_strike; }));
}

bool isFinite(Vec2 v)
{
    return std::isfinite(v.x) && std::isfinite(v.y);
}

double distance(Vec2 a, Vec2 b)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    return std::sqrt(dx * dx + dy * dy);
}

}  // namespace

int main()
{
    g_sim_verbose = false;
    TestSuite suite("headless_regression_tests");

    const auto& lib = scenarioLibrary();
    AppConfig   base_cfg;

    {
        AppConfig run_cfg = base_cfg;
        const ScenarioDef def = lib.at("stand_still")(run_cfg);
        std::ostringstream csv;
        std::ostringstream report;
        const bool ok = runScenario(def, run_cfg, csv, report);
        const auto rows = parseRows(csv.str(), suite);

        TEST_EXPECT_MSG(suite, ok, report.str());
        TEST_EXPECT(suite, !rows.empty());

        double max_abs_cm_x = 0.0;
        for (const auto& row : rows)
            max_abs_cm_x = std::max(max_abs_cm_x, std::fabs(row.cm_x));

        TEST_EXPECT(suite, max_abs_cm_x < 0.2);
        TEST_EXPECT(suite, countHeelStrikes(rows) == 0);
        TEST_EXPECT(suite, rows.back().loco_state == "Standing");
    }

    {
        AppConfig run_cfg = base_cfg;
        const ScenarioDef def = lib.at("walk_3s")(run_cfg);
        std::ostringstream csv;
        std::ostringstream report;
        const bool ok = runScenario(def, run_cfg, csv, report);
        const auto rows = parseRows(csv.str(), suite);

        TEST_EXPECT_MSG(suite, ok, report.str());
        TEST_EXPECT(suite, !rows.empty());
        TEST_EXPECT(suite, rows.back().cm_x > 1.5);
        TEST_EXPECT(suite, rows.back().cm_x < 6.0);
        TEST_EXPECT(suite, std::all_of(rows.begin(), rows.end(),
            [](const ParsedRow& row) { return !std::isnan(row.cm_y); }));
        TEST_EXPECT(suite, std::any_of(rows.begin(), rows.end(),
            [](const ParsedRow& row) { return row.loco_state == "Walking"; }));
    }

    {
        AppConfig run_cfg = base_cfg;
        const ScenarioDef def = lib.at("perturbation_recovery")(run_cfg);
        std::ostringstream csv;
        std::ostringstream report;
        const bool ok = runScenario(def, run_cfg, csv, report);
        const auto rows = parseRows(csv.str(), suite);

        TEST_EXPECT_MSG(suite, ok, report.str());
        TEST_EXPECT(suite, !rows.empty());
        TEST_EXPECT(suite, rows.back().loco_state != "Airborne");
        TEST_EXPECT(suite, std::all_of(rows.begin(), rows.end(),
            [](const ParsedRow& row) { return !std::isnan(row.cm_x) && !std::isnan(row.cm_y); }));
    }

    {
        AppConfig run_cfg = base_cfg;
        const ScenarioDef def = lib.at("upper_body_walk_gaze")(run_cfg);
        std::ostringstream csv;
        std::ostringstream report;
        const bool ok = runScenario(def, run_cfg, csv, report);
        const auto rows = parseRows(csv.str(), suite);

        TEST_EXPECT_MSG(suite, ok, report.str());
        TEST_EXPECT(suite, !rows.empty());
        TEST_EXPECT(suite, rows.back().cm_x > 1.0);
        TEST_EXPECT(suite, std::any_of(rows.begin(), rows.end(),
            [](const ParsedRow& row) { return row.loco_state == "Walking"; }));
    }

    {
        AppConfig run_cfg = base_cfg;
        run_cfg.terrain.enabled = false;

        SimulationCore core(run_cfg);
        const double   L = run_cfg.character.body_height_m / 5.0;
        ScenarioInit   init;
        init.cm_pos = {
            0.0,
            computeNominalY(L, run_cfg.standing.d_pref, run_cfg.character.cm_pelvis_ratio)
        };
        core.reset(init);

        bool airborne_seen = false;
        for (int frame = 0; frame < 24; ++frame) {
            InputFrame input;
            if (frame == 2)
                input.set_velocity = Vec2{0.0, run_cfg.physics.jump_impulse};

            core.step(run_cfg.sim_loop.fixed_dt_s, input);
            const SimState& s  = core.state();
            const auto&     ch = s.character;
            if (ch.locomotion_state == LocomotionState::Airborne)
                airborne_seen = true;

            if (!airborne_seen)
                continue;

            TEST_EXPECT(suite, std::isfinite(s.cm.position.x) && std::isfinite(s.cm.position.y));
            TEST_EXPECT(suite, isFinite(ch.pelvis));
            TEST_EXPECT(suite, isFinite(ch.knee_left));
            TEST_EXPECT(suite, isFinite(ch.knee_right));
            TEST_EXPECT(suite, isFinite(ch.foot_left.pos));
            TEST_EXPECT(suite, isFinite(ch.foot_right.pos));
            TEST_EXPECT(suite, isFinite(ch.debug_ground_back));
            TEST_EXPECT(suite, isFinite(ch.debug_ground_fwd));
            TEST_EXPECT(suite, distance(ch.pelvis, ch.foot_left.pos) <= 2.0 * L + 1e-6);
            TEST_EXPECT(suite, distance(ch.pelvis, ch.foot_right.pos) <= 2.0 * L + 1e-6);
        }

        TEST_EXPECT(suite, airborne_seen);
    }

    {
        AppConfig run_cfg = base_cfg;
        SimulationCore core(run_cfg);
        const double   L = run_cfg.character.body_height_m / 5.0;
        ScenarioInit   init;
        init.cm_pos = {
            0.0,
            computeNominalY(L, run_cfg.standing.d_pref, run_cfg.character.cm_pelvis_ratio)
        };
        core.reset(init);

        bool moved_head = false;
        bool moved_arms = false;
        for (int frame = 0; frame < 120; ++frame) {
            InputFrame input;
            input.key_right = (frame < 90);
            input.gaze_target_world = Vec2{2.0, 2.6};

            core.step(run_cfg.sim_loop.fixed_dt_s, input);
            const SimState& s  = core.state();
            const auto& ch = s.character;

            TEST_EXPECT(suite, isFinite(ch.head_center));
            TEST_EXPECT(suite, isFinite(ch.eye_left));
            TEST_EXPECT(suite, isFinite(ch.eye_right));
            TEST_EXPECT(suite, isFinite(ch.elbow_left));
            TEST_EXPECT(suite, isFinite(ch.elbow_right));
            TEST_EXPECT(suite, isFinite(ch.hand_left));
            TEST_EXPECT(suite, isFinite(ch.hand_right));
            TEST_EXPECT(suite, std::isfinite(ch.head_tilt));

            if (std::abs(ch.head_tilt) > 1e-4)
                moved_head = true;
            if ((ch.hand_left - ch.torso_top).length() > 0.25 * L
                && (ch.hand_right - ch.torso_top).length() > 0.25 * L)
                moved_arms = true;
        }

        TEST_EXPECT(suite, moved_head);
        TEST_EXPECT(suite, moved_arms);
    }

    return suite.finish();
}
