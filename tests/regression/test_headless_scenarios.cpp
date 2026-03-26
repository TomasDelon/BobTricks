#include "tests/TestSupport.h"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>

#include "config/AppConfig.h"
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
            TEST_EXPECT_MSG(suite, cols.size() == 18, "unexpected telemetry header width");
            continue;
        }
        TEST_EXPECT_MSG(suite, cols.size() == 18, "unexpected telemetry row width");
        if (cols.size() != 18)
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
        TEST_EXPECT(suite, countHeelStrikes(rows) >= 3);
        TEST_EXPECT(suite, countHeelStrikes(rows) <= 8);
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
        TEST_EXPECT(suite, countHeelStrikes(rows) >= 1);
        TEST_EXPECT(suite, rows.back().loco_state != "Airborne");
        TEST_EXPECT(suite, std::all_of(rows.begin(), rows.end(),
            [](const ParsedRow& row) { return !std::isnan(row.cm_x) && !std::isnan(row.cm_y); }));
    }

    return suite.finish();
}
