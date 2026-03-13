#pragma once

#include <array>
#include "../state/CharacterState.h"
#include "../state/CMState.h"
#include "../state/ProceduralPoseState.h"
#include "../state/TuningParams.h"

/// @brief Reconstruit les positions de tous les nœuds du stickman à partir des
/// cibles procédurales produites par LocomotionController.
///
/// Stratégie de reconstruction (mi-parcours) :
///  - ancrage bassin = target_position du ProceduralCMState  (= torso_bottom)
///  - chaîne tronc   = 3 segments dans la direction du tronc (trunk_lean)
///  - jambes         = IK analytique 2 segments, genou vers l'avant
///  - bras           = balancement procédural opposé à la jambe correspondante
class ProceduralAnimator {
public:
    ProceduralAnimator() = default;

    /// @brief Met à jour les positions de tous les nœuds.
    /// @param cm         état CM procédural (position bassin, lean tronc)
    /// @param left_foot  position monde de la cheville gauche
    /// @param right_foot position monde de la cheville droite
    /// @param pose       état procédural (mode, cycle, pour le balancement des bras)
    /// @param tuning     paramètres géométriques
    void update(const ProceduralCMState&   cm,
                Vec2                       left_foot,
                Vec2                       right_foot,
                const ProceduralPoseState& pose,
                const TuningParams&        tuning);

    /// @brief Retourne les positions courantes de tous les nœuds.
    const std::array<Vec2, NODE_COUNT>& getNodePositions() const;

private:
    std::array<Vec2, NODE_COUNT> nodes_ = {};

    /// @brief IK analytique 2 segments : retourne la position du genou.
    /// Le genou est toujours placé du côté qui protrude vers l'avant (+x).
    Vec2 solveKnee(Vec2 hip, Vec2 ankle, double L) const;

    Vec2& pos(NodeId n) { return nodes_[static_cast<int>(n)]; }
};
