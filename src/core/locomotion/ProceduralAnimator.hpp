#pragma once

#include "core/state/CharacterState.hpp"

namespace bobtricks {

/**
 * \brief Produit une pose minimale a partir de l'etat de locomotion.
 */
class ProceduralAnimator {
public:
    /**
     * \brief Met a jour la pose procedurale minimale du personnage.
     */
    void update(double dtSeconds, CharacterState& characterState) const;
};

}  // namespace bobtricks
