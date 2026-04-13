#pragma once

#include "render/RenderState.hpp"

struct SDL_Renderer;

namespace bobtricks {

/**
 * \brief Renderer SDL limite a l'affichage de RenderState.
 */
class SDLRenderer {
public:
    SDLRenderer() = default;
    ~SDLRenderer();

    /**
     * \brief Initialise le renderer SDL a partir d'une fenetre native.
     */
    bool initialize(void* nativeWindowHandle);

    /**
     * \brief Libere les ressources SDL du renderer.
     */
    void shutdown();

    /**
     * \brief Affiche une frame a partir d'un RenderState.
     */
    void render(const RenderState& renderState);

private:
    SDL_Renderer* renderer_ {nullptr};
};

}  // namespace bobtricks
