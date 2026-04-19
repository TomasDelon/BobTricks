#pragma once

#include "config/AppConfig.h"
#include "core/character/CharacterState.h"
#include "core/character/CMState.h"
#include "core/math/Vec2.h"
#include "core/terrain/Terrain.h"

struct GroundReferenceSample {
    Vec2 back;
    Vec2 fwd;
    double mean_y = 0.0;
    double slope  = 0.0;
};

struct GroundReferenceState {
    Vec2 pelvis_ref;
    bool airborne_ref = false;
    GroundReferenceSample sample;
};

double clampTerrainEndpointX(const Terrain& terrain,
                             const Vec2& pelvis,
                             double radius,
                             double x_start,
                             double x_target);

GroundReferenceSample computeGroundReferenceSample(const Terrain& terrain,
                                                   const Vec2& pelvis,
                                                   double cm_x,
                                                   double facing,
                                                   double speed_x,
                                                   double L,
                                                   const TerrainSamplingConfig& ts,
                                                   double dt,
                                                   double left_prev_x,
                                                   double right_prev_x);

GroundReferenceState updateGroundReference(CharacterState& ch,
                                           const Terrain& terrain,
                                           const CMState& cm,
                                           const Vec2& pelvis_ref,
                                           bool airborne_ref,
                                           const TerrainSamplingConfig& ts,
                                           double L,
                                           double dt);
