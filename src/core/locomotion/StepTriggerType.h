#pragma once

/** @brief Type de déclencheur ayant provoqué une demande de pas. */
enum class StepTriggerType {
    None,       // no trigger — step not requested this tick
    Normal,     // rear foot too far behind pelvis at walking speed
    Emergency,  // XCoM (capture point) escaped past front foot
};
