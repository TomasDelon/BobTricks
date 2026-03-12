#pragma once

#include "core/state/CharacterState.hpp"
#include "platform/PlatformRuntime.hpp"
#include "render/RenderState.hpp"

namespace bobtricks {

/**
 * \brief Adapte le snapshot coeur en donnees de rendu.
 */
class RenderStateAdapter {
public:
    /**
     * \brief Construit l'etat de rendu a partir de l'etat autoritaire.
     */
    RenderState build(const CharacterState& characterState, WindowSize viewport) const;
};

}  // namespace bobtricks
