#pragma once

#include <SDL2/SDL.h>
#include "core/runtime/Camera2D.h"
#include "core/character/CMState.h"
#include "core/character/CharacterState.h"
#include "config/AppConfig.h"

class CharacterRenderer
{
public:
    void render(SDL_Renderer*         renderer,
                const Camera2D&       camera,
                const CMState&        cm,
                const CharacterState& character,
                const CMConfig&       cmConfig,
                double                ground_y,
                int                   viewport_w,
                int                   viewport_h) const;

private:
    static void drawFilledCircle(SDL_Renderer* renderer, float cx, float cy, float radius);
};
