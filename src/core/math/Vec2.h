#pragma once

#include <cmath>

/// @brief Vecteur 2D à virgule flottante double précision.
struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    // --- Arithmétique de base ---

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(double s)      const { return {x * s,   y * s};   }
    Vec2 operator/(double s)      const { return {x / s,   y / s};   }

    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }
    Vec2& operator*=(double s)      { x *= s;   y *= s;   return *this; }
    Vec2& operator/=(double s)      { x /= s;   y /= s;   return *this; }

    Vec2 operator-() const { return {-x, -y}; }

    bool operator==(const Vec2& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Vec2& o) const { return !(*this == o); }

    // --- Géométrie ---

    /// @brief Longueur (norme euclidienne).
    double length() const { return std::sqrt(x * x + y * y); }

    /// @brief Longueur au carré (sans racine carrée).
    double lengthSq() const { return x * x + y * y; }

    /// @brief Vecteur unitaire, ou zéro si la norme est nulle.
    Vec2 normalized() const {
        const double len = length();
        return len > 0.0 ? *this / len : Vec2{};
    }

    /// @brief Produit scalaire.
    double dot(const Vec2& o) const { return x * o.x + y * o.y; }

    /// @brief Produit vectoriel scalaire (composante z).
    double cross(const Vec2& o) const { return x * o.y - y * o.x; }

    /// @brief Vecteur perpendiculaire (rotation 90° antihoraire).
    Vec2 perp() const { return {-y, x}; }

    /// @brief Distance euclidienne vers un autre point.
    double distanceTo(const Vec2& o) const { return (*this - o).length(); }

    // --- Interpolation ---

    /// @brief Interpolation linéaire vers `o` par facteur `t` dans [0, 1].
    Vec2 lerp(const Vec2& o, double t) const {
        return {x + (o.x - x) * t, y + (o.y - y) * t};
    }
};

/// @brief Multiplication scalaire commutative (s * v).
inline Vec2 operator*(double s, const Vec2& v) { return v * s; }
