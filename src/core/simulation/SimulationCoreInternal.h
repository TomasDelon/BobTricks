#pragma once

#include "core/simulation/SimulationCore.h"

namespace simcore_detail {

struct HeightTargetState {
    double h_ip = 0.0;
    double speed_drop = 0.0;
    double slope_drop = 0.0;
    double y_tgt = 0.0;
};

struct LandingFootPlan {
    Vec2 left  = {0.0, 0.0};
    Vec2 right = {0.0, 0.0};
};

struct StepTriggerEval {
    double front_x = 0.0;
    double rear_x = 0.0;
    double d_rear = 0.0;
    bool   step_left_xcom = false;
    bool   xcom_trigger = false;
    bool   rear_trigger = false;
};

Vec2 reconstructPelvis(const CMState& cm,
                       const CharacterConfig& cfg,
                       const CharacterReconstructionConfig& rcfg);

double smooth01(double x);

LandingFootPlan planLandingFeet(const CharacterState& ch,
                                const StandingConfig& stand_cfg,
                                const Terrain& terrain,
                                const Vec2& pelvis,
                                double reach_radius,
                                double L);

double predictLandingTime(const CMState& cm,
                          const CharacterConfig& char_cfg,
                          const CharacterReconstructionConfig& recon_cfg,
                          const Terrain& terrain,
                          double g,
                          double L);

void beginJumpPreload(CharacterState& ch,
                      bool run_mode,
                      double speed_abs,
                      double L,
                      const CMState& cm,
                      const JumpConfig& jump_cfg);

void beginAirborneLandingProtocol(CharacterState& ch,
                                  const AppConfig& config,
                                  const Terrain& terrain,
                                  const CharacterReconstructionConfig& recon_cfg,
                                  const CMState& cm,
                                  double g,
                                  double L);

void prepareJumpFoot(FootState& foot);

void updateJumpLandingTargets(CharacterState& ch,
                              const AppConfig& config,
                              const Terrain& terrain,
                              const CharacterReconstructionConfig& recon_cfg,
                              const CMState& cm,
                              double g,
                              double L);

void updateJumpFeetInFlight(CharacterState& ch,
                            const Vec2& pelvis,
                            double progress);

void updateJumpFootInFlight(FootState& foot,
                            const Vec2& start,
                            const Vec2& target,
                            const Vec2& pelvis,
                            double smooth_progress,
                            double tuck,
                            double tuck_weight);

RunTimingTargets computeRunTimingTargets(const RunConfig& run_cfg,
                                         double speed_abs,
                                         double max_speed,
                                         double L);

double computeRunLandingX(const RunConfig& run_cfg,
                          const CharacterState& ch,
                          const FootState& stance_foot,
                          const Vec2& pelvis,
                          double velocity_x,
                          double reach_radius,
                          double L,
                          const RunTimingTargets& timing,
                          double ref_slope);

double computeStepLandingX(const WalkConfig& walk_cfg,
                           const CharacterState& ch,
                           const FootState& stance_foot,
                           const Vec2& pelvis,
                           double xi,
                           double L,
                           double reach_radius,
                           double ref_slope);

void refreshSwingArcProfile(FootState& foot,
                            const Terrain& terrain,
                            double L,
                            const StepConfig& step_cfg,
                            const WalkConfig& walk_cfg,
                            double speed_abs,
                            double walk_max_speed);

double estimateSwingRemainingTime(const FootState& foot,
                                  const WalkConfig& walk_cfg);

double retargetLandingRecoveryX(double target_x,
                                const CharacterState& ch,
                                const FootState& stance_foot,
                                const Vec2& pelvis,
                                double future_cm_x,
                                double reach_radius,
                                double L,
                                double ref_slope,
                                const WalkConfig& walk_cfg,
                                double recovery_gain);

void retargetSwingIfLate(FootState& swing_foot,
                         const FootState& stance_foot,
                         const CharacterState& ch,
                         const Terrain& terrain,
                         const Vec2& pelvis,
                         const WalkConfig& walk_cfg,
                         const StepConfig& step_cfg,
                         double cm_x,
                         double cm_vx,
                         double reach_radius,
                         double L,
                         double ref_slope,
                         double recovery_gain,
                         double speed_abs,
                         double walk_max_speed);

void beginSwingStep(FootState& step_foot,
                    FootState& stance_foot,
                    CharacterState& ch,
                    double tx,
                    const Terrain& terrain,
                    bool corrective_followthrough,
                    double L,
                    const StepConfig& step_cfg,
                    const WalkConfig& walk_cfg,
                    double speed_abs,
                    double walk_max_speed);

void releaseFeetAirborne(CharacterState& ch);
void releaseFootAirborne(FootState& foot);
void plantFootOnTerrain(FootState& foot, const Terrain& terrain, double x);
void plantFootAtTarget(FootState& foot, const Vec2& target);

void bootstrapFeetOnLanding(CharacterState& ch,
                            const StandingConfig& stand_cfg,
                            const Terrain& terrain,
                            const Vec2& pelvis,
                            double reach_radius,
                            double L);

void initializeFeetUnderPelvis(CharacterState& ch,
                               const Terrain& terrain,
                               const Vec2& pelvis,
                               double L,
                               double d_pref);

void applyPinnedFootPositions(CharacterState& ch);

void applyDraggedFootPositions(CharacterState& ch,
                               const InputFrame& input);

void advanceSwingFoot(FootState& foot,
                      const Terrain& terrain,
                      const Vec2& pelvis,
                      double dt,
                      double reach_radius,
                      const WalkConfig& walk_cfg);

void applyFootConstraints(CharacterState& ch,
                          const Terrain& terrain,
                          const Vec2& pelvis,
                          double reach_radius);

StepTriggerEval evaluateStepTriggers(const CharacterState& ch,
                                     double eff_lx,
                                     double eff_rx,
                                     double xi,
                                     double velocity_x,
                                     double eps_step,
                                     const Vec2& pelvis,
                                     double L,
                                     double d_rear_max);

void cacheXCoMState(SimState& state,
                    const CharacterState& ch,
                    const CMState& cm,
                    const WalkConfig& walk_cfg,
                    double eff_lx,
                    double eff_rx,
                    double xi,
                    double L);

void updateDownhillCrouch(CharacterState& ch,
                          const WalkConfig& walk_cfg,
                          const PhysicsConfig& physics_cfg,
                          double ref_slope,
                          double velocity_x,
                          double max_spd,
                          double dt);

bool isSlideActive(const FootState& foot, double velocity_x);

HeightTargetState computeHeightTargetState(const CharacterState& ch,
                                           const CMState& cm,
                                           const WalkConfig& walk_cfg,
                                           const PhysicsConfig& physics_cfg,
                                           double ref_ground,
                                           double ref_slope,
                                           double h_nominal,
                                           double cm_pelvis_ratio,
                                           double L);

double computeHorizontalAcceleration(const CharacterState& ch,
                                     const CMState& cm,
                                     const PhysicsConfig& physics_cfg,
                                     double input_dir,
                                     double sin_t,
                                     double cos_t,
                                     double g,
                                     bool airborne_pre);

void integrateHorizontalMotion(CMState& cm,
                               const CharacterState& ch,
                               const PhysicsConfig& physics_cfg,
                               double input_dir,
                               double sin_t,
                               double cos_t,
                               double g,
                               bool airborne_pre,
                               double max_spd,
                               double dt,
                               Vec2& accel);

bool integrateVerticalMotion(CMState& cm,
                             Vec2& accel,
                             const Terrain& terrain,
                             const CharacterConfig& char_cfg,
                             const CharacterReconstructionConfig& recon_cfg,
                             const PhysicsConfig& physics_cfg,
                             double y_tgt,
                             double ref_ground,
                             double L,
                             double g,
                             double dt);

void refreshGroundContact(FootState& foot, const Terrain& terrain);
void applyGroundConstraint(FootState& foot, const Terrain& terrain);

void unpinFootIfLifted(FootState& foot, bool was_grounded);

double invertedPendulumFootY(const FootState& foot,
                             const CMState& cm,
                             const WalkConfig& walk_cfg,
                             double R_bob,
                             double bob_max);

void blendWalkRunConfig(WalkConfig& eff_walk,
                        StepConfig& eff_step,
                        PhysicsConfig& eff_physics,
                        const AppConfig& cfg,
                        double rb);

} // namespace simcore_detail
