#include "core/telemetry/TelemetryRecorder.h"

#include <iomanip>
#include <ostream>

// ── helpers ──────────────────────────────────────────────────────────────────

static const char* triggerName(StepTriggerType t)
{
    switch (t) {
        case StepTriggerType::None:      return "None";
        case StepTriggerType::Normal:    return "Normal";
        case StepTriggerType::Emergency: return "Emergency";
    }
    return "None";
}

static const char* locoName(LocomotionState s)
{
    switch (s) {
        case LocomotionState::Standing: return "Standing";
        case LocomotionState::Walking:  return "Walking";
        case LocomotionState::Airborne: return "Airborne";
    }
    return "Standing";
}

// ── TelemetryRecorder ────────────────────────────────────────────────────────

void TelemetryRecorder::record(const SimState& s)
{
    const auto& cm  = s.cm;
    const auto& ch  = s.character;

    TelemetryRow row;
    row.t             = s.sim_time;
    row.cm_x          = cm.position.x;
    row.cm_vx         = cm.velocity.x;
    row.cm_y          = cm.position.y;
    row.cm_vy         = cm.velocity.y;
    row.pelvis_x      = ch.pelvis.x;
    row.xcom          = ch.balance.xcom;
    row.mos           = ch.balance.mos;
    row.support_left  = ch.support.x_left;
    row.support_right = ch.support.x_right;
    row.support_width = ch.support.width();
    row.foot_L_x      = ch.foot_left.pos.x;
    row.foot_R_x      = ch.foot_right.pos.x;
    row.step_active   = ch.step_plan.active;
    row.swing_right   = ch.step_plan.active && ch.step_plan.move_right;  // false when no step in flight
    row.trigger       = ch.last_trigger;
    row.heel_strike   = ch.heel_strike_this_tick;
    row.loco_state    = ch.locomotion_state;

    m_rows.push_back(row);
}

void TelemetryRecorder::writeCsv(std::ostream& out) const
{
    // Frozen header — column order matches TelemetryRow field order.
    out << "t,cm_x,cm_vx,cm_y,cm_vy,"
           "pelvis_x,"
           "xcom,mos,"
           "support_left,support_right,support_width,"
           "foot_L_x,foot_R_x,"
           "step_active,swing_right,"
           "trigger,"
           "heel_strike,"
           "loco_state\n";

    out << std::fixed << std::setprecision(6);
    for (const auto& r : m_rows) {
        out << r.t       << ','
            << r.cm_x    << ',' << r.cm_vx  << ','
            << r.cm_y    << ',' << r.cm_vy  << ','
            << r.pelvis_x << ','
            << r.xcom    << ',' << r.mos     << ','
            << r.support_left  << ',' << r.support_right << ',' << r.support_width << ','
            << r.foot_L_x << ',' << r.foot_R_x << ','
            << static_cast<int>(r.step_active)   << ','
            << static_cast<int>(r.swing_right)   << ','
            << triggerName(r.trigger)             << ','
            << static_cast<int>(r.heel_strike)    << ','
            << locoName(r.loco_state)             << '\n';
    }
}

void TelemetryRecorder::clear()
{
    m_rows.clear();
    m_asserts.clear();
}

void TelemetryRecorder::addAssertion(
    const std::string& name,
    std::function<bool(const std::vector<TelemetryRow>&)> fn)
{
    m_asserts.push_back({ name, std::move(fn) });
}

bool TelemetryRecorder::runAssertions(std::ostream& report) const
{
    bool all_pass = true;
    for (const auto& a : m_asserts) {
        const bool pass = a.check(m_rows);
        report << (pass ? "PASS" : "FAIL") << "  " << a.name << '\n';
        if (!pass) all_pass = false;
    }
    return all_pass;
}
