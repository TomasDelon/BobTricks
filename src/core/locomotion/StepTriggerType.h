#pragma once

// Type of trigger that fired in shouldStep().
// Stored in CharacterState::last_trigger for telemetry.
enum class StepTriggerType {
    None,       // no trigger — step not requested this tick
    Normal,     // rear foot too far behind pelvis at walking speed
    Emergency,  // XCoM (capture point) escaped past front foot
};
