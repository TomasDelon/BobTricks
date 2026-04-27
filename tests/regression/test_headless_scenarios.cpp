#include "tests/TestSupport.h"

#include <algorithm>
#include <cmath>
#include <map>
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
    double      cm_vx = 0.0;
    double      cm_y = 0.0;
    bool        foot_left_on_ground = false;
    bool        foot_right_on_ground = false;
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
        const std::vector<std::string> cols = splitCsvLine(line);
        if (!header_seen) {
            header_seen = true;
            TEST_EXPECT_MSG(suite, cols.size() >= 15, "unexpected telemetry header width");
            continue;
        }
        TEST_EXPECT_MSG(suite, cols.size() >= 15, "unexpected telemetry row width");
        if (cols.size() < 15)
            continue;

        ParsedRow row;
        row.cm_x = std::stod(cols[1]);
        row.cm_vx = std::stod(cols[2]);
        row.cm_y = std::stod(cols[3]);
        row.foot_left_on_ground = (cols[10] == "1");
        row.foot_right_on_ground = (cols[11] == "1");
        row.loco_state = cols[13];
        rows.push_back(row);
    }

    return rows;
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

bool everyRowUnderSpeed(const std::vector<ParsedRow>& rows, double max_speed)
{
    for (const ParsedRow& row : rows) {
        if (row.cm_vx > max_speed)
            return false;
    }
    return true;
}

}  // namespace

int main()
{
    g_sim_verbose = false;
    TestSuite suite("headless_regression_tests");

    const std::map<std::string, ScenarioFactory>& lib = scenarioLibrary();
    AppConfig   base_cfg;

    {
        AppConfig run_cfg = base_cfg;
        const ScenarioDef def = lib.at("stand_still")(run_cfg);
        std::ostringstream csv;
        std::ostringstream report;
        const bool ok = runScenario(def, run_cfg, csv, report);
        const std::vector<ParsedRow> rows = parseRows(csv.str(), suite);

        TEST_EXPECT_MSG(suite, ok, report.str());
        TEST_EXPECT(suite, !rows.empty());

        double max_abs_cm_x = 0.0;
        for (const ParsedRow& row : rows)
            max_abs_cm_x = std::max(max_abs_cm_x, std::fabs(row.cm_x));

        TEST_EXPECT(suite, max_abs_cm_x < 0.2);
        TEST_EXPECT(suite, rows.back().loco_state == "Debout");
    }

    {
        AppConfig run_cfg = base_cfg;
        const ScenarioDef def = lib.at("walk_3s")(run_cfg);
        std::ostringstream csv;
        std::ostringstream report;
        const bool ok = runScenario(def, run_cfg, csv, report);
        const std::vector<ParsedRow> rows = parseRows(csv.str(), suite);

        TEST_EXPECT_MSG(suite, ok, report.str());
        TEST_EXPECT(suite, !rows.empty());
        TEST_EXPECT(suite, rows.back().cm_x > 1.5);
        TEST_EXPECT(suite, rows.back().cm_x < 6.0);
        TEST_EXPECT(suite, std::all_of(rows.begin(), rows.end(),
            [](const ParsedRow& row) { return !std::isnan(row.cm_y); }));
        TEST_EXPECT(suite, std::any_of(rows.begin(), rows.end(),
            [](const ParsedRow& row) { return row.loco_state == "Marche"; }));
    }

    {
        AppConfig run_cfg = base_cfg;
        const ScenarioDef def = lib.at("perturbation_recovery")(run_cfg);
        std::ostringstream csv;
        std::ostringstream report;
        const bool ok = runScenario(def, run_cfg, csv, report);
        const std::vector<ParsedRow> rows = parseRows(csv.str(), suite);

        TEST_EXPECT_MSG(suite, ok, report.str());
        TEST_EXPECT(suite, !rows.empty());
        TEST_EXPECT(suite, rows.back().loco_state != "Aerien");
        TEST_EXPECT(suite, std::all_of(rows.begin(), rows.end(),
            [](const ParsedRow& row) { return !std::isnan(row.cm_x) && !std::isnan(row.cm_y); }));
    }

    {
        AppConfig run_cfg = base_cfg;
        const ScenarioDef def = lib.at("walk_max_from_start")(run_cfg);
        std::ostringstream csv;
        std::ostringstream report;
        const bool ok = runScenario(def, run_cfg, csv, report);
        const std::vector<ParsedRow> rows = parseRows(csv.str(), suite);

        TEST_EXPECT_MSG(suite, ok, report.str());
        TEST_EXPECT(suite, rows.size() >= 2);
        TEST_EXPECT(suite, rows[1].loco_state == "Marche");
        TEST_EXPECT(suite, std::abs(rows[1].cm_vx - run_cfg.physics.walk_max_speed) <= 0.05);
        TEST_EXPECT(suite, everyRowUnderSpeed(rows, run_cfg.physics.walk_max_speed + 0.05));
    }

    {
        AppConfig run_cfg = base_cfg;
        const ScenarioDef def = lib.at("run_3s")(run_cfg);
        std::ostringstream csv;
        std::ostringstream report;
        const bool ok = runScenario(def, run_cfg, csv, report);
        const std::vector<ParsedRow> rows = parseRows(csv.str(), suite);

        TEST_EXPECT_MSG(suite, ok, report.str());
        TEST_EXPECT(suite, !rows.empty());
        TEST_EXPECT(suite, rows.back().cm_x > 4.0);
        TEST_EXPECT(suite, std::any_of(rows.begin(), rows.end(),
            [](const ParsedRow& row) { return row.loco_state == "Course"; }));
        TEST_EXPECT(suite, std::any_of(rows.begin(), rows.end(),
            [](const ParsedRow& row) { return row.foot_left_on_ground != row.foot_right_on_ground; }));
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
            const SimState&       s  = core.state();
            const CharacterState& ch = s.character;
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
        run_cfg.terrain.enabled = false;

        SimulationCore core(run_cfg);
        const double   L = run_cfg.character.body_height_m / 5.0;
        ScenarioInit   init;
        init.cm_pos = {
            0.0,
            computeNominalY(L, run_cfg.standing.d_pref, run_cfg.character.cm_pelvis_ratio)
        };
        core.reset(init);

        bool preload_seen = false;
        bool airborne_seen = false;
        for (int frame = 0; frame < 60; ++frame) {
            InputFrame input;
            if (frame == 1)
                input.jump = true;

            const double prev_y = core.state().cm.position.y;
            core.step(run_cfg.sim_loop.fixed_dt_s, input);
            const SimState&       s  = core.state();
            const CharacterState& ch = s.character;

            if (!airborne_seen && s.cm.position.y < prev_y - 1e-5)
                preload_seen = true;
            if (ch.locomotion_state == LocomotionState::Airborne)
                airborne_seen = true;

            TEST_EXPECT(suite, isFinite(ch.foot_left.pos));
            TEST_EXPECT(suite, isFinite(ch.foot_right.pos));
        }

        TEST_EXPECT(suite, preload_seen);
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

        bool head_ready = false;
        bool moved_arms = false;
        for (int frame = 0; frame < 120; ++frame) {
            InputFrame input;
            input.key_right = (frame < 90);
            core.step(run_cfg.sim_loop.fixed_dt_s, input);
            const SimState&       s  = core.state();
            const CharacterState& ch = s.character;

            TEST_EXPECT(suite, isFinite(ch.head_center));
            TEST_EXPECT(suite, isFinite(ch.elbow_left));
            TEST_EXPECT(suite, isFinite(ch.elbow_right));
            TEST_EXPECT(suite, isFinite(ch.hand_left));
            TEST_EXPECT(suite, isFinite(ch.hand_right));

            if (ch.head_radius > 0.0 && ch.head_center.y > ch.torso_top.y)
                head_ready = true;
            if ((ch.hand_left - ch.torso_top).length() > 0.25 * L
                && (ch.hand_right - ch.torso_top).length() > 0.25 * L)
                moved_arms = true;
        }

        TEST_EXPECT(suite, head_ready);
        TEST_EXPECT(suite, moved_arms);
    }

    return suite.finish();
}
