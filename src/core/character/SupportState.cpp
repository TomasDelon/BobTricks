#include "core/character/SupportState.h"

#include <cmath>

double SupportState::center() const
{
    return 0.5 * (x_left + x_right);
}

double SupportState::width() const
{
    return std::abs(x_right - x_left);
}

double SupportState::ground_center() const
{
    return 0.5 * (y_left + y_right);
}

bool SupportState::both_planted() const
{
    return left_planted && right_planted;
}
