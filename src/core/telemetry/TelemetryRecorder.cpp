#include "core/telemetry/TelemetryRecorder.h"

#include <iomanip>
#include <ostream>

static const char* locoName(LocomotionState s)
{
    switch (s) {
        case LocomotionState::Standing: return "Debout";
        case LocomotionState::Walking:  return "Marche";
        case LocomotionState::Running:  return "Course";
        case LocomotionState::Airborne: return "Aerien";
    }
    return "Debout";
}

void TelemetryRecorder::record(const SimState& s)
{
    const CMState&        cm = s.cm;
    const CharacterState& ch = s.character;

    TelemetryRow row;
    row.t                 = s.sim_time;
    row.cm_x              = cm.position.x;
    row.cm_vx             = cm.velocity.x;
    row.cm_y              = cm.position.y;
    row.cm_vy             = cm.velocity.y;
    row.pelvis_x          = ch.pelvis.x;
    row.loco_state        = ch.locomotion_state;
    row.foot_left_x          = ch.foot_left.pos.x;
    row.foot_right_x         = ch.foot_right.pos.x;
    row.foot_left_y          = ch.foot_left.pos.y;
    row.foot_right_y         = ch.foot_right.pos.y;
    row.foot_left_on_ground  = ch.foot_left.on_ground;
    row.foot_right_on_ground = ch.foot_right.on_ground;
    row.cm_target_y       = ch.debug_cm_target_y;
    row.ref_ground        = ch.debug_ref_ground;
    row.ref_slope         = ch.debug_ref_slope;
    row.h_ip              = ch.debug_h_ip;
    row.cm_offset         = ch.debug_cm_offset;
    row.speed_drop        = ch.debug_speed_drop;
    row.slope_drop        = ch.debug_slope_drop;

    m_rows.push_back(row);
}

void TelemetryRecorder::writeCsv(std::ostream& out) const
{
    out << "t,cm_x,cm_vx,cm_y,cm_vy,"
           "pelvis_x,"
           "foot_left_x,foot_right_x,"
           "foot_left_y,foot_right_y,"
           "foot_left_on_ground,foot_right_on_ground,"
           "cm_target_y,"
           "loco_state,"
           "ref_ground,ref_slope,h_ip,cm_offset,speed_drop,slope_drop\n";

    out << std::fixed << std::setprecision(6);
    for (const TelemetryRow& r : m_rows) {
        out << r.t        << ','
            << r.cm_x     << ',' << r.cm_vx << ','
            << r.cm_y     << ',' << r.cm_vy << ','
            << r.pelvis_x << ','
            << r.foot_left_x << ',' << r.foot_right_x << ','
            << r.foot_left_y << ',' << r.foot_right_y << ','
            << static_cast<int>(r.foot_left_on_ground) << ','
            << static_cast<int>(r.foot_right_on_ground) << ','
            << r.cm_target_y << ','
            << locoName(r.loco_state) << ','
            << r.ref_ground << ','
            << r.ref_slope << ','
            << r.h_ip << ','
            << r.cm_offset << ','
            << r.speed_drop << ','
            << r.slope_drop << '\n';
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
    for (const Assertion& a : m_asserts) {
        const bool pass = a.check(m_rows);
        report << (pass ? "PASS" : "FAIL") << "  " << a.name << '\n';
        if (!pass) all_pass = false;
    }
    return all_pass;
}
