#include "LocomotionController.h"

#include <cmath>
#include <algorithm>

static constexpr double PI = 3.14159265358979323846;

// Bornes de phases — Walk
static constexpr double W_DS1_END = 0.10; // [0.00, 0.10) DoubleSupport
static constexpr double W_LS_END  = 0.50; // [0.10, 0.50) LeftSupport
static constexpr double W_DS2_END = 0.60; // [0.50, 0.60) DoubleSupport
                                           // [0.60, 1.00) RightSupport

// Bornes de phases — Run
static constexpr double R_LS_END  = 0.35; // [0.00, 0.35) LeftSupport
static constexpr double R_FL1_END = 0.50; // [0.35, 0.50) Flight
static constexpr double R_RS_END  = 0.85; // [0.50, 0.85) RightSupport
                                           // [0.85, 1.00) Flight

// --------------------------------------------------------------------------

LocomotionController::LocomotionController()
    : mode_(LocomotionMode::Stand)
    , pending_mode_(LocomotionMode::Stand)
    , pending_mode_change_(false)
    , gait_phase_(GaitPhase::None)
    , normalized_cycle_(0.0)
    , gait_phase_time_(0.0)
    , mode_time_(0.0)
    , cycle_duration_s_(1.0)
    , desired_forward_speed_(0.0)
    , support_side_(SupportSide::Both)
    , active_swing_target_{}
    , left_foot_pos_{}
    , right_foot_pos_{}
    , pelvis_pos_{}
    , trunk_lean_(0.0)
    , initialized_(false)
{}

// --------------------------------------------------------------------------
// Helpers
// --------------------------------------------------------------------------

double LocomotionController::cycleDurationForMode(LocomotionMode m,
                                                   const TuningParams& t) const
{
    return (m == LocomotionMode::Walk) ? t.locomotion.walk_cycle_duration_s
                                       : t.locomotion.run_cycle_duration_s;
}

double LocomotionController::speedForMode(LocomotionMode m,
                                           const TuningParams& t) const
{
    const double leg = t.body.leg_length();
    if (m == LocomotionMode::Walk)
        return t.locomotion.walk_step_length_ratio * leg * 2.0
               / t.locomotion.walk_cycle_duration_s;
    if (m == LocomotionMode::Run)
        return t.locomotion.run_step_length_ratio * leg * 2.0
               / t.locomotion.run_cycle_duration_s;
    return 0.0;
}

GaitPhase LocomotionController::phaseFromCycle(double nc) const
{
    switch (mode_) {
    case LocomotionMode::Walk:
        if (nc < W_DS1_END) return GaitPhase::DoubleSupport;
        if (nc < W_LS_END)  return GaitPhase::LeftSupport;
        if (nc < W_DS2_END) return GaitPhase::DoubleSupport;
        return GaitPhase::RightSupport;

    case LocomotionMode::Run:
        if (nc < R_LS_END)  return GaitPhase::LeftSupport;
        if (nc < R_FL1_END) return GaitPhase::Flight;
        if (nc < R_RS_END)  return GaitPhase::RightSupport;
        return GaitPhase::Flight;

    default:
        return GaitPhase::None;
    }
}

double LocomotionController::phaseLocalU() const
{
    const double nc = normalized_cycle_;
    switch (mode_) {
    case LocomotionMode::Walk:
        if (nc < W_DS1_END) return nc / W_DS1_END;
        if (nc < W_LS_END)  return (nc - W_DS1_END) / (W_LS_END  - W_DS1_END);
        if (nc < W_DS2_END) return (nc - W_LS_END)  / (W_DS2_END - W_LS_END);
        return               (nc - W_DS2_END) / (1.0 - W_DS2_END);

    case LocomotionMode::Run:
        if (nc < R_LS_END)  return nc / R_LS_END;
        if (nc < R_FL1_END) return (nc - R_LS_END)  / (R_FL1_END - R_LS_END);
        if (nc < R_RS_END)  return (nc - R_FL1_END) / (R_RS_END  - R_FL1_END);
        return               (nc - R_RS_END) / (1.0 - R_RS_END);

    default:
        return 0.0;
    }
}

Vec2 LocomotionController::swingFootPos(const FootTarget& t, double u) const
{
    const double x = t.takeoff_position.x
                   + (t.target_position.x - t.takeoff_position.x) * u;
    const double y = t.lift_height * std::sin(PI * u);
    return {x, y};
}

void LocomotionController::snapFeetToStand(const TuningParams& tuning)
{
    const double half_sep = tuning.body.limb_rest_length * 0.15;
    left_foot_pos_  = {pelvis_pos_.x - half_sep, 0.0};
    right_foot_pos_ = {pelvis_pos_.x + half_sep, 0.0};
}

// --------------------------------------------------------------------------
// Phase transitions
// --------------------------------------------------------------------------

void LocomotionController::onPhaseEnter(GaitPhase phase, const TuningParams& tuning)
{
    const double leg = tuning.body.leg_length();

    // Calcul de la longueur de pas (formule spec, puis clamp)
    double step = desired_forward_speed_ * cycle_duration_s_ * 0.5;
    const double max_step = (mode_ == LocomotionMode::Walk)
        ? tuning.locomotion.walk_step_length_ratio * leg
        : tuning.locomotion.run_step_length_ratio  * leg;
    step = std::min(std::max(step, 0.0), max_step);

    const double lift = (mode_ == LocomotionMode::Walk)
        ? tuning.locomotion.walk_swing_lift_ratio * leg
        : tuning.locomotion.run_swing_lift_ratio  * leg;

    switch (phase) {
    case GaitPhase::LeftSupport:
        // Pied gauche = stance. Pied droit commence à swinguer.
        support_side_                     = SupportSide::Left;
        active_swing_target_.foot         = FootSide::Right;
        active_swing_target_.takeoff_position = right_foot_pos_;
        active_swing_target_.target_position  = {pelvis_pos_.x + step * 0.5, 0.0};
        active_swing_target_.lift_height      = lift;
        break;

    case GaitPhase::RightSupport:
        // Pied droit = stance. Pied gauche commence à swinguer.
        support_side_                     = SupportSide::Right;
        active_swing_target_.foot         = FootSide::Left;
        active_swing_target_.takeoff_position = left_foot_pos_;
        active_swing_target_.target_position  = {pelvis_pos_.x + step * 0.5, 0.0};
        active_swing_target_.lift_height      = lift;
        break;

    case GaitPhase::DoubleSupport:
        support_side_ = SupportSide::Both;
        break;

    case GaitPhase::Flight:
        // Snapper le pied qui vient de terminer son swing sur sa cible.
        support_side_ = SupportSide::None;
        if (normalized_cycle_ < 0.5)
            right_foot_pos_ = active_swing_target_.target_position; // après LeftSupport
        else
            left_foot_pos_  = active_swing_target_.target_position; // après RightSupport
        break;

    default:
        break;
    }
}

// --------------------------------------------------------------------------
// Mise à jour pieds et bassin
// --------------------------------------------------------------------------

void LocomotionController::updateSwingFoot()
{
    const double u = phaseLocalU();
    switch (gait_phase_) {
    case GaitPhase::LeftSupport:
        right_foot_pos_ = swingFootPos(active_swing_target_, u);
        break;
    case GaitPhase::RightSupport:
        left_foot_pos_ = swingFootPos(active_swing_target_, u);
        break;
    default:
        break; // Les pieds gardent leur dernière position
    }
}

void LocomotionController::updatePelvisHeight(const TuningParams& tuning)
{
    const double leg = tuning.body.leg_length();
    const double u   = phaseLocalU();
    const double nc  = normalized_cycle_;

    if (mode_ == LocomotionMode::Walk) {
        const double bob = tuning.locomotion.walk_pelvis_bob_ratio * leg;
        pelvis_pos_.y = leg - bob * std::cos(4.0 * PI * nc);
        trunk_lean_   = tuning.locomotion.walk_torso_lean_rad;
    } else { // Run
        const double compress    = tuning.locomotion.run_stance_compress_ratio * leg;
        const double flight_lift = tuning.locomotion.run_flight_lift_ratio     * leg;
        switch (gait_phase_) {
        case GaitPhase::LeftSupport:
        case GaitPhase::RightSupport:
            pelvis_pos_.y = leg - compress * std::sin(PI * u);
            break;
        case GaitPhase::Flight:
            pelvis_pos_.y = leg + flight_lift * std::sin(PI * u);
            break;
        default:
            pelvis_pos_.y = leg;
            break;
        }
        trunk_lean_ = tuning.locomotion.run_torso_lean_rad;
    }
}

// --------------------------------------------------------------------------
// Transitions de mode
// --------------------------------------------------------------------------

void LocomotionController::handleModeRequest(const IntentRequest& intent,
                                              const TuningParams&  tuning)
{
    const LocomotionMode req = intent.requested_mode;
    if (req == mode_ && !pending_mode_change_) return;

    if (mode_ == LocomotionMode::Stand && req != LocomotionMode::Stand) {
        // Stand → Walk/Run : immédiat
        mode_                  = req;
        normalized_cycle_      = 0.0;
        gait_phase_time_       = 0.0;
        mode_time_             = 0.0;
        cycle_duration_s_      = cycleDurationForMode(req, tuning);
        desired_forward_speed_ = speedForMode(req, tuning);
        pending_mode_change_   = false;
        gait_phase_            = phaseFromCycle(0.0);
        onPhaseEnter(gait_phase_, tuning);

    } else if (req == LocomotionMode::Stand) {
        // Walk/Run → Stand : immédiat
        mode_                  = LocomotionMode::Stand;
        gait_phase_            = GaitPhase::None;
        support_side_          = SupportSide::Both;
        normalized_cycle_      = 0.0;
        mode_time_             = 0.0;
        desired_forward_speed_ = 0.0;
        pending_mode_change_   = false;
        snapFeetToStand(tuning);

    } else if (req != mode_) {
        // Walk ↔ Run : différé à la prochaine frontière de cycle
        pending_mode_        = req;
        pending_mode_change_ = true;
    }
}

void LocomotionController::advanceCycle(double dt, const TuningParams& tuning)
{
    const double new_nc = normalized_cycle_ + dt / cycle_duration_s_;

    if (new_nc >= 1.0 && pending_mode_change_) {
        mode_                  = pending_mode_;
        pending_mode_change_   = false;
        cycle_duration_s_      = cycleDurationForMode(mode_, tuning);
        desired_forward_speed_ = speedForMode(mode_, tuning);
    }

    normalized_cycle_ = std::fmod(new_nc, 1.0);
    if (normalized_cycle_ < 0.0) normalized_cycle_ = 0.0;
}

// --------------------------------------------------------------------------
// Boucle principale
// --------------------------------------------------------------------------

void LocomotionController::update(double dt,
                                   const IntentRequest& intent,
                                   const TuningParams&  tuning)
{
    if (!initialized_) {
        pelvis_pos_     = {0.0, tuning.body.leg_length()};
        snapFeetToStand(tuning);
        initialized_    = true;
    }

    handleModeRequest(intent, tuning);
    mode_time_ += dt;

    if (mode_ == LocomotionMode::Stand) {
        pelvis_pos_.y = tuning.body.leg_length();
        trunk_lean_   = 0.0;
        return;
    }

    const GaitPhase prev_phase = gait_phase_;
    advanceCycle(dt, tuning);
    const GaitPhase new_phase = phaseFromCycle(normalized_cycle_);

    if (new_phase != prev_phase) {
        gait_phase_      = new_phase;
        gait_phase_time_ = 0.0;
        onPhaseEnter(new_phase, tuning);
    } else {
        gait_phase_time_ += dt;
    }

    pelvis_pos_.x += desired_forward_speed_ * dt;
    updatePelvisHeight(tuning);
    updateSwingFoot();
}

// --------------------------------------------------------------------------
// Accesseurs
// --------------------------------------------------------------------------

ProceduralPoseState LocomotionController::getPoseState() const
{
    ProceduralPoseState s;
    s.mode                 = mode_;
    s.gait_phase           = gait_phase_;
    s.gait_phase_time      = gait_phase_time_;
    s.normalized_cycle     = normalized_cycle_;
    s.mode_time            = mode_time_;
    s.support_side         = support_side_;
    s.forward_speed        = desired_forward_speed_;
    s.cycle_duration_s     = cycle_duration_s_;
    s.active_swing_target  = active_swing_target_;
    return s;
}

ProceduralCMState LocomotionController::getCMState() const
{
    ProceduralCMState s;
    s.target_position      = pelvis_pos_;
    s.target_velocity      = {desired_forward_speed_, 0.0};
    s.operating_height     = pelvis_pos_.y;
    s.pelvis_offset_target = pelvis_pos_.y;
    s.trunk_lean_target    = trunk_lean_;
    return s;
}

Vec2 LocomotionController::getLeftFootPos()  const { return left_foot_pos_;  }
Vec2 LocomotionController::getRightFootPos() const { return right_foot_pos_; }
