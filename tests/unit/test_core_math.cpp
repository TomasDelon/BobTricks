#include "tests/TestSupport.h"

#include <cmath>

#include "config/AppConfig.h"
#include "core/character/CharacterState.h"
#include "core/character/ArmController.h"
#include "core/character/HeadController.h"
#include "core/math/Bezier.h"
#include "core/math/StrokePath.h"
#include "core/locomotion/BalanceComputer.h"
#include "core/locomotion/LegIK.h"
#include "core/physics/Geometry.h"
#include "core/terrain/Terrain.h"

int main()
{
    TestSuite suite("core_unit_tests");

    {
        const BezierQuadratic q{{0.0, 0.0}, {1.0, 2.0}, {2.0, 0.0}};
        TEST_EXPECT_NEAR(suite, q.eval(0.0).x, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, q.eval(0.0).y, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, q.eval(1.0).x, 2.0, 1e-12);
        TEST_EXPECT_NEAR(suite, q.eval(1.0).y, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, q.eval(0.5).x, 1.0, 1e-12);
        TEST_EXPECT_NEAR(suite, q.eval(0.5).y, 1.0, 1e-12);
    }

    {
        const BezierCubic c{{0.0, 0.0}, {1.0, 0.0}, {2.0, 1.0}, {3.0, 1.0}};
        TEST_EXPECT_NEAR(suite, c.eval(0.0).x, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, c.eval(1.0).x, 3.0, 1e-12);
        TEST_EXPECT_NEAR(suite, c.eval(1.0).y, 1.0, 1e-12);
        TEST_EXPECT_NEAR(suite, c.tangent(0.0).x, 3.0, 1e-12);
        TEST_EXPECT_NEAR(suite, c.tangent(0.0).y, 0.0, 1e-12);
    }

    {
        StrokePath path;
        path.moveTo({0.0, 0.0});
        path.lineTo({1.0, 0.0});
        path.quadTo({2.0, 1.0}, {3.0, 0.0});
        const std::vector<Vec2> pts = path.flatten(8);
        TEST_EXPECT(suite, pts.size() >= 10);
        TEST_EXPECT_NEAR(suite, pts.front().x, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, pts.front().y, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, pts.back().x, 3.0, 1e-12);
        TEST_EXPECT_NEAR(suite, pts.back().y, 0.0, 1e-12);
    }

    {
        const double L = 0.36;
        const double d_pref = 0.90;
        const double ratio = 0.75;
        const double expected = std::sqrt(4.0 * L * L - std::pow(0.5 * d_pref * L, 2.0))
                              + ratio * L;
        TEST_EXPECT_NEAR(suite, computeNominalY(L, d_pref, ratio), expected, 1e-9);
    }

    {
        const double low = computeNominalY(0.36, 0.90, 0.60);
        const double high = computeNominalY(0.36, 0.90, 0.85);
        TEST_EXPECT(suite, high > low);
    }

    {
        const Vec2 pelvis{0.0, 0.9};
        const Vec2 foot{0.3, 0.1};
        const double L = 0.5;
        const LegIKResult ik = computeKnee(pelvis, foot, L, 1.0);

        TEST_EXPECT_NEAR(suite, (ik.knee - pelvis).length(), L, 1e-6);
        TEST_EXPECT_NEAR(suite, (ik.knee - ik.foot_eff).length(), L, 1e-6);
        TEST_EXPECT_NEAR(suite, ik.foot_eff.x, foot.x, 1e-9);
        TEST_EXPECT_NEAR(suite, ik.foot_eff.y, foot.y, 1e-9);
    }

    {
        const Vec2 pelvis{0.0, 0.9};
        const Vec2 far_foot{2.0, 0.0};
        const double L = 0.5;
        const LegIKResult ik = computeKnee(pelvis, far_foot, L, -1.0);

        TEST_EXPECT(suite, (ik.foot_eff - pelvis).length() <= 2.0 * L);
        TEST_EXPECT_NEAR(suite, (ik.knee - pelvis).length(), L, 1e-6);
        TEST_EXPECT_NEAR(suite, (ik.knee - ik.foot_eff).length(), L, 1e-6);
        TEST_EXPECT(suite, ik.knee.x < pelvis.x + 0.5 * (ik.foot_eff.x - pelvis.x));
    }

    {
        CharacterState ch;
        CharacterConfig char_cfg;
        CharacterReconstructionConfig recon_cfg;
        CMState cm;
        cm.position = {0.0, 1.2};
        cm.velocity = {1.5, 0.0};

        for (int i = 0; i < 120; ++i)
            updateCharacterState(ch, cm, char_cfg, recon_cfg, true, false, 1.0 / 60.0, 0.0);

        const double L = char_cfg.body_height_m / 5.0;
        const Vec2 lower = ch.torso_center - ch.pelvis;
        const Vec2 upper = ch.torso_top - ch.torso_center;
        TEST_EXPECT_NEAR(suite, lower.length(), L, 1e-6);
        TEST_EXPECT_NEAR(suite, upper.length(), L, 1e-6);
        TEST_EXPECT(suite, upper.x > lower.x + 1e-4);
    }

    {
        CharacterState ch;
        CharacterConfig char_cfg;
        HeadConfig head_cfg;
        ch.facing = 1.0;
        ch.torso_center = {0.0, 1.0};
        ch.torso_top = {0.0, 2.0};

        updateHeadState(ch, char_cfg, head_cfg, std::nullopt, 1.0 / 60.0);

        TEST_EXPECT(suite, ch.head_center.y > ch.torso_top.y);
        TEST_EXPECT_NEAR(suite, ch.head_radius, 0.5 * (char_cfg.body_height_m / 5.0), 1e-9);
        TEST_EXPECT_NEAR(suite, ch.head_center.y - ch.torso_top.y, ch.head_radius, 1e-9);
        TEST_EXPECT_NEAR(suite, ch.head_tilt, 0.0, 1e-6);
        TEST_EXPECT_NEAR(suite, ch.eye_left.x, ch.eye_right.x, 1e-12);
        TEST_EXPECT_NEAR(suite, ch.eye_left.y, ch.eye_right.y, 1e-12);
        TEST_EXPECT_NEAR(suite, ch.eye_left.x - ch.head_center.x, 0.5 * ch.head_radius, 1e-9);
        TEST_EXPECT_NEAR(suite, ch.eye_left.y - ch.head_center.y, 0.25 * ch.head_radius, 1e-9);
    }

    {
        CharacterState ch;
        CharacterConfig char_cfg;
        HeadConfig head_cfg;
        ch.facing = 1.0;
        ch.torso_center = {0.0, 1.0};
        ch.torso_top = {0.0, 2.0};

        updateHeadState(ch, char_cfg, head_cfg, Vec2{0.0, 10.0}, 1.0);

        TEST_EXPECT(suite, ch.head_tilt > 0.0);
        TEST_EXPECT(suite, std::abs(ch.head_tilt)
                           <= head_cfg.max_tilt_deg * 3.14159265358979323846 / 180.0 + 1e-6);
    }

    {
        CharacterState ch;
        CharacterConfig char_cfg;
        HeadConfig head_cfg;
        ch.facing = 1.0;
        ch.torso_center = {0.0, 1.0};
        ch.torso_top = {0.0, 2.0};

        for (int i = 0; i < 300; ++i)
            updateHeadState(ch, char_cfg, head_cfg, Vec2{0.0, 10.0}, 1.0 / 60.0);

        const double max_tilt = head_cfg.max_tilt_deg * 3.14159265358979323846 / 180.0;
        TEST_EXPECT(suite, ch.head_tilt > 0.0);
        TEST_EXPECT_NEAR(suite, ch.head_tilt, max_tilt, 0.03);
    }

    {
        CharacterState ch;
        ch.head_center = {1.0, 2.0};
        ch.eye_left = {0.9, 2.1};
        ch.eye_right = {1.1, 2.1};
        ch.head_radius = 0.4;
        ch.head_tilt = 0.3;

        resetHeadState(ch);

        TEST_EXPECT_NEAR(suite, ch.head_center.x, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, ch.head_center.y, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, ch.eye_left.x, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, ch.eye_left.y, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, ch.eye_right.x, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, ch.eye_right.y, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, ch.head_radius, 0.0, 1e-12);
        TEST_EXPECT_NEAR(suite, ch.head_tilt, 0.0, 1e-12);
    }

    {
        ArmPose pose;
        const bool reached = solveTwoBoneArm({0.6, -0.8}, 0.6, 0.6, {0.0, 0.0}, {-1.0, -0.2}, std::nullopt, pose);

        TEST_EXPECT(suite, reached);
        TEST_EXPECT_NEAR(suite, (pose.elbow - pose.shoulder).length(), 0.6, 1e-6);
        TEST_EXPECT_NEAR(suite, (pose.hand - pose.elbow).length(), 0.6, 1e-6);
        TEST_EXPECT_NEAR(suite, pose.hand.x, 0.6, 1e-6);
        TEST_EXPECT_NEAR(suite, pose.hand.y, -0.8, 1e-6);
    }

    {
        ArmPose pose;
        const bool reached = solveTwoBoneArm({2.0, 0.0}, 0.6, 0.6, {0.0, 0.0}, {-1.0, -0.2}, std::nullopt, pose);

        TEST_EXPECT(suite, !reached);
        TEST_EXPECT_NEAR(suite, (pose.hand - pose.shoulder).length(), 1.2, 1e-6);
        TEST_EXPECT_NEAR(suite, (pose.elbow - pose.shoulder).length(), 0.6, 1e-6);
    }

    {
        ArmPose pose_a;
        ArmPose pose_b;
        const Vec2 bend_pref{-1.0, -0.2};
        solveTwoBoneArm({0.45, -0.85}, 0.6, 0.6, {0.0, 0.0}, bend_pref, std::nullopt, pose_a);
        solveTwoBoneArm({0.15, -1.05}, 0.6, 0.6, {0.0, 0.0}, bend_pref, std::nullopt, pose_b);

        const double side_a = pose_a.elbow.x - pose_a.shoulder.x;
        const double side_b = pose_b.elbow.x - pose_b.shoulder.x;
        TEST_EXPECT(suite, side_a * side_b >= 0.0);
    }

    {
        ArmPose pose_a;
        ArmPose pose_b;
        solveTwoBoneArm({0.55, -0.75}, 0.6, 0.6, {0.0, 0.0}, {-1.0, -0.2}, std::nullopt, pose_a);
        solveTwoBoneArm({0.35, -0.82}, 0.6, 0.6, {0.0, 0.0}, {-1.0, -0.2}, pose_a.elbow, pose_b);
        TEST_EXPECT(suite, (pose_b.elbow - pose_a.elbow).length() < 0.5);
    }

    {
        CharacterState ch;
        CharacterConfig char_cfg;
        PhysicsConfig physics_cfg;
        WalkConfig walk_cfg;
        ArmConfig arm_cfg;
        CMState cm;
        cm.velocity = {0.0, 0.0};
        ch.facing = 1.0;
        ch.torso_center = {0.0, 1.0};
        ch.torso_top = {0.0, 1.72};

        updateArmState(ch, cm, char_cfg, physics_cfg, walk_cfg, arm_cfg, 0.0,
                       std::nullopt, std::nullopt, 1.0 / 60.0);

        TEST_EXPECT_NEAR(suite, ch.shoulder_left.x, ch.torso_top.x, 1e-9);
        TEST_EXPECT_NEAR(suite, ch.shoulder_right.x, ch.torso_top.x, 1e-9);
        TEST_EXPECT_NEAR(suite, ch.shoulder_left.y, ch.torso_top.y, 1e-9);
        TEST_EXPECT_NEAR(suite, ch.shoulder_right.y, ch.torso_top.y, 1e-9);
        TEST_EXPECT(suite, ch.elbow_left.y < ch.shoulder_left.y);
        TEST_EXPECT(suite, ch.elbow_right.y < ch.shoulder_right.y);
        TEST_EXPECT(suite, ch.hand_left.y < ch.shoulder_left.y);
        TEST_EXPECT(suite, ch.hand_right.y < ch.shoulder_right.y);
        TEST_EXPECT(suite, (ch.hand_left - ch.torso_top).length() <= 2.0 * (char_cfg.body_height_m / 5.0) + 1e-6);
        TEST_EXPECT(suite, (ch.hand_right - ch.torso_top).length() <= 2.0 * (char_cfg.body_height_m / 5.0) + 1e-6);
    }

    {
        CharacterState ch;
        CharacterState ch_dragged;
        CharacterConfig char_cfg;
        PhysicsConfig physics_cfg;
        WalkConfig walk_cfg;
        ArmConfig arm_cfg;
        CMState cm;
        ch.facing = 1.0;
        ch.torso_center = {0.0, 1.0};
        ch.torso_top = {0.0, 1.72};
        ch_dragged = ch;
        const Vec2 drag_target{0.20, 0.90};

        updateArmState(ch, cm, char_cfg, physics_cfg, walk_cfg, arm_cfg, 1.0,
                       std::nullopt, std::nullopt, 1.0 / 60.0);
        updateArmState(ch_dragged, cm, char_cfg, physics_cfg, walk_cfg, arm_cfg, 1.0,
                       drag_target, std::nullopt, 1.0 / 60.0);

        const double auto_dist = (ch.hand_left - drag_target).length();
        const double dragged_dist = (ch_dragged.hand_left - drag_target).length();
        TEST_EXPECT(suite, dragged_dist < auto_dist);
        TEST_EXPECT(suite, dragged_dist < 0.25);
    }

    {
        CharacterState ch0;
        CharacterState ch1;
        CharacterConfig char_cfg;
        PhysicsConfig physics_cfg;
        WalkConfig walk_cfg;
        ArmConfig arm_cfg;
        CMState cm;
        ch0.facing = 1.0;
        ch0.torso_center = {0.0, 1.0};
        ch0.torso_top = {0.0, 1.72};
        ch0.foot_left.swinging = true;
        ch0.foot_left.swing_t = 0.0;
        ch1 = ch0;
        ch1.foot_left.swing_t = 1.0;

        updateArmState(ch0, cm, char_cfg, physics_cfg, walk_cfg, arm_cfg, 1.0,
                       std::nullopt, std::nullopt, 1.0 / 60.0);
        updateArmState(ch1, cm, char_cfg, physics_cfg, walk_cfg, arm_cfg, 1.0,
                       std::nullopt, std::nullopt, 1.0 / 60.0);

        TEST_EXPECT(suite, (ch0.hand_left - ch1.hand_left).length() > 1e-3);
        TEST_EXPECT(suite, (ch0.hand_right - ch1.hand_right).length() > 1e-3);
    }

    {
        CharacterState slow;
        CharacterState fast;
        CharacterConfig char_cfg;
        PhysicsConfig physics_cfg;
        WalkConfig walk_cfg;
        ArmConfig arm_cfg;
        CMState cm_slow;
        CMState cm_fast;
        slow.facing = 1.0;
        slow.torso_center = {0.0, 1.0};
        slow.torso_top = {0.0, 1.72};
        slow.arm_phase = 1.0;
        slow.arm_pose_initialized = true;
        fast = slow;
        cm_fast.velocity = {physics_cfg.walk_max_speed, 0.0};

        updateArmState(slow, cm_slow, char_cfg, physics_cfg, walk_cfg, arm_cfg, 0.0,
                       std::nullopt, std::nullopt, 0.0);
        updateArmState(fast, cm_fast, char_cfg, physics_cfg, walk_cfg, arm_cfg, 0.0,
                       std::nullopt, std::nullopt, 0.0);

        const double mid_angle_deg = 0.5 * (arm_cfg.walk_front_hand_start_deg + arm_cfg.walk_front_hand_end_deg);
        const double a = mid_angle_deg * 3.14159265358979323846 / 180.0;
        const double L = char_cfg.body_height_m / 5.0;
        const double radius = std::max(0.05 * L, 2.0 * L - arm_cfg.walk_hand_reach_reduction_L * L);
        const Vec2 body_up{0.0, 1.0};
        const Vec2 body_right{1.0, 0.0};
        const Vec2 mid_target = slow.torso_top
                              + body_right * (std::cos(a) * radius)
                              + body_up    * (std::sin(a) * radius);
        const double slow_deviation = (slow.hand_right - mid_target).length();
        const double fast_deviation = (fast.hand_right - mid_target).length();
        TEST_EXPECT(suite, fast_deviation > slow_deviation + 1e-3);
    }

    // ── computeBalanceState ───────────────────────────────────────────────────

    {
        // omega0 is positive for any valid grounded support
        CMState cm;
        cm.position = {0.0, 1.0};
        cm.velocity = {0.0, 0.0};
        SupportState support;
        support.x_left  = -0.2;  support.y_left  = 0.0;
        support.x_right =  0.2;  support.y_right = 0.0;
        support.left_planted = support.right_planted = true;
        CharacterConfig char_cfg;
        PhysicsConfig phys_cfg;

        const BalanceState bal = computeBalanceState(cm, support, char_cfg, phys_cfg);
        TEST_EXPECT(suite, bal.omega0 > 0.0);
    }

    {
        // xcom is ahead of CM when moving forward
        CMState cm;
        cm.position = {0.0, 1.0};
        cm.velocity = {1.5, 0.0};
        SupportState support;
        support.x_left  = -0.4;  support.y_left  = 0.0;
        support.x_right =  0.4;  support.y_right = 0.0;
        support.left_planted = support.right_planted = true;
        CharacterConfig char_cfg;
        PhysicsConfig phys_cfg;

        const BalanceState bal = computeBalanceState(cm, support, char_cfg, phys_cfg);
        TEST_EXPECT(suite, bal.xcom > cm.position.x);
    }

    {
        // MoS > 0 when CM is stationary inside support polygon
        CMState cm;
        cm.position = {0.0, 1.0};
        cm.velocity = {0.0, 0.0};
        SupportState support;
        support.x_left  = -0.2;  support.y_left  = 0.0;
        support.x_right =  0.2;  support.y_right = 0.0;
        support.left_planted = support.right_planted = true;
        CharacterConfig char_cfg;
        PhysicsConfig phys_cfg;

        const BalanceState bal = computeBalanceState(cm, support, char_cfg, phys_cfg);
        TEST_EXPECT(suite, bal.mos > 0.0);
    }

    {
        // Higher velocity pushes xcom outside the support base → MoS decreases
        SupportState support;
        support.x_left  = -0.2;  support.y_left  = 0.0;
        support.x_right =  0.2;  support.y_right = 0.0;
        support.left_planted = support.right_planted = true;
        CharacterConfig char_cfg;
        PhysicsConfig phys_cfg;

        CMState cm_slow, cm_fast;
        cm_slow.position = cm_fast.position = {0.0, 1.0};
        cm_slow.velocity = {0.3, 0.0};
        cm_fast.velocity = {1.5, 0.0};

        const BalanceState bal_slow = computeBalanceState(cm_slow, support, char_cfg, phys_cfg);
        const BalanceState bal_fast = computeBalanceState(cm_fast, support, char_cfg, phys_cfg);
        TEST_EXPECT(suite, bal_fast.mos < bal_slow.mos);
    }

    // ── Terrain ───────────────────────────────────────────────────────────────

    {
        // Flat terrain (enabled=false): height=0, vertical normal, zero slope
        TerrainConfig cfg;
        cfg.enabled = false;
        Terrain terrain(cfg);
        terrain.generate();

        TEST_EXPECT_NEAR(suite, terrain.height_at(0.0),   0.0, 1e-9);
        TEST_EXPECT_NEAR(suite, terrain.height_at(10.0),  0.0, 1e-9);
        TEST_EXPECT_NEAR(suite, terrain.height_at(-5.0),  0.0, 1e-9);
        TEST_EXPECT_NEAR(suite, terrain.normal_at(0.0).x, 0.0, 1e-6);
        TEST_EXPECT_NEAR(suite, terrain.normal_at(0.0).y, 1.0, 1e-6);
        TEST_EXPECT_NEAR(suite, terrain.slope_at(0.0),    0.0, 1e-9);
    }

    {
        // Normal is always unit length regardless of slope
        TerrainConfig cfg;
        cfg.enabled = true;
        cfg.seed    = 42;
        Terrain terrain(cfg);
        terrain.generate();

        for (double x : {-10.0, 0.0, 5.0, 15.0, 30.0}) {
            const Vec2 n = terrain.normal_at(x);
            TEST_EXPECT_NEAR(suite, n.x * n.x + n.y * n.y, 1.0, 1e-9);
        }
    }

    {
        // Same seed produces identical terrain
        TerrainConfig cfg;
        cfg.enabled = true;
        cfg.seed    = 42;
        Terrain t1(cfg);
        t1.generate();
        Terrain t2(cfg);
        t2.generate();

        TEST_EXPECT_NEAR(suite, t1.height_at(5.0),  t2.height_at(5.0),  1e-12);
        TEST_EXPECT_NEAR(suite, t1.height_at(20.0), t2.height_at(20.0), 1e-12);
    }

    {
        // Different seeds produce different terrain
        TerrainConfig cfg;
        cfg.enabled = true;
        cfg.seed    = 42;
        Terrain t1(cfg);
        t1.generate();
        cfg.seed = 99;
        Terrain t2(cfg);
        t2.generate();

        TEST_EXPECT(suite, std::fabs(t1.height_at(20.0) - t2.height_at(20.0)) > 1e-3);
    }

    return suite.finish();
}
