#pragma once

namespace bobtricks {

/**
 * \brief Vecteur 2D minimal utilise par le coeur du projet.
 */
struct Vec2 {
    double x {0.0};
    double y {0.0};
};

inline Vec2 operator+(const Vec2& lhs, const Vec2& rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

inline Vec2 operator-(const Vec2& lhs, const Vec2& rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

inline Vec2 operator*(const Vec2& value, double scalar) {
    return {value.x * scalar, value.y * scalar};
}

}  // namespace bobtricks
