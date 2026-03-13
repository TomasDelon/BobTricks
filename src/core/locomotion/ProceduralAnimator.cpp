#include "ProceduralAnimator.h"

#include <cmath>
#include <algorithm>

static constexpr double PI = 3.14159265358979323846;

// --------------------------------------------------------------------------
// IK analytique 2 segments
// --------------------------------------------------------------------------

Vec2 ProceduralAnimator::solveKnee(Vec2 hip, Vec2 ankle, double L) const
{
    Vec2 d    = ankle - hip;
    double dist = d.length();
    if (dist < 0.001) dist = 0.001;

    // Normalise la direction hanche → cheville
    Vec2 dn   = d / dist;
    Vec2 perp = {-dn.y, dn.x};  // perpendiculaire (rotation 90° antihoraire)

    // Clamp : on ne peut pas dépasser 2L
    double clamped = std::min(dist, 2.0 * L * 0.9999);

    // Règle du cosinus dans le triangle hanche–genou–cheville (segments égaux = L)
    // cos(angle à la hanche) = clamped / (2L)
    double cos_a = clamped / (2.0 * L);
    double sin_a = std::sqrt(std::max(0.0, 1.0 - cos_a * cos_a));

    // Les deux positions candidates pour le genou
    Vec2 k1 = hip + (dn * cos_a + perp *  sin_a) * L;
    Vec2 k2 = hip + (dn * cos_a - perp *  sin_a) * L;

    // Choisir celle qui protrude vers l'avant (+x = direction de marche)
    return (k1.x >= k2.x) ? k1 : k2;
}

// --------------------------------------------------------------------------
// Mise à jour principale
// --------------------------------------------------------------------------

void ProceduralAnimator::update(const ProceduralCMState&   cm,
                                Vec2                       left_foot,
                                Vec2                       right_foot,
                                const ProceduralPoseState& pose,
                                const TuningParams&        tuning)
{
    const double L           = tuning.body.limb_rest_length;
    const double trunk_lean  = cm.trunk_lean_target;

    // Direction du tronc dans l'espace monde (lean vers l'avant = +x)
    const Vec2 trunk_dir = {std::sin(trunk_lean), std::cos(trunk_lean)};

    // --- Tronc ---
    pos(NodeId::TorsoBottom) = cm.target_position;
    pos(NodeId::TorsoCenter) = pos(NodeId::TorsoBottom) + trunk_dir * L;
    pos(NodeId::TorsoTop)    = pos(NodeId::TorsoCenter) + trunk_dir * L;
    pos(NodeId::HeadTop)     = pos(NodeId::TorsoTop)    + trunk_dir * L;

    // --- Jambes (IK 2 segments) ---
    pos(NodeId::AnkleLeft)  = left_foot;
    pos(NodeId::AnkleRight) = right_foot;
    pos(NodeId::KneeLeft)   = solveKnee(pos(NodeId::TorsoBottom), left_foot,  L);
    pos(NodeId::KneeRight)  = solveKnee(pos(NodeId::TorsoBottom), right_foot, L);

    // --- Bras (balancement procédural opposé à la jambe correspondante) ---
    const double nc = pose.normalized_cycle;

    double arm_swing = 0.0;
    if (pose.mode == LocomotionMode::Walk)
        arm_swing = tuning.locomotion.walk_arm_swing_rad;
    else if (pose.mode == LocomotionMode::Run)
        arm_swing = tuning.locomotion.run_arm_swing_rad;

    // Bras droit en phase avec jambe gauche (opposé à la jambe droite)
    const double angle_r =  arm_swing * std::sin(2.0 * PI * nc);
    const double angle_l = -arm_swing * std::sin(2.0 * PI * nc);

    // Direction des bras : pendule vers le bas depuis torso_top
    // angle = 0 → (0, -1) = droit vers le bas
    const Vec2 dir_r = { std::sin(angle_r), -std::cos(angle_r) };
    const Vec2 dir_l = { std::sin(angle_l), -std::cos(angle_l) };

    pos(NodeId::ElbowRight) = pos(NodeId::TorsoTop) + dir_r * L;
    pos(NodeId::WristRight) = pos(NodeId::ElbowRight) + dir_r * L;
    pos(NodeId::ElbowLeft)  = pos(NodeId::TorsoTop) + dir_l * L;
    pos(NodeId::WristLeft)  = pos(NodeId::ElbowLeft)  + dir_l * L;
}

// --------------------------------------------------------------------------

const std::array<Vec2, NODE_COUNT>& ProceduralAnimator::getNodePositions() const
{
    return nodes_;
}
